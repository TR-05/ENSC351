#ifndef ENCODER_H
#define ENCODER_H

#include <stdbool.h>

/*
Header file giving access to encoder data
*/

int Encoder_init(int pinA, int pinB, int pinButton);
int Encoder_read(void);
int Encoder_cleanup(void);

#endif // ENCODER_H