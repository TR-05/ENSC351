#ifndef GPIO_H
#define GPIO_H

#include <stdbool.h>

/*
Header file giving access to gpio gpio data
*/

// initializes gpio communication with the gpio
int gpio_initialize(int pin);

// returns if gpio is currently pressed or not
bool gpio_read(int pin);

// returns 1 if it detects a rising edge, 0 otherwise
bool gpio_rising_edge(void);

// returns 1 if it detects a falling edge, 0 otherwise
bool gpio_falling_edge(void);

void gpio_disable(void);

#endif // GPIO_H