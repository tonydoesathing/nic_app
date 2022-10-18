#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "nic_app.h"
#define APP_ID 99

time_t startSecs;
long startNSecs;

char role;
void receiveMessage(uint8_t *message)
{
    if (role == 'r') {//echo back recieved message to who sent it
        send_message(message[1],APP_ID,&message[6],message[0]-5);
    }
    else {
        struct timespec time;
        clock_gettime(CLOCK_BOOTTIME, &time);
        int elapsedMs = (time.tv_sec-startSecs)*1000 + (time.tv_nsec-startNSecs)/1000000;
        printf("Hops: %d RTT: %dms\n",message[2],elapsedMs);
    }
}
/**
 * If the passed role is s for sender
 *  Send a timestamp and see how long it takes to return
 * If the passed role is r for reciever -
 *  Echo sent messages, sending back to where they came from
*/
int main(int argc, char **argv) {
    if (argc != 3)
    {
        printf("Usage: ./RoundTripTime <dest ID if applicable, 0 if not> <role (s/r)>\n");
        return (1);
    }

    int destID = atoi(argv[1]);

    role = argv[2][0];
    // init nic_app
    nic_app_init(APP_ID, receiveMessage);

    while(1) {
        if(role == 's') {
            struct timespec time;
            clock_gettime(CLOCK_BOOTTIME, &time);
            startSecs = time.tv_sec;
            startNSecs = time.tv_nsec;
            uint8_t msg[1] = {'\0'};
            send_message(destID,APP_ID,msg,1);
            sleep(10);
        }
    }
}