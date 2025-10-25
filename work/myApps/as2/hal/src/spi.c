#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "hal/spi.h"

#define DEBUG_MODE false

// default values, shouldn't ever be used as must initialze before doing a transfer but idk
static uint8_t mode = 0;        // SPI mode: Mode 0 (CPOL=0, CPHA=0)
static uint8_t bits = 8;        // Bits per word
static uint32_t speed = 250000; // Hz

int spi_initialize(int* fd, char* device, uint8_t spi_mode, uint8_t bits_per_word, uint32_t speed_hz) {
    *fd = open(device, O_RDWR);
    if (*fd < 0)
    {
        perror("open");
        return 1;
    }
    if (ioctl(*fd, SPI_IOC_WR_MODE, &spi_mode) == -1)
    {
        perror("mode");
        return 1;
    }
    mode = spi_mode;
    if (ioctl(*fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) == -1)
    {
        perror("bpw");
        return 1;
    }
    bits = bits_per_word;
    if (ioctl(*fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed_hz) == -1)
    {
        perror("speed");
        return 1;
    }
    speed = speed_hz;
    #if DEBUG_MODE == 1
    printf("SPI device opened successfully at %s\n", device);
    printf("SPI Mode: %d, Bits per Word: %d, Speed: %d Hz\n", mode, bits, speed);
    #endif
    return 0;
}

int spi_transfer(int fd, uint8_t *tx, uint8_t *rx, size_t length)
{
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = length,
        .speed_hz = speed,
        .bits_per_word = bits,
        .cs_change = 0};

    int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < -1)
    {
        perror("Can't send SPI message");
    }
    return ret;
}

int spi_read_mcp3208_channel(uint8_t channel, int* fd)
{
    if (channel > 7)
    {
        fprintf(stderr, "Invalid channel specified. MCP3208 has 8 channels (0-7).\n");
        return -1;
    }

    uint8_t tx[3] = {(uint8_t)(0x06 | ((channel & 0x04) >> 2)),
                     (uint8_t)((channel & 0x03) << 6),
                     0x00};
    uint8_t rx[3] = {0};

    int bytes_read = spi_transfer(*fd, tx, rx, sizeof(tx));

    if (bytes_read > 0)
    {
        // The 12-bit value is split across the two received bytes (rx_buffer[1] and rx_buffer[2]).
        // The upper 4 bits of the value are in rx_buffer[1], and the lower 8 bits are in rx_buffer[2].
        // The `& 0x0F` masks off the junk bits in the first data byte.
        return ((rx[1] & 0x0F) << 8) | rx[2];
    }

    return -1; // Return -1 on failure
}

void spi_disable(int* fd) {
    close(*fd);
}


