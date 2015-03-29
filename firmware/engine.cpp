#include <avr/io.h>
#include <avr/interrupt.h>

#include <bbq/time.h>

#include "engine.h"


Engine::Engine(
	volatile uint8_t *ddr,
	volatile uint8_t *port,
	uint8_t pin_direction,
	uint8_t pin_break,
	uint8_t pin_pwm
) {
	// init speed and direction properties
	speed = 0;

	// Remember some ports and pins.
	motor_port = port;
	motor_direction = pin_direction;
	motor_break = pin_break;
	motor_pwm = pin_pwm;

	// Configure pins.
	*ddr |= ((1 << pin_direction) | (1 << pin_break) | (1 << pin_pwm));

	TCCR2A |= (1 << WGM20);
	TCCR2B |= (1 << CS22);
};

int8_t Engine::get_speed(void) {
	return speed;
}

void Engine::set_speed(int8_t new_speed) {
	uint8_t pwm_speed;

	// Set or release break based on speed.
	if (speed == 0) {
		*motor_port |= (1 << motor_break);
	} else {
		*motor_port &= ~(1 << motor_break);
	}

	// Set direction according to speed (pos/neg.)
	if (speed > 0) {
		*motor_port |= (1 << motor_direction);
	} else {
		*motor_port &= ~(1 << motor_direction);
	}

	// Convert speed from range (-128 - 127) to range (0 - 255)
	if (speed > 0) {
		pwm_speed = speed * 2;
	} else {
		pwm_speed = (speed * -2) - 1;
	}

	if (pwm_speed == 0) {
		// Halt. Disable timer and set pin to low.
		TCCR2A &= ~(1 << COM2B1);
		*motor_port &= ~(1 << motor_pwm);
	} else if (pwm_speed == 255) {
		// Full speed, disable timer and set pin to high.
		TCCR2A &= ~(1 << COM2B1);
		*motor_port |= (1 << motor_pwm);
	} else {
		// Something between, activate timer and set speed as compare value.
		TCCR2A |= (1 << COM2B1);
		OCR2B = pwm_speed;
	}

	// Remember new speed.
	speed = new_speed;
}
