#include <stdint.h>


#define STEERING_LEFT -1
#define STEERING_NONE 0
#define STEERING_RIGHT 1


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
		void setDirection(int8_t direction);
		uint8_t getSpeed(void);
		void setSpeed(uint8_t speed);

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
