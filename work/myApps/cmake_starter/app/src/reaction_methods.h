#ifndef REACTION_METHODS_H
#define REACTION_METHODS_H

/*
Header file organizing the reaction timer assignment methods 
*/

void reaction_initialize(void);
void reaction_prep_user(void);
bool reaction_gen_rng(void);
void reaction_start_timer(void);
float reaction_read_timer_ms(void);

#endif // REACTION_METHODS_H