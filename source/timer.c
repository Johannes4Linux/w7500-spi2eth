#include "timer.h"

void delay_us(volatile int delay) {
	delay = delay * 3;
	while (delay--)
		__asm__("nop");
}

void delay_ms(volatile int delay) {
	delay = delay * 2500;
	while (delay--)
		__asm__("nop");
}

