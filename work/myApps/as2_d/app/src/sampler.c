#include "sampler.h"
#include "periodTimer.h"
#include "hal/time.h"
#include "hal/spi.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "hal/pwm.h"

#define SAMPLE_BUFFER_SIZE 1000
const bool display = true;

static pthread_mutex_t SamplerMutex = PTHREAD_MUTEX_INITIALIZER; /* mutex lock for history */

static double currentAverageLight = 0; // from 0 - 1
static int samplesTaken = 0;
static int samplesTakenLastSecond = 0;

static double stable_history[SAMPLE_BUFFER_SIZE]; // updates every 1 second

static void Sampler_moveCurrentDataToHistory(double *history, int size);
int Sampler_getDips(void);

static void *SamplerThreadFunc()
{
    const long long startTime = get_time_in_ms();
    long long lastTime = startTime;
    int fd;
    bool firstSample = true;
    int sampleCount = 0;
    spi_initialize(&fd, "/dev/spidev0.0", 0, 8, 250000);
    double volatile_history[1500]; // should only need 1000 but why not
    Period_init();
    while (1)
    {
        long long currentTime = get_time_in_ms();

        // Sample from photocell
        sampleCount++;
        int index = sampleCount - 1;
        volatile_history[index] = (spi_read_mcp3208_channel(4, &fd)) * 1.8 / 4095.0; // convert to voltage by (read value from 0 - 1) * VRef
        Period_markEvent(PERIOD_EVENT_SAMPLE_LIGHT);
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
            lastTime = get_time_in_ms();
            double displaySamples[10];
            int displaySampleIndices[10];
            pthread_mutex_lock(&SamplerMutex);
            samplesTakenLastSecond = sampleCount;
            samplesTaken += sampleCount;
            if (samplesTakenLastSecond < 10)
            {
                for (int i = 0; i < samplesTakenLastSecond; i++)
                {
                    displaySamples[i] = volatile_history[i];
                    displaySampleIndices[i] = i;
                }
            }
            else
            {
                for (int i = 0; i < 10; i++)
                {
                    displaySamples[i] = volatile_history[(int)floor(i * (samplesTakenLastSecond / 10.0f))];
                    displaySampleIndices[i] = (int)floor(i * (samplesTakenLastSecond / 10.0f));
                }
            }

            Sampler_moveCurrentDataToHistory(volatile_history, samplesTakenLastSecond);
            int s = samplesTakenLastSecond;
            double avg = currentAverageLight;
            pthread_mutex_unlock(&SamplerMutex);

            // print out every 1 second
            Period_statistics_t stats;
            Period_getStatisticsAndClear(PERIOD_EVENT_SAMPLE_LIGHT, &stats);

            if (display)
            {
                printf("#Smpl/s = %3d   flash @ %3lluHz   avg = %5.3fV   dips = %3d   Smpl ms[%5.3f,%5.3f] avg %5.3f/%3d\n", s, pwm_get_frequency(), avg, Sampler_getDips(), stats.minPeriodInMs, stats.maxPeriodInMs, stats.avgPeriodInMs, stats.numSamples);

                printf(" ");
                if (s < 10)
                {
                    for (int i = 0; i < s; i++)
                    {
                        printf("%d:%5.3f ", displaySampleIndices[i], displaySamples[i]);
                    }
                }
                else
                {
                    for (int i = 0; i < 10; i++)
                    {
                        printf("%d:%5.3f ", displaySampleIndices[i], displaySamples[i]);
                    }
                }
                printf("\n");
            }

            sampleCount = 0;
        }

        sleep_for_ms(1);
    }
    spi_disable(&fd);
    Period_cleanup();
    return 0;
}
pthread_t tidSampler;
void Sampler_init(void)
{
    pthread_create(&tidSampler, NULL, SamplerThreadFunc, NULL);
}

void Sampler_cleanup(void)
{
    pthread_mutex_destroy(&SamplerMutex);
    pthread_cancel(tidSampler);
}

// Must be called once every 1s.
// Moves the samples that it has been collecting this second into
// the history, which makes the samples available for reads (below).
static void Sampler_moveCurrentDataToHistory(double *history, int size)
{
    for (int i = 0; i < size; i++)
    {
        stable_history[i] = history[i];
    }
}

int Sampler_getDips(void)
{
    int dips = 0;
    int size = 0;
    double *history = Sampler_getHistory(&size);
    double avg = Sampler_getAverageReading();
    bool inADip = false;
    for (int i = 0; i < size; i++)
    {
        double s = history[i];
        if (s <= avg - 0.1 && !inADip)
        {
            dips++;
            inADip = true;
        }
        else if (s >= avg - 0.07)
        {
            inADip = false;
        }
    }
    free(history);
    return dips;
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
    *size = samplesTakenLastSecond;
    double *newHistory = malloc(sizeof(double) * samplesTakenLastSecond);
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