#ifndef REACTION_METHODS_H
#define REACTION_METHODS_H

#include <stdbool.h>
/*
Header file organizing the reaction timer assignment methods
*/

void game_initialize(void);
void game_cleanup(void);
void game_print_intro(void);

void game_ready_sequence(void);
int game_wait_for_stimulus(void);

void game_start_timer(void);
float game_read_timer_ms(void);

bool game_process_reaction(int required_dir, float *best_time);

#endif // REACTION_METHODS_H