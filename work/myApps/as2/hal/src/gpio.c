#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include "hal/gpio.h"

struct gpiod_chip *chip = NULL;
struct gpiod_line_settings *settings = NULL;
struct gpiod_line_config *line_cfg = NULL;
struct gpiod_line_request *request0 = NULL;
struct gpiod_line_request *request1 = NULL;
struct gpiod_line_request *request2 = NULL;

typedef struct {
    const char *chip_name;
    unsigned int line_offset;
    const char *label_name;
} gpio_map_t;

gpio_map_t pin_map;

static int gpio_map_label_to_gpio(int pin, gpio_map_t *result);

unsigned int chip0_offsets[8] = {0};
int chip_0_offset_count = 0;
unsigned int chip1_offsets[8] = {0};
int chip_1_offset_count = 0;
unsigned int chip2_offsets[8] = {0};
int chip_2_offset_count = 0;

typedef struct 
{
    unsigned int *offsets;
    int *count;
} gpio_info_t;



static gpio_info_t gpio_add_offset(const char chip[], unsigned int offset)
{
    int *c;
    unsigned int *offsets;

    if (chip[8] == '0')
    {
        offsets = chip0_offsets;
        c = &chip_0_offset_count;
    }
    else if (chip[8] == '1')
    {
        offsets = chip1_offsets;
        c = &chip_1_offset_count;
    }
    else
    {
        offsets = chip2_offsets;
        c = &chip_2_offset_count;
    }
    bool already = false;
    for (int i = 0; i < *c; i++)
    {
        if (offset == offsets[*c])
        {
            already = true;
            break;
        }
    }
    if (!already && *c < 8)
    {
        offsets[*c] = offset;
        *c += 1;
    }
    gpio_info_t output;
    output.offsets = offsets;
    output.count = c;

    return output;
}

// initializes gpio communication with the gpio
int gpio_initialize(int pin)
{
    if (chip) {
        gpiod_chip_close(chip);
        chip = NULL; // Set to NULL after freeing
    }
    if (settings) {
        gpiod_line_settings_free(settings);
        settings = NULL;
    }
    if (line_cfg) {
        gpiod_line_config_free(line_cfg);
        line_cfg = NULL;
    }

    gpio_map_label_to_gpio(pin, &pin_map);
    gpio_info_t info = gpio_add_offset(pin_map.chip_name, pin_map.line_offset);
    char chip_path[64];

    // 1. Open the GPIO chip by path
    snprintf(chip_path, sizeof(chip_path), "/dev/%s", pin_map.chip_name);

    chip = gpiod_chip_open(chip_path);
    settings = gpiod_line_settings_new();

    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_bias(settings, GPIOD_LINE_BIAS_PULL_UP);
    line_cfg = gpiod_line_config_new();

    gpiod_line_config_add_line_settings(line_cfg, info.offsets, *info.count, settings);

    if (pin_map.chip_name[8] == '0')
    {
        gpiod_line_request_release(request0);
        request0 = gpiod_chip_request_lines(chip, NULL, line_cfg);
        if (!request0)
        {
            fprintf(stderr, "Error requesting GPIO line 0: %s\n", strerror(errno));
            return 1;
        }
    }
    else if (pin_map.chip_name[8] == '1')
    {
        gpiod_line_request_release(request1);
        request1 = gpiod_chip_request_lines(chip, NULL, line_cfg);
        if (!request1)
        {
            fprintf(stderr, "Error requesting GPIO line 1: %s\n", strerror(errno));
            return 1;
        }
    }
    else
    {
        gpiod_line_request_release(request2);
        request2 = gpiod_chip_request_lines(chip, NULL, line_cfg);
        if (!request2)
        {
            fprintf(stderr, "Error requesting GPIO line 2: %s\n", strerror(errno));
            return 1;
        }
    }

    return 0;
}

