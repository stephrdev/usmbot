#include <Arduino.h>

const int BUTTON_LEFT = 22;
const int BUTTON_RIGHT = 23;
const int LED_LEFT = 26;
const int LED_RIGHT = 27;

const int MOTOR_PIN1 = 8;	// Blue   - 28BYJ48 pin 1
const int MOTOR_PIN2 = 9;	// Pink   - 28BYJ48 pin 2
const int MOTOR_PIN3 = 10;	// Yellow - 28BYJ48 pin 3
const int MOTOR_PIN4 = 11;	// Orange - 28BYJ48 pin 4
							// Red    - 28BYJ48 pin 5 (VCC)

const int MOTOR_OFF = 8;

const int MOTOR_LOOKUP[9] = {
	B01000,
	B01100,
	B00100,
	B00110,
	B00010,
	B00011,
	B00001,
	B01001,
	B00000
};

const long BUTTON_DELAY = 50;

long currentMillis;

int leftButtonState = LOW;
int rightButtonState = LOW;
int leftLedState = LOW;
int rightLedState = LOW;

int prevLeftButtonState = LOW;
int prevRightButtonState = LOW;

long prevLeftButtonMillis = 0;
long prevRightButtonMillis = 0;

long prevMotorMillis = 0;
int motorSpeed = 100;
int motorStep = 0;
int motorDirection = 1;

void setup(void) {
	pinMode(BUTTON_LEFT, INPUT);
	pinMode(BUTTON_RIGHT, INPUT);
	pinMode(LED_LEFT, OUTPUT);
	pinMode(LED_RIGHT, OUTPUT);

	pinMode(MOTOR_PIN1, OUTPUT);
	pinMode(MOTOR_PIN2, OUTPUT);
	pinMode(MOTOR_PIN3, OUTPUT);
	pinMode(MOTOR_PIN4, OUTPUT);
}


void loopButton(void) {
	int pressedLeft = digitalRead(BUTTON_LEFT);

	if (pressedLeft != prevLeftButtonState) {
		prevLeftButtonMillis = currentMillis;
	}
	if ((currentMillis - prevLeftButtonMillis) > BUTTON_DELAY) {
		if (pressedLeft == HIGH) {
			leftLedState = leftLedState == LOW ? HIGH : LOW;
			if (leftLedState == HIGH) {
				rightLedState = LOW;
			}
		}
	}
	prevLeftButtonState = pressedLeft;

	int pressedRight = digitalRead(BUTTON_RIGHT);
	if (pressedRight != prevRightButtonState) {
		prevRightButtonMillis = currentMillis;
	}
	if ((currentMillis - prevRightButtonMillis) > BUTTON_DELAY) {
		if (pressedRight == HIGH) {
			rightLedState = rightLedState == LOW ? HIGH : LOW;
			if (rightLedState == HIGH) {
				leftLedState = LOW;
			}
		}
	}
	prevRightButtonState = pressedRight;
}


void loopLed(void) {
	digitalWrite(LED_LEFT, leftLedState);
	digitalWrite(LED_RIGHT, rightLedState);
}


void setMotorPins(int out) {
	digitalWrite(MOTOR_PIN1, bitRead(MOTOR_LOOKUP[out], 0));
	digitalWrite(MOTOR_PIN2, bitRead(MOTOR_LOOKUP[out], 1));
	digitalWrite(MOTOR_PIN3, bitRead(MOTOR_LOOKUP[out], 2));
	digitalWrite(MOTOR_PIN4, bitRead(MOTOR_LOOKUP[out], 3));
}

void loopMotor(void) {
	if (leftLedState == HIGH) {
		motorDirection = -1;
	} else if (rightLedState == HIGH) {
		motorDirection = 1;
	} else {
		motorDirection = 0;
		setMotorPins(MOTOR_OFF);
	}

	if ((currentMillis - prevMotorMillis) >= motorSpeed && motorDirection != 0) {
		prevMotorMillis = currentMillis;
		if (motorStep < 0) {
			motorStep = 7;
		} else if (motorStep > 7) {
			motorStep = 0;
		}
		setMotorPins(motorStep);

		motorStep = motorStep - motorDirection;
	}
}

void loop(void) {
	currentMillis = millis();
	loopButton();
	loopLed();
	loopMotor();
}
