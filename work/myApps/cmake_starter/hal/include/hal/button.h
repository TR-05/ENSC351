#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

/*
Header file giving access to gpio button data
*/

// initializes gpio communication with the button
int button_initialize(void);

// returns if button is currently pressed or not
bool button_pressing(void);

// returns 1 if it detects a rising edge, 0 otherwise
bool button_rising_edge(void);

// returns 1 if it detects a falling edge, 0 otherwise
bool button_falling_edge(void);

void button_disable(void);

#endif // BUTTON_H