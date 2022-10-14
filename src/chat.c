#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nic_app.h"

char buf[SIZE] = {0};
uint8_t APP_ID=2;

/**
 * Print out the message received
 */ 
void receiveMessage(uint8_t* message) {
    printf("%d: %s \n", message[1], &message[6]);
}

/**
 * Initialize the router connection and then listen for user input
 */ 
int main(){
    // init nic_app
    nic_app_init(APP_ID, receiveMessage);

    // get user input
    while (1){
        printf("Destination ID: \n");
        char destString[5];
        fgets(destString,5,stdin);
        uint8_t destID = atoi(destString);
        printf("Message: \n");
        fgets(buf,SIZE-2,stdin); //will this yell at us? lets find out.
        
        send_message(destID,APP_ID,buf,strlen(buf)+1);
    }
    return 0;
}
