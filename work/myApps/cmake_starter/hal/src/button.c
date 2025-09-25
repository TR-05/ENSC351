#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "hal/button.h"
#include "hal/gpio_map.h"

// --- Configuration Constants ---

// ONLY WORKS FOR GPIO INPUT FOR NOW
#define NUM_LINES 1
#define BUTTON_PIN "GPIO4"

struct gpiod_chip *chip = NULL;
struct gpiod_line_settings *settings = NULL;
struct gpiod_line_config *line_cfg = NULL;
struct gpiod_line_request *request = NULL;

gpio_map_t pin_map;


unsigned int offsets[NUM_LINES] = {0};

// initializes gpio communication with the button
int button_initialize(void)
{
    map_label_to_gpio(BUTTON_PIN, &pin_map);

    char chip_path[64];
    offsets[0] = pin_map.line_offset;
    printf("Starting GPIO monitoring (libgpiod read) for line: %s offset %d (GPIO4)\n",
           pin_map.chip_name, offsets[0] );

    // 1. Open the GPIO chip by path
    snprintf(chip_path, sizeof(chip_path), "/dev/%s", pin_map.chip_name);

    chip = gpiod_chip_open(chip_path);
    if (!chip)
    {
        perror("Error opening GPIO chip. Check if the path is correct.");
        return 1;
    }

    // 2. Create and configure line settings (Input + Pull-up)
    settings = gpiod_line_settings_new();
    if (!settings)
    {
        perror("Error creating line settings");
        return 1;
    }

    // Set direction to input and enable internal pull-up bias
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_bias(settings, GPIOD_LINE_BIAS_PULL_UP);

    // 3. Create line configuration and add the settings for our offset
    line_cfg = gpiod_line_config_new();
    if (!line_cfg)
    {
        perror("Error creating line config");
        return 1;
    }
    gpiod_line_config_add_line_settings(line_cfg, offsets, NUM_LINES, settings);

    // 4. Request the line(s) for exclusive usage
    request = gpiod_chip_request_lines(chip, NULL, line_cfg);
    if (!request)
    {
        fprintf(stderr, "Error requesting GPIO line: %s\n", strerror(errno));
        return 1;
    }

    // 5. Main monitoring loop: Use the gpiod_line_request_get_value function
    printf("Monitoring... Press Ctrl+C to exit.\n");
    return 0;
}

static bool last_read = false;
// returns if button is currently pressed or not
bool button_pressing(void)
{
    // Use the single-line read function, passing the offset of the line within the request.
    enum gpiod_line_value line_value = gpiod_line_request_get_value(request, offsets[0]);
    if (line_value == GPIOD_LINE_VALUE_ERROR)
    {
        // Check for a recoverable interrupt error
        if (errno == EINTR)
            perror("Error reading GPIO line value using gpiod_line_request_get_value");
    }

    // Output based on the reading
    if (line_value == GPIOD_LINE_VALUE_INACTIVE)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// returns 1 if it detects a rising edge, 0 otherwise
bool button_rising_edge(void)
{
    bool cur = button_pressing();
    bool rising = cur && !last_read;
    last_read = cur;
    return rising;
}

// returns 1 if it detects a falling edge, 0 otherwise
bool button_falling_edge(void)
{
    bool cur = button_pressing();
    bool falling = !cur && last_read;
    last_read = cur;
    return falling;
}

void button_disable()
{
    if (request)
    {
        gpiod_line_request_release(request);
    }
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);
    if (chip)
    {
        gpiod_chip_close(chip);
    }
    printf("button stopped\n");
}