#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "nic_app.h"
#define APP_ID 4

uint8_t testMessage[13] = "Test Message";

/**
 * Recieve messages from another instance of this app and tally
 * whether they arrived correctly or not.
 */
int GoodMsgRcvd = 0;
int BadMsgRcvd = 0;
void receiveMessage(uint8_t *message)
{
    // recheck the 6...
    if (!strcmp(&message[6], testMessage))
    { // recieved is the same as sent
        GoodMsgRcvd++;
    }
    else
    {
        BadMsgRcvd++;
    }
}

/**
 * Send 50 messages to the specified ID, then wait and print how many your recieved.
 */
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: ./lotsOMessages <Destiniation ID> <role (s/r)>\n");
        return (1);
    }
    int destID = atoi(argv[1]);

    char role = argv[2][0];

    // init nic_app
    nic_app_init(APP_ID, receiveMessage);
    // get user input
    int i = 0;
    if (role == 's')
    {
        while (1)
        {
            if (i % 10 == 0)
            {
                printf("Sending message %d\n", i);
            }
            send_message(destID, APP_ID, testMessage, strlen(testMessage) + 1);
            usleep(500000);
            //sleep(1);
            i++;
        }
    }
    while (1)
    {
        sleep(10);
        printf("Recieved %d good and %d bad\n", GoodMsgRcvd, BadMsgRcvd);
    }
    return 0;
}