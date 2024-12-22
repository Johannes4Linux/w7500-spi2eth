#include "print.h"
#include "W7500x_uart.h"

void print(const char *format, ...) {
	va_list args;
	va_start(args, format);

	for (const char *p = format; *p != '\0'; p++) {
		if (*p == '%') {
			p++;
			switch (*p) {
				case 'd': {
						  int value = va_arg(args, int);
						  if (value < 0) {
							  S_UartPutc('-');
							  value = -value;
						  }
						  int divisor = 1;
						  while (value / divisor >= 10) {
							  divisor *= 10;
						  }
						  while (divisor != 0) {
							  S_UartPutc('0' + value / divisor);
							  value %= divisor;
							  divisor /= 10;
						  }
						  break;
					  }
				case 's': {
						  char *str = va_arg(args, char *);
						  while (*str != '\0') {
							  S_UartPutc(*str++);
						  }
						  break;
					  }
				case 'x': {
						  unsigned int value = va_arg(args, unsigned int);
						  char hex[] = "0123456789abcdef";
						  for (int i = 7; i >= 0; i--) {
							  S_UartPutc(hex[(value >> (i * 4)) & 0xF]);
						  }
						  break;
					  }
				default:
					  S_UartPutc(*p);
					  break;
			}
		} else {
			S_UartPutc(*p);
		}
	}

	va_end(args);
}
