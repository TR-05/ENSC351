#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include "hal/led.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "hal/joystick.h"

// JOYSTICK CONFIGURATION:
#define X_CHANNEL 6
#define Y_CHANNEL 5

static int x_max_val = 3071;
static int x_mid_val = 2550;
static int x_min_val = 2048;

static int y_max_val = 3071;
static int y_mid_val = 2556;
static int y_min_val = 2048;

// SPI methods and constants:

// SPI device path
static const char *device = "/dev/spidev0.0";
// SPI mode: Mode 0 (CPOL=0, CPHA=0)
static uint8_t mode = 0;
// Bits per word
static uint8_t bits = 8;
// SPI speed in Hz
static uint32_t speed = 1000000; // 1 MHz

// Function to perform an SPI transfer
static int spi_transfer(int fd, uint8_t *tx_buffer, uint8_t *rx_buffer, size_t length)
{
    // Structure for the SPI transfer message
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buffer, // Transmit buffer
        .rx_buf = (unsigned long)rx_buffer, // Receive buffer
        .len = length,                      // Length of the transfer
        .speed_hz = speed,                  // SPI speed
        .bits_per_word = bits,              // Bits per word
    };

    // Perform the transfer using ioctl
    int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
    {
        perror("Can't send SPI message");
    }
    return ret;
}

// Function to read a single channel from the MCP3208 ADC
static int read_mcp3208_channel(int fd, uint8_t channel)
{
    if (channel > 7)
    {
        fprintf(stderr, "Invalid channel specified. MCP3208 has 8 channels (0-7).\n");
        return -1;
    }

    // The MCP3208 command for single-ended mode is a 3-byte transfer.
    // Tx: [Start bit], [SGL/DIF bit + Channel], [Dummy byte]
    // Rx: [Junk], [Data MSB], [Data LSB]
    uint8_t tx_buffer[3] = {0};
    uint8_t rx_buffer[3] = {0};
    int bytes_read;
    int adc_value;

    // Byte 1: Start bit (0x01)
    tx_buffer[0] = 0x01;

    // Byte 2: Single-ended bit (0x80) and channel bits
    // The single-ended bit `0x80` (0b10000000) is combined with the channel
    // number shifted to the most significant nibble.
    tx_buffer[1] = 0x80 | (channel << 4);

    // Byte 3: Dummy byte to clock out the remaining data
    tx_buffer[2] = 0x00;

    // Perform the transfer
    bytes_read = spi_transfer(fd, tx_buffer, rx_buffer, sizeof(tx_buffer));

    if (bytes_read > 0)
    {
        // The 12-bit value is split across the two received bytes (rx_buffer[1] and rx_buffer[2]).
        // The upper 4 bits of the value are in rx_buffer[1], and the lower 8 bits are in rx_buffer[2].
        // The `& 0x0F` masks off the junk bits in the first data byte.
        adc_value = ((rx_buffer[1] & 0x0F) << 8) | rx_buffer[2];
        return adc_value;
    }

    return -1; // Return -1 on failure
}

// JOYSTICK methods
static int fd;
int joystick_initialize(void)
{
    int ret = 0;

    // Open the SPI device file
    fd = open(device, O_RDWR);
    if (fd < 0)
    {
        perror("Error opening SPI device");
        return 1;
    }

    // Set SPI mode
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
    {
        perror("Can't set SPI mode");
        close(fd);
        return 1;
    }

    // Set bits per word
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
    {
        perror("Can't set bits per word");
        close(fd);
        return 1;
    }

    // Set SPI speed
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
    {
        perror("Can't set SPI speed");
        close(fd);
        return 1;
    }

    printf("SPI device opened successfully at %s\n", device);
    printf("SPI Mode: %d, Bits per Word: %d, Speed: %d Hz\n", mode, bits, speed);
    printf("Reading MCP3208 channels 6 and 7. Press Ctrl+C to exit...\n");
    return 0;
}

// gets the raw 12 bit data from the joystick
float joystick_get_x_raw(void)
{
    return (float)(read_mcp3208_channel(fd, X_CHANNEL));
}
float joystick_get_y_raw(void)
{
    return (float)(read_mcp3208_channel(fd, Y_CHANNEL));
}

// gets joystick data normalized from -1 to 1
float joystick_get_x_normalized(void)
{
    return (joystick_get_x_raw() - x_mid_val) / ((x_max_val - x_min_val) / 2.0f);
}
float joystick_get_y_normalized(void)
{
    return (joystick_get_y_raw() - y_mid_val) / ((y_max_val - y_min_val) / 2.0f);
}

// override the default calibration values, may be useful?
void joystick_configure_x(int max, int mid, int min)
{
    x_max_val = max;
    x_mid_val = mid;
    x_min_val = min;
}
void joystick_configure_y(int max, int mid, int min)
{
    y_max_val = max;
    y_mid_val = mid;
    y_min_val = min;
}

void joystick_disable(void)
{
    close(fd);
}
