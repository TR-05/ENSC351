#include "hal/joystick.h"
#include "hal/led.h"
#include "hal/button.h"
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>

void game_initialize(void)
{
    srand(time(NULL));
    joystick_initialize();
    led_initialize();
    led_act_set_off();
    led_pwr_set_off();
}

void game_cleanup(void)
{
    joystick_disable();
    led_act_set_off();
    led_pwr_set_off();
}

void game_print_intro(void)
{
    printf("Hello embedded world, from Trevor!\n");
    printf("When the LEDs light up, press the joystick in that direction!\n");
    printf("(Press left or right to exit)\n");
    fflush(stdout);
}

void game_ready_sequence(void)
{
    printf("Get ready...\n");
    fflush(stdout);

    // Blink LEDs
    for (int i = 0; i < 4; i++)
    {
        led_act_set_on();
        led_pwr_set_off();
        usleep(250000); // 250 ms
        led_act_set_off();
        led_pwr_set_on();
        usleep(250000); // 250 ms
    }
    led_act_set_off();
    led_pwr_set_off();

    // Wait for player to release joystick
    while (fabs(joystick_get_y_normalized()) > 0.1)
    {
        printf("Please let go of joystick (y: %.2f)\n", joystick_get_y_normalized());
        usleep(10000);
    }
}

// Returns 1 for UP, -1 for DOWN, or 0 if player quits with X-axis input
int game_wait_for_stimulus(void)
{
    int dir = 0;
    while (true)
    {
        int random_sleep_time = 500000 + (rand() % 2500000); // 0.5-3 seconds
        usleep(random_sleep_time);

        // Check for quit condition BEFORE showing stimulus
        if (fabs(joystick_get_x_normalized()) > 0.75)
        {
            return 0; // Quit
        }

        // Check for "too soon" input BEFORE showing stimulus
        if (fabs(joystick_get_y_normalized()) > 0.1)
        {
            printf("Too soon! Try again.\n");
            fflush(stdout);
            continue; // Loop back to start for a new random wait
        }

        // Stimulus is shown
        dir = (rand() % 2) ? 1 : -1; // Randomly choose 1 (UP) or -1 (DOWN)
        if (dir == 1)
        {
            printf("Press UP now!\n");
            led_act_set_on(); // UP (Green/ACT)
            led_pwr_set_off();
        }
        else
        {
            printf("Press DOWN now!\n");
            led_act_set_off();
            led_pwr_set_on(); // DOWN (Red/PWR)
        }
        fflush(stdout);
        break; // Exit the while(true) loop and proceed to timer start
    }
    return dir;
}

struct timeval start_time;
void game_start_timer(void)
{
    gettimeofday(&start_time, NULL);
}

float game_read_timer_ms(void)
{
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    return ((current_time.tv_sec - start_time.tv_sec) +
            (current_time.tv_usec - start_time.tv_usec) / 1000000.0) *
           1000.0;
}

// best_time is passed by reference to allow modification
bool game_process_reaction(int required_dir, float *best_time)
{
    // Wait for player input or timeout (5000ms)
    while (!(fabs(joystick_get_x_normalized()) > 0.75 || fabs(joystick_get_y_normalized()) > 0.75 || game_read_timer_ms() >= 5000))
        ;

    float reaction_time = game_read_timer_ms();

    // Turn off LEDs immediately after reaction
    led_act_set_off();
    led_pwr_set_off();

    // Check for timeout
    if (reaction_time >= 5000)
    {
        printf("No input within 5000ms; quitting!\n");
        fflush(stdout);
        return false; // Signal to quit the main loop
    }

    float x = joystick_get_x_normalized();
    float y = joystick_get_y_normalized();

    // Check for quit via X-axis
    if (fabs(x) > 0.75)
    {
        printf("User selected to quit.\n");
        fflush(stdout);
        return false; // Signal to quit the main loop
    }

    // Check for correct direction (y-axis sign must match required_dir)
    if (y / fabs(y) == required_dir && fabs(y) > 0.75)
    { // Check for magnitude as well
        printf("Correct!\n");
        if (reaction_time < *best_time)
        {
            *best_time = reaction_time; // Update best time via pointer
            printf("New best time!\n");
        }
        printf("Your reaction time was %dms; best so far in game is %dms\n", (int)reaction_time, (int)*best_time);

        // Success Feedback
        for (int i = 0; i < 5; i++)
        {
            led_act_set_on();
            usleep(100000);
            led_act_set_off();
            usleep(100000);
        }
    }
    else
    {
        // Failure Feedback (Wrong direction or magnitude too low)
        printf("Incorrect or insufficient input.\n");
        for (int i = 0; i < 5; i++)
        {
            led_pwr_set_on();
            usleep(100000);
            led_pwr_set_off();
            usleep(100000);
        }
    }
    fflush(stdout);
    return true; // Continue the main loop
}