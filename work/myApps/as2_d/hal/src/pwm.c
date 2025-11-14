#include "hal/pwm.h"
#include "hal/time.h"
#include "hal/encoder.h"

#include <pthread.h>
#include <stdio.h>
#include <math.h>

static pthread_t pwmThread;
static pthread_mutex_t pwmMutex = PTHREAD_MUTEX_INITIALIZER;

#define MIN_HZ 0
#define MAX_HZ 500

static int frequency = 10; // default to 10 Hz
static int lastFrequency = 0;

static void pwm_write_to_file(char *file_address, char *data)
{
    FILE *file = fopen(file_address, "w");
    if (file == NULL)
    {
        perror("Error opening pwm file");
    }

    int charWritten = fprintf(file, "%s", data);
    if (charWritten <= 0)
    {
        perror("Error writing data to pwm file");
    }
    fclose(file);
    sleep_for_ms(2);
}

bool state = false;
void togglePWM(void)
{
    pwm_write_to_file("/dev/hat/pwm/GPIO5/period", "1000000");
    if (state)
    {
        pwm_write_to_file("/dev/hat/pwm/GPIO5/enable", "1");
    }
    else
    {
        pwm_write_to_file("/dev/hat/pwm/GPIO5/enable", "0");
    }
    pwm_write_to_file("/dev/hat/pwm/GPIO5/duty_cycle", "1000000");
    state = !state;
}

static void *pwm_update_loop()
{
    int lastPulseCount = 0;
    long long startTime = get_time_in_ms();
    while (1)
    {
        int newPulseCount = Encoder_read();
        int delta = newPulseCount - lastPulseCount;
        pthread_mutex_lock(&pwmMutex);
        frequency += delta;
        if (frequency <= MIN_HZ)
        {
            frequency = MIN_HZ;
        }
        else if (frequency >= MAX_HZ)
        {
            frequency = MAX_HZ;
        }

        if (frequency != lastFrequency && frequency > 3)
        {
            pwm_set_frequency(frequency);
        }
        else if (frequency <= 3 && frequency > 0)
        {
            long long dt = get_time_in_ms() - startTime;
            long long period = floor(1000 / (double)frequency);
            if (dt + 30 > period) // togglePWM takes ~30 ms (3 calls of write with 10 ms sleep each)
            {
                startTime = get_time_in_ms();
                togglePWM();
            }
        }
        else if (frequency == 0)
        {
            pwm_write_to_file("/dev/hat/pwm/GPIO5/period", "1000000");
            pwm_write_to_file("/dev/hat/pwm/GPIO5/enable", "0");
            pwm_write_to_file("/dev/hat/pwm/GPIO5/duty_cycle", "1000000");
        }

        lastFrequency = frequency;
        lastPulseCount = newPulseCount;
        pthread_mutex_unlock(&pwmMutex);
        sleep_for_ms(10);
    }
    pwm_cleanup();
    return 0;
}

void pwm_init(void)
{

    pthread_create(&pwmThread, NULL, pwm_update_loop, NULL);
    pwm_write_to_file("/dev/hat/pwm/GPIO5/period", "1000000");
    pwm_write_to_file("/dev/hat/pwm/GPIO5/enable", "1");
    pwm_write_to_file("/dev/hat/pwm/GPIO5/duty_cycle", "0");
}

void pwm_set_frequency(int newFrequency)
{
    if (newFrequency == 0)
    {
        pwm_write_to_file("/dev/hat/pwm/GPIO5/duty_cycle", "0");
    }
    else
    {
        double periodFloatingPoint = (1 / (double)newFrequency) * 1000000000; // in nanoseconds
        long long newPeriod = floor(periodFloatingPoint);
        long long newDutyCycle = (double)(0.5 * newPeriod);
        //printf("freq: %d, newPeriod: %llu, newDuty: %llu\n", newFrequency, newPeriod, newDutyCycle);
        pwm_write_to_file("/dev/hat/pwm/GPIO5/duty_cycle", "0");

        char period_str[32];
        snprintf(period_str, sizeof(period_str), "%llu", newPeriod);
        char duty_str[32];
        snprintf(duty_str, sizeof(duty_str), "%llu", newDutyCycle);

        pwm_write_to_file("/dev/hat/pwm/GPIO5/period", period_str);
        pwm_write_to_file("/dev/hat/pwm/GPIO5/duty_cycle", duty_str);
        pwm_write_to_file("/dev/hat/pwm/GPIO5/enable", "1");
    }
}

long long pwm_get_frequency(void)
{
    long long val = 0;
    pthread_mutex_lock(&pwmMutex);
    val = frequency;
    pthread_mutex_unlock(&pwmMutex);
    return val;
}

void pwm_cleanup(void)
{
    pwm_write_to_file("/dev/hat/pwm/GPIO5/duty_cycle", "0");
    pwm_write_to_file("/dev/hat/pwm/GPIO5/enable", "0");
    pthread_mutex_destroy(&pwmMutex);
    pthread_cancel(pwmThread);
}
