#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>

#include "sim_avr.h"
#include "avr_ioport.h"
#include "sim_elf.h"
#include "sim_gdb.h"
#include "sim_vcd_file.h"
#include "button.h"

#define TRUE  1
#define FALSE 0

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


avr_t * avr = NULL;
avr_vcd_t vcd_file;


// NONBLOCKING KEYBOARD INPUT
// http://cc.byexamples.com/2007/04/08/non-blocking-user-input-in-loop-without-ncurses/
void set_term_nonblock(int enable) {
    struct termios ttystate;
 
    // get current state
    tcgetattr(STDIN_FILENO, &ttystate);
 
    if (enable) {
        // turn off canonical mode (wait for newline)
        ttystate.c_lflag &= ~ICANON;
        ttystate.c_lflag &= ~ECHO;
        // set min. # of input to read
        ttystate.c_cc[VMIN] = 1;
    } else {
        // re-enable canonical mode
        ttystate.c_lflag |= ICANON;
        ttystate.c_lflag |= ECHO;
    }

    // set new state
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

int kbhit() {
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}


// INPUT HANDLING
typedef struct input_callback_t {
	char c;
	void (* callback)(char c, void * param);
	void * param;
} input_callback_t;

int handle_user_input(input_callback_t * charmap, int charmap_size, char c) {
	// Exit on "q"
	if (c == 'q') {
		return FALSE;
	}
	fprintf(stdout, ANSI_COLOR_BLUE "IN -> %c\n" ANSI_COLOR_RESET, c);

	// Look through registered chars and trigger callback if one was found.
	for(int i = 0; i < charmap_size; i++) {
		if (charmap[i].c == c) {
			// Call callback function with previous provided param.
			charmap[i].callback(c, charmap[i].param);
			return TRUE;
		}
	}
	return TRUE;
}

void user_input_loop(input_callback_t charmap[], int charmap_size) {
	// set non blocking for term, we dont want to wait for newlines.
	set_term_nonblock(TRUE);
	int i;

	for (;;) {
		i = kbhit();

		// Handle input if available (i), if function returns false, exit simulator.
		if (i && !handle_user_input(charmap, charmap_size, getc(stdin))) {
			break;
		};

		// Sleep a bit.
		usleep(1);
	};

	// disable nonblocking.
	set_term_nonblock(FALSE);
}


// PIN MONITORING
typedef struct pin_monitor_t {
	char port;
	int pin;
} pin_monitor_t;

void pin_monitor_callback(struct avr_irq_t * irq, uint32_t value, void * param) {
	pin_monitor_t * pm = (pin_monitor_t *)param;

	// Print info, green for HIGH levels and red for LOW levels.
	fprintf(
		stdout,
		"%s%c%i -> %s%s\n",
		value > 0 ? ANSI_COLOR_GREEN : ANSI_COLOR_RED,
		pm->port,
		pm->pin,
		value > 0 ? "HIGH" : "LOW",
		ANSI_COLOR_RESET
	);
}

pin_monitor_t * new_pin_monitor(char port, int pin) {
	// Create a new port monitor (will be pased to callback,
	// includes infos about monitored pin)
	pin_monitor_t * pm = (pin_monitor_t *)malloc(sizeof(pin_monitor_t *));
	pm->port = port;
	pm->pin = pin;

	// Register monitor, pass monitor struct too.
	avr_irq_register_notify(
		avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ(pm->port), pm->pin),
		pin_monitor_callback,
		pm
	);

	return pm;
}


// PIN TRIGGERING
typedef struct pin_trigger_t {
	char port;
	int pin;
	int pullup;

	avr_irq_t * irq;
} pin_trigger_t;

pin_trigger_t * new_pin_trigger(char port, int pin, int pullup) {
	pin_trigger_t * pr = (pin_trigger_t *)malloc(sizeof(pin_trigger_t *));
	pr->port = port;
	pr->pin = pin;
	pr->pullup = pullup;

	// alloc_irq needs a name, therefore provide one.
	const char * name = "pin_trigger";
	pr->irq = avr_alloc_irq(&avr->irq_pool, 0, 1, &name);

	// Set default value of pin - fake pullup or pulldown.
	avr_raise_irq(pr->irq, pr->pullup > 0 ? 1 : 0);

	// Connect irq with port/pin, irq will be raised when the trigger is activated.
	avr_connect_irq(
		pr->irq,
		avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ(pr->port), pr->pin)
	);
	return pr;
}

static avr_cycle_count_t pin_trigger_release(avr_t * avr, avr_cycle_count_t when, void * param) {
	// Reconstruct trigger struct and reset pin after timeout.
	pin_trigger_t * pr = (pin_trigger_t *)param;
	avr_raise_irq(pr->irq, pr->pullup > 0 ? 1 : 0);
	return 0;
}

void pin_trigger_raise(pin_trigger_t * pr, uint32_t duration_usec) {
	avr_cycle_timer_cancel(avr, pin_trigger_release, pr);

	// raise pin and prepare reset after the given duration.
	avr_raise_irq(pr->irq, pr->pullup > 0 ? 0 : 1);
	avr_cycle_timer_register_usec(avr, duration_usec, pin_trigger_release, pr);
}


// HELPERS
//
void press_button(char c, void * param) {
	pin_trigger_t * pr = (pin_trigger_t *)param;
	// trigger pin with 850ms.
	pin_trigger_raise(pr, 850000);
}


// AVR CPU LOOP
static void * avr_run_loop(void *param) {
	for (;;) {
		avr_run(avr);
	}

	return NULL;
}


// SIMULATOR STARTUP
int main(int argc, char *argv[]) {
	elf_firmware_t firmware;
	const char * firmware_path = "../firmware/build-mega/firmware.elf";

	elf_read_firmware(firmware_path, &firmware);
	fprintf(stdout, "%s: Firmware: %s\n", argv[0], firmware_path);

	// Create cpu to simulate.
	avr = avr_make_mcu_by_name("atmega1280");
	if (!avr) {
		fprintf(stderr, "%s: AVR Error.\n", argv[0]);
		exit(1);
	}

	// Boot avr cpu and load firmware.
	avr_init(avr);
	avr_load_firmware(avr, &firmware);

	// Initialize gdb
	avr->gdb_port = 1234;
	if (0) {
		// avr->state = cpu_Stopped;
		avr_gdb_init(avr);
	}

	// The AVR run on it's own thread. it even allows for debugging!
	pthread_t run;
	pthread_create(&run, NULL, avr_run_loop, NULL);

	// Monitor two leds
	new_pin_monitor('A', 4);
	new_pin_monitor('A', 5);

	// Monitor motor pins.
	new_pin_monitor('H', 6);
	new_pin_monitor('H', 5);
	new_pin_monitor('B', 4);
	new_pin_monitor('B', 5);

	// prepare two buttons with pull-down setup
	pin_trigger_t * button_left = new_pin_trigger('A', 0, FALSE); // 22
	pin_trigger_t * button_right = new_pin_trigger('A', 1, FALSE); // 23

	// register some char inputs which are mapped to pin triggers.
	input_callback_t charmap[2] = {
		{'c', press_button, button_left},
		{'v', press_button, button_right}
	};
	user_input_loop(charmap, 2);
}
