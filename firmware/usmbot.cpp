#include <stdint.h>
#include <bbq/serial.h>


int main(void) {
	Serial terminal = Serial(9600U);

	terminal.printLine("We are here!");

	while(true) {
	}

	return(0);
}
