#include <gpiod.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "hal/gpio_map.h"

/**
 * @brief Maps a standard GPIO label string (e.g., "GPIO4") to the
 * correct libgpiod chip name and line offset.
 * * @param label_string The human-readable label (e.g., "GPIO21").
 * @param result Pointer to a gpio_map_t structure to store the output.
 * @return 0 on success (mapping found), -1 on failure (label not found).
 */
int map_label_to_gpio(const char *label_string, gpio_map_t *result) {
    if (!result || !label_string) return -1;

    // Use strcmp for string matching to determine the chip and offset
    // based directly on your gpioinfo output.
    
    // --- GPIO Chip 0 Mappings ---
    if (strcmp(label_string, "GPIO23") == 0) {
        result->chip_name = "gpiochip0";
        result->line_offset = 7;
        result->label_name = "GPIO23";
    } else if (strcmp(label_string, "GPIO24") == 0) {
        result->chip_name = "gpiochip0";
        result->line_offset = 10;
        result->label_name = "GPIO24";
    } else if (strcmp(label_string, "GPIO3") == 0) {
        result->chip_name = "gpiochip0";
        result->line_offset = 17;
        result->label_name = "GPIO3";
    } else if (strcmp(label_string, "GPIO2") == 0) {
        result->chip_name = "gpiochip0";
        result->line_offset = 18;
        result->label_name = "GPIO2";
        
    // --- GPIO Chip 1 Mappings ---
    } else if (strcmp(label_string, "GPIO27") == 0) {
        result->chip_name = "gpiochip1";
        result->line_offset = 33;
        result->label_name = "GPIO27";
    } else if (strcmp(label_string, "GPIO26") == 0) {
        result->chip_name = "gpiochip1";
        result->line_offset = 36;
        result->label_name = "GPIO26";
    } else if (strcmp(label_string, "GPIO4") == 0) {
        result->chip_name = "gpiochip1";
        result->line_offset = 38;
        result->label_name = "GPIO4";
    } else if (strcmp(label_string, "GPIO22") == 0) {
        result->chip_name = "gpiochip1";
        result->line_offset = 41;
        result->label_name = "GPIO22";
    } else if (strcmp(label_string, "GPIO25") == 0) {
        result->chip_name = "gpiochip1";
        result->line_offset = 42;
        result->label_name = "GPIO25";

    // --- GPIO Chip 2 Mappings ---
    } else if (strcmp(label_string, "GPIO16") == 0) {
        result->chip_name = "gpiochip2";
        result->line_offset = 7;
        result->label_name = "GPIO16";
    } else if (strcmp(label_string, "GPIO17") == 0) {
        result->chip_name = "gpiochip2";
        result->line_offset = 8;
        result->label_name = "GPIO17";
    } else if (strcmp(label_string, "GPIO21") == 0) {
        result->chip_name = "gpiochip2";
        result->line_offset = 9;
        result->label_name = "GPIO21";
    } else if (strcmp(label_string, "GPIO20") == 0) {
        result->chip_name = "gpiochip2";
        result->line_offset = 10;
        result->label_name = "GPIO20";
    } else if (strcmp(label_string, "GPIO18") == 0) {
        result->chip_name = "gpiochip2";
        result->line_offset = 11;
        result->label_name = "GPIO18";
    } else if (strcmp(label_string, "GPIO19") == 0) {
        result->chip_name = "gpiochip2";
        result->line_offset = 12;
        result->label_name = "GPIO19";
    } else if (strcmp(label_string, "GPIO15") == 0) {
        result->chip_name = "gpiochip2";
        result->line_offset = 13;
        result->label_name = "GPIO15";
    } else if (strcmp(label_string, "GPIO14") == 0) {
        result->chip_name = "gpiochip2";
        result->line_offset = 14;
        result->label_name = "GPIO14";
    } else if (strcmp(label_string, "GPIO5") == 0) {
        result->chip_name = "gpiochip2";
        result->line_offset = 15;
        result->label_name = "GPIO5";
    } else if (strcmp(label_string, "GPIO12") == 0) {
        result->chip_name = "gpiochip2";
        result->line_offset = 16;
        result->label_name = "GPIO12";
    } else if (strcmp(label_string, "GPIO6") == 0) {
        result->chip_name = "gpiochip2";
        result->line_offset = 17;
        result->label_name = "GPIO6";
    } else if (strcmp(label_string, "GPIO13") == 0) {
        result->chip_name = "gpiochip2";
        result->line_offset = 18;
        result->label_name = "GPIO13";
    } else {
        // Label not found in the defined inputs
        return -1;
    }

    return 0; // Success
}