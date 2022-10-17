#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nic_app.h"
#define APP_ID 4

uint8_t testMessage[13] = "Test Message";

/**
 * Recieve messages from another instance of this app and tally 
 * whether they arrived correctly or not.
 */ 
int GoodMsgRcvd = 0;
int BadMsgRcvd = 0;
void receiveMessage(uint8_t* message) {
    //recheck the 6...
    if (!strcmp(&message[6],testMessage)) {// recieved is the same as sent
        GoodMsgRcvd++;
    }
    else {BadMsgRcvd++;}
}

/**
 * Send 50 messages to the specified ID, then wait and print how many your recieved.
 */ 
int main(int argc, char**argv){
    if (argc < 2)
    {
        printf("Usage: ./lotsOMessages <Destiniation ID>\n");
        return (1);
    }
    int destID = atoi(argv[1]);

    // init nic_app
    nic_app_init(APP_ID, receiveMessage);
    // get user input
    int howMany = 50;
    for (int i=0; i<howMany; i++) {
        if (i%10 == 0) {
            printf("Sending message %d of %d\n",i,howMany);
        }
        send_message(destID,APP_ID,testMessage,strlen(testMessage)+1);
    }
    while(1) {
        printf("Recieved %d good and %d bad\n",GoodMsgRcvd,BadMsgRcvd);
    }
    return 0;
}