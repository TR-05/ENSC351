#include "hal/time.h"
#include "hal/spi.h"
#include "udp.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include "sampler.h"
#include <math.h>

#define MAX_LEN 1500
#define PORT 12345

pthread_mutex_t UDPmutex = PTHREAD_MUTEX_INITIALIZER; /* mutex lock for history */

pthread_t tidUDP;

static bool exitSignal = false;
static int socketDescriptor;
static struct sockaddr_in sinRemote;

static void UDP_process_message(char command[]);

static void *UDPThreadFunc(void *pArg)
{
    bool *exitSignal = ((bool *)pArg);
    while (1)
    {
        // recieve a message
        unsigned int sin_len = sizeof(sinRemote);
        char messageRx[MAX_LEN];
        int bytesRx = recvfrom(socketDescriptor, messageRx, MAX_LEN - 1, 0, (struct sockaddr *)&sinRemote, &sin_len);
        if (bytesRx < MAX_LEN - 1)
        {
            messageRx[bytesRx - 1] = 0;
        }
        else
        {
            messageRx[MAX_LEN - 1] = 0;
        }

        printf("Message Recieved(%d bytes): '%s'\n", bytesRx, messageRx);
        UDP_process_message(messageRx);

        *exitSignal = false;
        sleep_for_ms(100);
    }
    UDP_cleanup();
    return 0;
}

void UDP_init(void)
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(PORT);

    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    bind(socketDescriptor, (struct sockaddr *)&sin, sizeof(sin));

    pthread_create(&tidUDP, NULL, UDPThreadFunc, &exitSignal);
    pthread_join(tidUDP, 0);
}

void UDP_cleanup(void)
{
    close(socketDescriptor);
    pthread_mutex_destroy(&UDPmutex);
    pthread_exit(0);
}

static void UDP_send_message(char message[])
{
    // send a message
    unsigned int sin_len = sizeof(sinRemote);
    sendto(socketDescriptor, message, strlen(message), 0, (struct sockaddr *)&sinRemote, sin_len);
}

static char prevCommand[MAX_LEN];
static char *helpMessage = "Accepted command examples:"
                           "count -- get the total number of samples taken.\n"
                           "length -- get the number of samples taken in the previously completed second.\n"
                           "dips -- get the number of dips in the previously completed second.\n"
                           "history -- get all the samples in the previously completed second.\n"
                           "stop -- cause the server program to end.\n"
                           "<enter> -- repeat last command.\n"
                           "?"
                           "Accepted command examples:\n"
                           "count -- get the total number of samples taken.\n"
                           "length -- get the number of samples taken in the previously completed second.\n"
                           "dips -- get the number of dips in the previously completed second.\n"
                           "history -- get all the samples in the previously completed second.\n"
                           "stop -- cause the server program to end.\n"
                           "<enter> -- repeat last command.\n";

// static char prevReturnCommand[MAX_LEN];
static void UDP_process_message(char command[])
{
    char returnMessage[MAX_LEN];
    snprintf(returnMessage, MAX_LEN, "Hello%d\n", 42);

    if (strcmp(command, "help") == 0 || strcmp(command, "?") == 0)
    {

        strncpy(returnMessage, helpMessage, MAX_LEN - 1);
        returnMessage[MAX_LEN - 1] = '\0';
        UDP_send_message(returnMessage);
    }
    else if (strcmp(command, "count") == 0)
    {
        snprintf(returnMessage, MAX_LEN, "# samples taken total: %llu\n", Sampler_getNumSamplesTaken());
        returnMessage[MAX_LEN - 1] = '\0';
        UDP_send_message(returnMessage);
    }
    else if (strcmp(command, "length") == 0)
    {
        snprintf(returnMessage, MAX_LEN, "# samples taken total: %d\n", Sampler_getHistorySize());
        returnMessage[MAX_LEN - 1] = '\0';
        UDP_send_message(returnMessage);
    }
    else if (strcmp(command, "dips") == 0)
    {
        UDP_send_message("return how many dips were detected in the last second (?)");
    }
    else if (strcmp(command, "history") == 0)
    {
        int size = 0;
        double* history = Sampler_getHistory(&size);
        int samplesPerPacket = floor(1500 / sizeof(double));
        int packets = ceil(size / (double)samplesPerPacket);
        char* initial_str;
        char* packet_str;
        for (int i = 0; i < packets; i++) {
            for (int j = 0; j < samplesPerPacket; j++) {
                char* sampleData;
                snprintf(sampleData, MAX_LEN, "%.3f,", history[i*samplesPerPacket + j]);
                int str_len = strlen(packet_str);
                free(packet_str);
                packet_str = (char *)malloc(str_len + strlen(sampleData) + 1);
                strcpy(packet_str, initial_str);
                strcat(packet_str, sampleData);
                packet_str = (char *)malloc(strlen(packet_str) + 1);
                strcpy(initial_str, packet_str);
            }
            UDP_send_message(packet_str);
        }


        UDP_send_message("return all data samples from previous second, see pdf for details");
        free(history);
    }
    else if (strcmp(command, "") == 0)
    {
        if (strcmp(command, prevCommand) == 0 && strcmp(command, "") != 0)
        {
            UDP_send_message("repeat previous command (if not first command!)");
        }
    }
    else if (strcmp(command, "stop") == 0)
    {
        UDP_send_message("exit the program gracefully, closing all open sockets threads dynamic memmory, etc");
    }

    for (size_t i = 0; i < MAX_LEN - 1; i++)
    {
        if (i < strlen(command) - 1)
        {
            prevCommand[i] = command[i];
        }
        else
        {
            prevCommand[i] = 0;
        }
    }
}