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
//static char prevReturnCommand[MAX_LEN];
static void UDP_process_message(char command[])
{
    char returnMessage[MAX_LEN];
    snprintf(returnMessage, MAX_LEN, "Hello%d\n", 42);

    if (strcmp(command, "help") == 0 || strcmp(command, "?") == 0)
    {
        UDP_send_message("return a brief summary of supported commands");
    }
    else if (strcmp(command, "count") == 0)
    {
        UDP_send_message("return the total number of light samples taken so far (big number)");
    }
    else if (strcmp(command, "length") == 0)
    {
        UDP_send_message("return how many samples were taken in the last second");
    }
    else if (strcmp(command, "dips") == 0)
    {
        UDP_send_message("return how many dips were detected in the last second (?)");
    }
    else if (strcmp(command, "history") == 0)
    {
        UDP_send_message("return all data samples from previous second, see pdf for details");
    }
    else if (strcmp(command, "") == 0)
    {
        if (strcmp(command, prevCommand) == 0 && strcmp(command, "") != 0) {
                UDP_send_message("repeat previous command (if not first command!)");
        }
    }
    else if (strcmp(command, "stop") == 0)
    {
        UDP_send_message("exit the program gracefully, closing all open sockets threads dynamic memmory, etc");

    }

    for (size_t i = 0; i < MAX_LEN - 1; i++) {
        if (i < strlen(command) - 1) {
        prevCommand[i] = command[i];
        } else {
            prevCommand[i] = 0;
        }
    }
}