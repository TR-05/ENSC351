#ifndef LED_H
#define LED_H

/*
Header file giving control of both the green ACT LED and red PWR LED on the BeagleboardY-AI
'*/
// set all LED addressable parameters to a known state, and set initalized = true so that other functions work appropriately
void led_initialize(void);

// set the specified LED to the specified on off state
void led_pwr_set_on(void); 
void led_act_set_on(void); 
void led_pwr_set_off(void); 
void led_act_set_off(void); 

void led_pwr_set_blink(int on_time_ms, int off_time_ms);
void led_act_set_blink(int on_time_ms, int off_time_ms);


#endif // LED_H