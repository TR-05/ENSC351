#include "hal/joystick.h"
#include "hal/led.h"
#include "hal/button.h"
#include <math.h>
#include <unistd.h>
#include <stdio.h>

int main(){
    joystick_initialize();
    led_initialize();
    button_initialize();
    led_act_set_off();
    led_pwr_set_off();

    while(1) {
        float x = joystick_get_x_normalized();
        usleep(100); // give ADC time to process (100 microseconds is good, less doesn't work reliably)
        float y = joystick_get_y_normalized();
        if (fabs(x) > 0.02) {
            led_act_set_blink(300000 / ((x*x) * 100), 1000 / ((x*x) * 100) * 2); 
        } else {
            led_act_set_off();
        }
        if (fabs(y) > 0.02) {
            led_pwr_set_blink(300000 / ((y*y) * 100), 3000 / ((y*y) * 100) * 2); 
        } else {
            led_pwr_set_off();
        }
        printf("X: %.2f, Y:%.2f, Sel: %d\n", x, y, button_pressing());
        usleep(50000);
    }
    joystick_disable(); // idk if this matters

    return 0;
}