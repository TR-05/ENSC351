#ifndef PWM_H
#define PWM_H

/*
Header file giving control of pwm
'*/
void pwm_init(void);

// set the pwm
void pwm_set_frequency(int newFrequency);
long long pwm_get_frequency(void);

void pwm_cleanup(void);

#endif // PWM_H