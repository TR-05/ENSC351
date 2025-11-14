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
#define MAX_PACKET_SIZE 1500
#define MAX_SAMPLE_STR_LEN 30

static pthread_mutex_t UDPmutex = PTHREAD_MUTEX_INITIALIZER; /* mutex lock for history */

static pthread_t tidUDP;

static bool exitSignal = false;
static int socketDescriptor;
static struct sockaddr_in sinRemote;

static int UDP_process_message(char command[]);

void UDP_join_thread(void) {
    pthread_join(tidUDP, NULL);
}
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

        //printf("Message Recieved(%d bytes): '%s'\n", bytesRx, messageRx);
        *exitSignal = UDP_process_message(messageRx);

        sleep_for_ms(100);
        if (*exitSignal)
        {
            break;
        }
    }
    Sampler_cleanup();
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
}

void UDP_cleanup(void)
{
    close(socketDescriptor);
    pthread_mutex_destroy(&UDPmutex);
    pthread_cancel(tidUDP);
}

static void UDP_send_message(char message[])
{
    // send a message
    unsigned int sin_len = sizeof(sinRemote);
    sendto(socketDescriptor, message, strlen(message), 0, (struct sockaddr *)&sinRemote, sin_len);
}

static char prevCommand[MAX_LEN];
static char *helpMessage = "\nAccepted command examples:\n"
                           "count      -- get the total number of samples taken.\n"
                           "length     -- get the number of samples taken in the previously completed second.\n"
                           "dips       -- get the number of dips in the previously completed second.\n"
                           "history    -- get all the samples in the previously completed second.\n"
                           "stop       -- cause the server program to end.\n"
                           "<enter>    -- repeat last command.\n";

static char prevCommand[MAX_LEN] = "help";
static int UDP_process_message(char command[])
{
    char returnMessage[MAX_LEN];
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
        snprintf(returnMessage, MAX_LEN, "# samples taken last second: %d\n", Sampler_getHistorySize());
        returnMessage[MAX_LEN - 1] = '\0';
        UDP_send_message(returnMessage);
    }
    else if (strcmp(command, "dips") == 0)
    {
        snprintf(returnMessage, MAX_LEN, "# Dips: %d\n", Sampler_getDips());
        returnMessage[MAX_LEN - 1] = '\0';
        UDP_send_message(returnMessage);
    }
    else if (strcmp(command, "history") == 0)
    {
        int size = 0;
        double *history = Sampler_getHistory(&size);
        char packet_str[MAX_PACKET_SIZE + 1];
        int history_index = 0;

        while (history_index < size)
        {
            packet_str[0] = '\0';
            int current_len = 0;

            while (history_index < size)
            {
                char sample_str[MAX_SAMPLE_STR_LEN];
                snprintf(sample_str, MAX_SAMPLE_STR_LEN, "%.3f, ", history[history_index]);
                if ((history_index+1) % 10 == 0) {
                    strcat(sample_str, "\n");
                }
                int sample_str_len = strlen(sample_str);
                if (current_len + sample_str_len >= MAX_PACKET_SIZE)
                {
                    break;
                }

                strcat(packet_str, sample_str);

                current_len += sample_str_len;
                history_index++;
            }

            if (current_len > 0 && packet_str[current_len - 1] == ',')
            {
                packet_str[current_len - 1] = '\0';
            }

            if (packet_str[0] != '\0')
            {
                UDP_send_message(packet_str);
            }
        }
        UDP_send_message("\n");
        free(history);
    }
    else if (strcmp(command, "") == 0 && strcmp(prevCommand, "") != 0)
    {
        UDP_process_message(prevCommand);
        return 0;
    }
    else if (strcmp(command, "stop") == 0)
    {
        //UDP_send_message("exit the program gracefully, closing all open sockets threads dynamic memmory, etc");
        return 1;
    }else {
        return 0;
    }
    //printf("prevCommand: %s\n", prevCommand);
    char commandBuffer[MAX_LEN];
    strcpy(commandBuffer, command);
    strcpy(prevCommand, commandBuffer);
    return 0;
}