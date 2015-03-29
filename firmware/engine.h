#include <stdint.h>


const int8_t ENGINE_MAX_SPEED = 127;
const int8_t ENGINE_MIN_SPEED = -128;

// freq = 1 / time
// pulse freq = FCPU / prescaler * ICR1
// 20000 = 16000000 / (1 * 800) -> 20khz is good to reduce noise
const uint16_t ENGINE_MAX_VELOCITY = 800;
// Minimum velocity, must be less than ENGINE_MAX_VELOCITY
const uint16_t ENGINE_MIN_VELOCITY = 120;

const uint8_t ENGINE_PRESCALER = (1 << CS10);


class Engine {
	public:
		Engine(
			volatile uint8_t *ddr,
			volatile uint8_t *port,
			uint8_t pin_direction,
			uint8_t pin_break,
			uint8_t pin_pwm
		);
		int8_t get_speed(void);
		void set_speed(int8_t speed);

	private:
		volatile uint8_t *motor_port;
		uint8_t motor_direction;
		uint8_t motor_break;
		uint8_t motor_pwm;

		int8_t speed;
};
