#include <stdio.h>
#ifndef UART_H
#define UART_H

void uart_init(void);
char uart_getchar(void);
void uart_putchar(char c);

#endif //UART_H