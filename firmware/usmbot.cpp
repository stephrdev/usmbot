#include <stdint.h>
#include <avr/interrupt.h>
#include <bbq/serial.h>
#include <bbq/time.h>

#include "steering.h"


int main(void) {
	// Init time lib (for time_ms function) and activate interrupts.
	time_init();
	sei();

	// Load serial terminal for debugging and communication.
	Serial terminal = Serial(9600U);

	// Init bot functionalities.
	Steering steer = Steering(&DDRB, &PORTB, PB4, PB5, PB6, PB7);


	terminal.transmit_text("USMBOT READY.", true);

	while(true) {
		// steering module need to update stepper based on current speed
		// and direction settings.
		steer.step();

		if (terminal.has_data()) {
			uint8_t ctrl = terminal.receive_byte();

			// we have speed limits. Therefore we validate the current speed.
			if (ctrl == 'w' && steer.get_speed() >= 10) {
				steer.set_speed(steer.get_speed() - 5);

				terminal.transmit_number(steer.get_speed());
				terminal.transmit_text(" set as new speed.", true);
			} else if (ctrl == 's' && steer.get_speed() < 50) {
				steer.set_speed(steer.get_speed() + 5);
				terminal.transmit_number(steer.get_speed());
				terminal.transmit_text(" set as new speed.", true);
			}

			// Handle direction changes.
			if (ctrl == 'a') {
				steer.set_direction(STEERING_LEFT);
				terminal.transmit_text("Steering left..", true);
			} else if (ctrl == 'd') {
				steer.set_direction(STEERING_RIGHT);
				terminal.transmit_text("Steering right..", true);
			} else if (ctrl == 'q') {
				steer.set_direction(STEERING_NONE);
				terminal.transmit_text("Steering stopped..", true);
			}
		}
	}

	return(0);
}
