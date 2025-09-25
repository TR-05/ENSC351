#ifndef GPIO_MAP_H
#define GPIO_MAP_H

/*
Header file mapping the board label GPIO pins to what the os needs
*/

// --- GPIO Pinout Mapping Structure ---
// Holds the libgpiod-specific identification data
typedef struct {
    const char *chip_name;
    unsigned int line_offset;
    const char *label_name;
} gpio_map_t;


/**
 * @brief Maps a standard GPIO label string (e.g., "GPIO4") to the
 * correct libgpiod chip name and line offset.
 * * @param label_string The human-readable label (e.g., "GPIO21").
 * @param result Pointer to a gpio_map_t structure to store the output.
 * @return 0 on success (mapping found), -1 on failure (label not found).
 */
int map_label_to_gpio(const char *label_string, gpio_map_t *result);


#endif // GPIO_MAP_H