// returns if gpio is currently pressed or not
bool gpio_read(int pin)
{
    gpio_map_label_to_gpio(pin, &pin_map);
    enum gpiod_line_value line_value;
    if (pin_map.chip_name[8] == '0')
    {
        line_value = gpiod_line_request_get_value(request0, pin_map.line_offset);
    }
    else if (pin_map.chip_name[8] == '1')
    {
        line_value = gpiod_line_request_get_value(request1, pin_map.line_offset);
    }
    else
    {
        line_value = gpiod_line_request_get_value(request2, pin_map.line_offset);
    }

    if (line_value == GPIOD_LINE_VALUE_ERROR)
    {
        // Check for a recoverable interrupt error
        if (errno == EINTR)
            perror("Error reading GPIO line value using gpiod_line_request_get_value");
    }
    return line_value != GPIOD_LINE_VALUE_INACTIVE;
}

void gpio_disable()
{
    if (request0)
    {
        gpiod_line_request_release(request0);
    }
    if (request1)
    {
        gpiod_line_request_release(request1);
    }
    if (request2)
    {
        gpiod_line_request_release(request2);
    }
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);
    if (chip)
    {
        gpiod_chip_close(chip);
    }
    printf("gpio stopped\n");
}



static int gpio_map_label_to_gpio(int pin, gpio_map_t *result) {
    
    // --- GPIO Chip 0 Mappings ---
    if (pin == 23) {
        result->chip_name = "gpiochip0";
        result->line_offset = 7;
        result->label_name = "GPIO23";
    } else if (pin == 24) {
        result->chip_name = "gpiochip0";
        result->line_offset = 10;
        result->label_name = "GPIO24";
    } else if (pin == 3) {
        result->chip_name = "gpiochip0";
        result->line_offset = 17;
        result->label_name = "GPIO3";
    } else if (pin == 2) {
        result->chip_name = "gpiochip0";
        result->line_offset = 18;
        result->label_name = "GPIO2";
        
    // --- GPIO Chip 1 Mappings ---
    } else if (pin == 27) {
        result->chip_name = "gpiochip1";
        result->line_offset = 33;
        result->label_name = "GPIO27";
    } else if (pin == 26) {
        result->chip_name = "gpiochip1";
        result->line_offset = 36;
        result->label_name = "GPIO26";
    } else if (pin == 4) {
        result->chip_name = "gpiochip1";
        result->line_offset = 38;
        result->label_name = "GPIO4";
    } else if (pin == 22) {
        result->chip_name = "gpiochip1";
        result->line_offset = 41;
        result->label_name = "GPIO22";
    } else if (pin == 25) {
        result->chip_name = "gpiochip1";
        result->line_offset = 42;
        result->label_name = "GPIO25";

    // --- GPIO Chip 2 Mappings ---
    } else if (pin == 16) {
        result->chip_name = "gpiochip2";
        result->line_offset = 7;
        result->label_name = "GPIO16";
    } else if (pin == 17) {
        result->chip_name = "gpiochip2";
        result->line_offset = 8;
        result->label_name = "GPIO17";
    } else if (pin == 21) {
        result->chip_name = "gpiochip2";
        result->line_offset = 9;
        result->label_name = "GPIO21";
    } else if (pin == 20) {
        result->chip_name = "gpiochip2";
        result->line_offset = 10;
        result->label_name = "GPIO20";
    } else if (pin == 18) {
        result->chip_name = "gpiochip2";
        result->line_offset = 11;
        result->label_name = "GPIO18";
    } else if (pin == 19) {
        result->chip_name = "gpiochip2";
        result->line_offset = 12;
        result->label_name = "GPIO19";
    } else if (pin == 15) {
        result->chip_name = "gpiochip2";
        result->line_offset = 13;
        result->label_name = "GPIO15";
    } else if (pin == 14) {
        result->chip_name = "gpiochip2";
        result->line_offset = 14;
        result->label_name = "GPIO14";
    } else if (pin == 5) {
        result->chip_name = "gpiochip2";
        result->line_offset = 15;
        result->label_name = "GPIO5";
    } else if (pin == 12) {
        result->chip_name = "gpiochip2";
        result->line_offset = 16;
        result->label_name = "GPIO12";
    } else if (pin == 6) {
        result->chip_name = "gpiochip2";
        result->line_offset = 17;
        result->label_name = "GPIO6";
    } else if (pin == 13) {
        result->chip_name = "gpiochip2";
        result->line_offset = 18;
        result->label_name = "GPIO13";
    } else {
        // Label not found in the defined inputs
        return -1;
    }

    return 0; // Success
}