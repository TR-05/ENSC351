#ifndef JOYSTICK_H
#define JOYSTICK_H

/*
Header file giving access to joystick data  over spi
*/

// initializes spi communication with the ADC, and the ADC reads joystick values
int joystick_initialize(void);

// gets the raw 12 bit data from the joystick
float joystick_get_x_raw(void);
float joystick_get_y_raw(void);

// gets joystick data normalized from -1 to 1 CANNOT BE CALLED DIRECTLY AFTER A JOYSTICK READ WITHOUT SLEEP
float joystick_get_x_normalized(void);
float joystick_get_y_normalized(void);

// override the default calibration values, may be useful?
void joystick_configure_x(int max, int mid, int min);
void joystick_configure_y(int max, int mid, int min);

void joystick_disable(void);

#endif // JOYSTICK_H