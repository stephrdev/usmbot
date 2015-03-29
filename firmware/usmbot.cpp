#include <stdint.h>
#include <avr/interrupt.h>
#include <bbq/serial.h>
#include <bbq/time.h>

#include "engine.h"
#include "steering.h"


int main(void) {
	// Init time lib (for time_ms function) and activate interrupts.
	time_init();
	sei();

	// Load serial terminal for debugging and communication.
	Serial terminal = Serial(9600U);

	// Init bot functionalities.
	Engine pedal = Engine(&DDRH, &PORTH, PH4, PH5, PH6);
	Steering wheel = Steering(&DDRB, &PORTB, PB4, PB5, PB6, PB7);


	terminal.transmit_text("USMBOT READY.", true);

	while(true) {
		// steering module need to update stepper based on current speed
		// and direction settings.
		wheel.step();

		if (terminal.has_data()) {
			uint8_t ctrl = terminal.receive_byte();

			if (ctrl == 'w' && pedal.get_speed() < ENGINE_MAX_SPEED) {
				pedal.set_speed(pedal.get_speed() + 1);

				terminal.transmit_number(pedal.get_speed());
				terminal.transmit_text(" set as new speed.", true);
			} else if (ctrl == 's' && pedal.get_speed() > ENGINE_MIN_SPEED) {
				pedal.set_speed(pedal.get_speed() - 1);
				terminal.transmit_number(pedal.get_speed());
				terminal.transmit_text(" set as new speed.", true);
			}

			// we have speed limits. Therefore we validate the current speed.
			if (ctrl == 'q' && wheel.get_speed() >= 10) {
				wheel.set_speed(wheel.get_speed() - 5);

				terminal.transmit_number(wheel.get_speed());
				terminal.transmit_text(" set as new speed.", true);
			} else if (ctrl == 'e' && wheel.get_speed() < 50) {
				wheel.set_speed(wheel.get_speed() + 5);
				terminal.transmit_number(wheel.get_speed());
				terminal.transmit_text(" set as new speed.", true);
			}

			// Handle direction changes.
			if (ctrl == 'a') {
				if (wheel.get_direction() == STEERING_RIGHT) {
					wheel.set_direction(STEERING_NONE);
					terminal.transmit_text("Steering stopped..", true);
				} else {
					wheel.set_direction(STEERING_LEFT);
					terminal.transmit_text("Steering left..", true);
				}
			} else if (ctrl == 'd') {
				if (wheel.get_direction() == STEERING_LEFT) {
					wheel.set_direction(STEERING_NONE);
					terminal.transmit_text("Steering stopped..", true);
				} else {
					wheel.set_direction(STEERING_RIGHT);
					terminal.transmit_text("Steering right..", true);
				}
			}
		}
	}

	return(0);
}
