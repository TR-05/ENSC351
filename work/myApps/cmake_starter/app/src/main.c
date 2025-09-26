#include "hal/joystick.h"
#include "hal/led.h"
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include "./app/src/reaction_methods.h"

int main()
{
    // setup leds and joystick, print intro text to player terminal
    reaction_initialize();

    float best_time = 5000;
    // main game loop
    while (true)
    {
        reaction_prep_user();
        // repeat regeneration if player inputs too soon
        while(reaction_gen_rng() == 1);

        // wait for player input or break if player is inactive
        while (true)
        {
            if (fabs(joystick_get_x_normalized()) > 0.75 || fabs(joystick_get_y_normalized()) > 0.75) break;
            if (reaction_read_timer_ms() >= 5000) break;   // 5 second timeout
        }

        // save loop time before anything else for accuracy
        float reaction_time = reaction_read_timer_ms();

        // exit main game loop if player is inactive
        if (reaction_read_timer_ms() >= 5000)
        {
            printf("No input within 5000ms; quitting!\n");
            break; // 5 second timeout
        }

        // turn off leds ones reaction test is finished
        led_act_set_off();
        led_pwr_set_off();

        // save joystick state immediately after loop exits to determine outcome
        float y = joystick_get_y_normalized();
        float x = joystick_get_x_normalized();

        // exit main game loop if player wants to quit (or just fumbles that badly)
        if (fabs(x) > 0.75)
        {
            printf("User selected to quit.\n");
            break;
        }

        // if user moved the joystick in the right direction readout their time + highscore
        if (y / fabs(y) == dir)
        {
            printf("Correct!\n");
            if (reaction_time < best_time)
            {
                best_time = reaction_time;
                printf("New best time!\n");
            }
            printf("Your reaction time was %dms; best so far in game is %dms\n", (int)reaction_time, (int)best_time);
            for (int i = 0; i < 5; i++)
            {
                led_act_set_on();
                usleep(100000);
                led_act_set_off();
                usleep(100000);
            }
        }
        // if user moved the joystick in the wrong direction make fun of them
        else
        {
            printf("Incorrect.\n");
            for (int i = 0; i < 5; i++)
            {
                led_pwr_set_on();
                usleep(100000);
                led_pwr_set_off();
                usleep(100000);
            }
        }
    }

    // disable spi cleanly and turn off leds to return to default state when program ends
    joystick_disable();
    led_act_set_off();
    led_pwr_set_off();
    return 0;
}