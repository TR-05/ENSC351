#include "hal/encoder.h"
#include "hal/gpio.h"
#include "hal/time.h"

#include <pthread.h>
#include <stdio.h>

static pthread_t EncoderThread;
static pthread_mutex_t EncoderMutex = PTHREAD_MUTEX_INITIALIZER;
static int Apin = 0;
static int Bpin = 0;
static int Buttonpin = 0;
static int count = 0;
static void *Encoder_update_loop()
{
    bool lastA = 1;
    int pulses = 0;

    while (1)
    {
        bool A = gpio_read(27);
        bool B = gpio_read(17);

        if (A != lastA && A == 1)
        {
            if (B != A) // clockwise
            {
                pulses++;
            }
            else // counter clockwise;
            {
                pulses--;
            }
        }
        lastA = A;
        pthread_mutex_lock(&EncoderMutex);
        count = pulses;
        pthread_mutex_unlock(&EncoderMutex);
        //printf("4: %d, 17: %d, 27: %d, p: %d\n", gpio_read(Buttonpin), A, B, count);
        sleep_for_ms(1);
    }
    Encoder_cleanup();
    return 0;
}

int Encoder_init(int pinA, int pinB, int pinButton)
{
    Apin = pinA;
    Bpin = pinB;
    Buttonpin = pinButton;
    gpio_initialize(Apin);
    gpio_initialize(Bpin);
    gpio_initialize(Buttonpin);
    pthread_create(&EncoderThread, NULL, Encoder_update_loop, NULL);

    return 0;
}

int Encoder_read(void)
{
    int val = 0;
    pthread_mutex_lock(&EncoderMutex);
    val = count;
    pthread_mutex_unlock(&EncoderMutex);
    return val;
}

int Encoder_cleanup(void)
{
    pthread_mutex_destroy(&EncoderMutex);
    pthread_cancel(EncoderThread);
    return 0;
}
