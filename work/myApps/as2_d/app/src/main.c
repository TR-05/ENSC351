#include "hal/gpio.h"
#include "hal/time.h"
#include "hal/spi.h"
#include "hal/encoder.h"
#include "hal/pwm.h"
#include "sampler.h"
#include "udp.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

int main()
{
    Sampler_init();
    UDP_init();
    Encoder_init(17, 27, 4);
    pwm_init();
    UDP_join_thread();
    Encoder_cleanup();
    pwm_cleanup();
    return 0;
}