#include "hal/joystick.h"
#include "hal/led.h"
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "game.h"

/*
Control Flow:

 - main initialization
 - game loop
  - ready print
  - generate rng and check for quit
  - start timer and break when user inputs joystick
  - quit if timer > 5000 ms or |x| above threshold
  - process time and joystick values for high score / led control
  - print out result
  - back to start of loop
- reset state and quit program
*/

int main()
{
    game_initialize();
    game_print_intro();

    float best_time = 5000; // Initialize with the timeout value

    // Game loop
    while (true)
    {
        game_ready_sequence(); // Ready phase and input check

        int required_dir = game_wait_for_stimulus(); //  Wait for stimulus and get required direction

        if (required_dir == 0)
        { // Player quit during the wait
            break;
        }

        game_start_timer();

        if (!game_process_reaction(required_dir, &best_time))
        { // Player quit or timed out during the reaction phase
            break;
        }
    }

    game_cleanup();
    return 0;
}