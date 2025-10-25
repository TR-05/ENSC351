#include "sampler.h"
#include "hal/time.h"
#include "hal/spi.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t SamplerMutex = PTHREAD_MUTEX_INITIALIZER; /* mutex lock for history */

static double currentAverageLight = 0; // from 0 - 1
static int samplesTaken = 0;
static int samplesTakenLastSecond = 0;
static void *SamplerThreadFunc(void *pArg)
{
    double *volatile_history = ((double *)pArg);
    const long long startTime = get_time_in_ms();
    long long lastTime = startTime;
    int fd;
    bool firstSample = true;
    int sampleCount = 0;
    spi_initialize(&fd, "/dev/spidev0.0", 0, 8, 250000);
    while (1)
    {
        long long currentTime = get_time_in_ms();

        // Sample from photocell
        int index = (currentTime - lastTime) % 1000;
        volatile_history[index] = (spi_read_mcp3208_channel(4, &fd)) * 5.0 / 4095.0; // convert to voltage by by (read value from 0 - 1) * VRef
        sampleCount++;
        if (firstSample)
        {
            currentAverageLight = volatile_history[index];
            firstSample = false;
        }
        else
        {
            currentAverageLight = volatile_history[index] * (1 - .999) + .999 * currentAverageLight;
        }

        // every 1 second transfer data
        if (currentTime - lastTime > 1000)
        {
            printf("transfer now: %.1f, %.2f\n", (currentTime - startTime) / 1000.0f, currentAverageLight);
            lastTime = get_time_in_ms();

            pthread_mutex_lock(&SamplerMutex);
            samplesTakenLastSecond = sampleCount;
            Sampler_moveCurrentDataToHistory();
            samplesTaken += sampleCount;

            pthread_mutex_unlock(&SamplerMutex);

            sampleCount = 0;
        }

        sleep_for_ms(1);
    }
    spi_disable(&fd);
    return 0;
}
pthread_t tidSampler;
static double volatile_history[1000]; // updates every 1ms using a circular buffer
static double stable_history[1000];   // updates every 1 second
void Sampler_init(void)
{
    pthread_create(&tidSampler, NULL, SamplerThreadFunc, &volatile_history);
    // pthread_join(tidSampler, 0);
}

void Sampler_cleanup(void)
{
    pthread_mutex_destroy(&SamplerMutex);
    pthread_exit(0);
}

// Must be called once every 1s.
// Moves the samples that it has been collecting this second into
// the history, which makes the samples available for reads (below).
void Sampler_moveCurrentDataToHistory(void)
{
    for (int i = 0; i < samplesTakenLastSecond; i++)
    {
        stable_history[i] = volatile_history[i];
    }
}

// Get the number of samples collected during the previous complete second.
int Sampler_getHistorySize(void)
{
    pthread_mutex_lock(&SamplerMutex);
    int num = samplesTakenLastSecond;
    pthread_mutex_unlock(&SamplerMutex);
    return num;
}
// Get a copy of the samples in the sample history.
// Returns a newly allocated array and sets `size` to be the
// number of elements in the returned array (output-only parameter).
// The calling code must call free() on the returned pointer.
// Note: It provides both data and size to ensure consistency.
double *Sampler_getHistory(int *size)
{
    printf("size: %d\n", *size);
    double *newHistory = malloc(sizeof(double) * *size);
    pthread_mutex_lock(&SamplerMutex);
    for (int i = 0; i < samplesTakenLastSecond; i++)
    {
        newHistory[i] = stable_history[i];
    }

    pthread_mutex_unlock(&SamplerMutex);
    return newHistory;
}
// Get the average light level (not tied to the history).
double Sampler_getAverageReading(void)
{
    pthread_mutex_lock(&SamplerMutex);
    double average = currentAverageLight;
    pthread_mutex_unlock(&SamplerMutex);
    return average;
}
// Get the total number of light level samples taken so far.
long long Sampler_getNumSamplesTaken(void)
{
    pthread_mutex_lock(&SamplerMutex);
    long long samples = samplesTaken;
    pthread_mutex_unlock(&SamplerMutex);
    return samples;
}