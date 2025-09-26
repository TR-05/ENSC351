#include "hal/joystick.h"
#include "hal/led.h"
#include "hal/button.h"
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>

void reaction_initialize(void) {
    srand(time(NULL));
    joystick_initialize();
    led_initialize();
    button_initialize();
    led_act_set_off();
    led_pwr_set_off();

    printf("Hello embedded world, from Trevor!\n");
    printf("When the LEDs light up, press the joystick in that direction!\n");
    printf("(Press left or right to exit)\n");
}

void reaction_prep_user(void) {
        printf("Get ready...\n");
        // blink red and green 4 times to tell player to get ready
        for (int i = 0; i < 4; i++)
        {
            led_act_set_on();
            led_pwr_set_off();
            usleep(250000); // 250 ms
            led_act_set_off();
            led_pwr_set_on();
            usleep(250000); // 250 ms
        }

        // turn off leds so when they next turn on the player tests their reaction time without distraction
        led_act_set_off();
        led_pwr_set_off();

        // ensure the player isn't cheating / not ready
        if (fabs(joystick_get_y_normalized()) > 0.1)
        {
            printf("Please let go of joystick\n");
        }
        while (fabs(joystick_get_y_normalized()) > 0.1)
        {
            usleep(10000); // 10 ms loop as it really isn't critical
        }
}


bool reaction_gen_rng(void) {
        int random_sleep_time = 500000 + (rand() % 2500000); // Random between 0.5-3 seconds
        usleep(random_sleep_time);
        if (fabs(joystick_get_y_normalized()) > 0.1)
        {
            printf("too soon\n"); // LOOP BACK TO WHILE LOOP
            return 1;
        }

        int dir = (rand() % 2) ? 1 : -1; // Randomly choose 1 or -1 (up or down)
        if (dir == 1)
        {
            printf("Press UP now!\n");
            led_act_set_on();
            led_pwr_set_off();
        }
        else
        {
            printf("Press DOWN now!\n");
            led_act_set_off();
            led_pwr_set_on();
        }
        return 0;
}

struct timeval start_time;
void reaction_start_timer(void) {
    gettimeofday(&start_time, NULL);
}

float reaction_read_timer_ms(void) {
struct timeval current_time;
    gettimeofday(&current_time, NULL);
    return ( (current_time.tv_sec - start_time.tv_sec) +
                      (current_time.tv_usec - start_time.tv_usec) / 1000000.0 ) / 1000.0;
}