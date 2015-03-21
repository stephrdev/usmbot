#include <avr/io.h>

#include <bbq/time.h>

#include "steering.h"


const uint8_t STEERING_MAP[8] = {
	0b1000,
	0b1100,
	0b0100,
	0b0110,
	0b0010,
	0b0011,
	0b0001,
	0b1001,
};


Steering::Steering(
	volatile uint8_t *ddr,
	volatile uint8_t *port,
	uint8_t pin1,
	uint8_t pin2,
	uint8_t pin3,
	uint8_t pin4
) {
	// init speed and direction properties
	direction = 0;
	speed = 10;

	// Remember some ports and pins.
	motor_port = port;
	motor_pin1 = pin1;
	motor_pin2 = pin2;
	motor_pin3 = pin3;
	motor_pin4 = pin4;

	// Configure pins.
	*ddr |= ((1 << pin1) | (1 << pin2) | (1 << pin3) | (1 << pin4));
};

void Steering::set_direction(int8_t new_direction) {
	direction = new_direction;

	if (direction == STEERING_NONE) {
		*motor_port &= ~(
			(1 << motor_pin1)
			| (1 << motor_pin2)
			| (1 << motor_pin3)
			| (1 << motor_pin4)
		);
	}
}

uint8_t Steering::get_speed(void) {
	return speed;
}

void Steering::set_speed(uint8_t new_speed) {
	speed = new_speed;
}

void Steering::step(void) {
	if (direction == STEERING_NONE) {
		return;
	}

	if ((time_ms() - last_step_ms) >= speed) {
		last_step_ms = time_ms();

		if (last_step > 7) {
			last_step = 0;
		} else if (last_step < 0) {
			last_step = 7;
		}

		if (STEERING_MAP[last_step] & (1 << 0)) {
			*motor_port |= (1 << motor_pin1);
		} else {
			*motor_port &= ~(1 << motor_pin1);
		}
		if (STEERING_MAP[last_step] & (1 << 1)) {
			*motor_port |= (1 << motor_pin2);
		} else {
			*motor_port &= ~(1 << motor_pin2);
		}
		if (STEERING_MAP[last_step] & (1 << 2)) {
			*motor_port |= (1 << motor_pin3);
		} else {
			*motor_port &= ~(1 << motor_pin3);
		}
		if (STEERING_MAP[last_step] & (1 << 3)) {
			*motor_port |= (1 << motor_pin4);
		} else {
			*motor_port &= ~(1 << motor_pin4);
		}

		last_step -= direction;
	}
}
