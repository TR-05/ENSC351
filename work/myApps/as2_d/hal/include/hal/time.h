#ifndef TIME_H
#define SPITIME_H_H

#include <stdint.h>
#include <unistd.h>

/*
Header file giving basic time related functionality
*/

long long get_time_in_ms(void);
void sleep_for_ms(long long delayInMs);

#endif // TIME_H