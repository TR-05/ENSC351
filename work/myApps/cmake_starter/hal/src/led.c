#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "hal/led.h"

//#define DEBUG_MODE

#define ACT_TRIGGER_FILE "/sys/class/leds/ACT/trigger"
#define PWR_TRIGGER_FILE "/sys/class/leds/PWR/trigger"
#define PWR_DELAY_ON_FILE "/sys/class/leds/PWR/delay_on"
#define PWR_DELAY_OFF_FILE "/sys/class/leds/PWR/delay_off"
#define ACT_DELAY_ON_FILE "/sys/class/leds/ACT/delay_on"
#define ACT_DELAY_OFF_FILE "/sys/class/leds/ACT/delay_off"
#define ACT_BRIGHTNESS_FILE "/sys/class/leds/ACT/brightness"
#define PWR_BRIGHTNESS_FILE "/sys/class/leds/PWR/brightness"


void led_sleep_ms(int sleep_ms) {
    long nanoseconds = sleep_ms * 1000000; // multiply milliseconds by 10^6 to get nanoseconds
    struct timespec reqDelay = {0, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *) NULL);
}

void led_write_to_file(char *file_address, char *data)
{
    #ifdef DEBUG_MODE
        printf("Trying to open file: %s\n", file_address);
    #endif
    FILE *file = fopen(file_address, "w");
    if (file == NULL)
    {
        perror("Error opening LED file");
        exit(EXIT_FAILURE);
    }

    int charWritten = fprintf(file, "%s", data);
    if (charWritten <= 0)
    {
        perror("Error writing data to LED file");
        exit(EXIT_FAILURE);
    }
    #ifdef DEBUG_MODE
        printf("Successfully wrote to file\n");
    #endif
    fclose(file);
}



// set all LED addressable parameters to a known state, and set initalized = true so that other functions work appropriately
void led_initialize()
{
    //printf("ACT_TRIGGER_FILE: %s\n", ACT_TRIGGER_FILE);
    //printf("PWR_TRIGGER_FILE: %s\n", PWR_TRIGGER_FILE);
    //printf("PWR_DELAY_ON_FILE: %s\n", PWR_DELAY_ON_FILE);
    //printf("PWR_DELAY_OFF_FILE: %s\n", PWR_DELAY_OFF_FILE);
    led_pwr_set_off();
    led_act_set_off();
}

// set the specified LED to the specified on off state
void led_pwr_set_on(void)
{
    led_write_to_file(PWR_TRIGGER_FILE, "none");
    led_write_to_file(PWR_BRIGHTNESS_FILE, "1");
}

void led_act_set_on(void)
{
    led_write_to_file(ACT_TRIGGER_FILE, "none");
    led_write_to_file(ACT_BRIGHTNESS_FILE, "1");
}

void led_pwr_set_off(void)
{
    led_write_to_file(PWR_TRIGGER_FILE, "none");
    led_write_to_file(PWR_BRIGHTNESS_FILE, "0");
}
void led_act_set_off(void)
{
    led_write_to_file(ACT_TRIGGER_FILE, "none");
    led_write_to_file(ACT_BRIGHTNESS_FILE, "0");
}

void led_pwr_set_blink(int on_time_ms, int off_time_ms)
{
    char on_time_str[16];
    snprintf(on_time_str, sizeof(on_time_str), "%d", on_time_ms);
    char off_time_str[16];
    snprintf(off_time_str, sizeof(off_time_str), "%d", off_time_ms);
    led_write_to_file(PWR_TRIGGER_FILE, "timer");
    led_write_to_file(PWR_BRIGHTNESS_FILE, "1");
    led_sleep_ms(50);
    led_write_to_file(PWR_DELAY_OFF_FILE, off_time_str);
    led_write_to_file(PWR_DELAY_ON_FILE, on_time_str);
}

void led_act_set_blink(int on_time_ms, int off_time_ms)
{
    char on_time_str[16];
    snprintf(on_time_str, sizeof(on_time_str), "%d", on_time_ms);
    char off_time_str[16];
    snprintf(off_time_str, sizeof(off_time_str), "%d", off_time_ms);
    led_write_to_file(ACT_TRIGGER_FILE, "timer");
    led_write_to_file(ACT_BRIGHTNESS_FILE, "1");
    led_sleep_ms(50);
    led_write_to_file(ACT_DELAY_OFF_FILE, off_time_str);
    led_write_to_file(ACT_DELAY_ON_FILE, on_time_str);
}