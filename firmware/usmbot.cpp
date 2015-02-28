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


	terminal.printLine("USMBOT READY.");

	while(true) {
		// steering module need to update stepper based on current speed
		// and direction settings.
		steer.step();

		if (terminal.hasData()) {
			uint8_t ctrl = terminal.receiveByte();

			// we have speed limits. Therefore we validate the current speed.
			if (ctrl == 'w' && steer.getSpeed() >= 10) {
				steer.setSpeed(steer.getSpeed() - 5);

				terminal.printNumber(steer.getSpeed());
				terminal.printLine(" set as new speed.");
			} else if (ctrl == 's' && steer.getSpeed() < 50) {
				steer.setSpeed(steer.getSpeed() + 5);
				terminal.printNumber(steer.getSpeed());
				terminal.printLine(" set as new speed.");
			}

			// Handle direction changes.
			if (ctrl == 'a') {
				steer.setDirection(STEERING_LEFT);
				terminal.printLine("Steering left..");
			} else if (ctrl == 'd') {
				steer.setDirection(STEERING_RIGHT);
				terminal.printLine("Steering right..");
			} else if (ctrl == 'q') {
				steer.setDirection(STEERING_NONE);
				terminal.printLine("Steering stopped..");
			}
		}
	}

	return(0);
}
