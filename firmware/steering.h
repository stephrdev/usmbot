#include <stdint.h>


const int8_t STEERING_LEFT = -1;
const int8_t STEERING_NONE = 0;
const int8_t STEERING_RIGHT = 1;


class Steering {
	public:
		Steering(
			volatile uint8_t *ddr,
			volatile uint8_t *port,
			uint8_t pin1,
			uint8_t pin2,
			uint8_t pin3,
			uint8_t pin4
		);
		int8_t get_direction(void);
		void set_direction(int8_t direction);
		uint8_t get_speed(void);
		void set_speed(uint8_t speed);

		void step(void);

	private:
		volatile uint8_t *motor_port;
		uint8_t motor_pin1;
		uint8_t motor_pin2;
		uint8_t motor_pin3;
		uint8_t motor_pin4;

		int8_t direction;
		uint8_t speed;

		uint16_t last_step_ms;
		int8_t last_step;
};
