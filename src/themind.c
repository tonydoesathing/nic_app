#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nic_app.h"
#include <pthread.h>

char buf[SIZE] = {0};
uint8_t APP_ID=3;
#define MAX_ROUNDS 10
#define MAX_PLAYERS 10

//ID of the game controller; -1 if self
int host = 0;
pthread_mutex_t play_lock;
int hand[MAX_ROUNDS+1];
int handSize;
int myIP = 0;

//controller specific
uint8_t round = 0;
int roundRunning = 0;
uint8_t cardsInPlay[MAX_ROUNDS*MAX_PLAYERS+1]; //all the cards currently in the round
int numPlayers = 0;
uint8_t playerIDs[MAX_PLAYERS];

/**
 * Send a message formatted for The Mind
 * @param destID: the ID of the node to send the message to
 * @param msgtype: the type of message to send
 * @param msg: the contents of the message
 * @param len: the length of msg, above
*/
void sendFormatted(uint8_t destID, uint8_t msgtype, uint8_t* msg, int len) {
    uint8_t message[len + 1];
    message[0] = msgtype;
    for (int i=0; i<len; i++) {
        message[i] = msg[i];
    }
    send_message(destID, APP_ID, message, len);

}

/**
 * Start a new round - should only be called by the host
*/
void newRound() {
    if (host == -1) {
        //generate a series of random hands
        srand(time(NULL));
        for(int i = 0; i<((numPlayers+1) * round); i++) {
            int unique = 0;
            do {
                cardsInPlay[i] == (rand() % 100) + 1;
                //make sure this card is unique
                for (int j = 0; j < i; j++) {
                    if (cardsInPlay[i] == cardsInPlay[j]) {
                        unique = 1;
                        break;
                    }
                }
            } while(!unique);
        }
        //send out the hands
        uint8_t outhand[round+1];
        for (int i = 0; i < numPlayers; i++) {
            outhand[0] = round;
            for (int r = 0; r < round; r++) {
                outhand[r+1] = cardsInPlay[round*i + r];
            }
            sendFormatted(playerIDs[i], 'h', outhand, round+1);
        }
        int i;
        //store our hand
        for (i = 0; i < round; i++) {
            hand[i] = cardsInPlay[(round*numPlayers) + i];
        }
        hand[i] = 0;
        //sort the cards into ascending order for the host to keep track
        //BubbleSort code from https://www.tutorialspoint.com/c-program-to-sort-an-array-in-an-ascending-order
        for (int i = 0; i < numPlayers * round; ++i){
            for (int j = i + 1; j < numPlayers * round; ++j){
                if (cardsInPlay[i] > cardsInPlay[j]){
                    int a = cardsInPlay[i];
                    cardsInPlay[i] = cardsInPlay[j];
                    cardsInPlay[j] = a;
                }
            }
        }
    }
    roundRunning = 1;
}


/**
 * Receive and process messages
 */ 
void receiveMessage(uint8_t* message) {
    if(myIP == 0) {
        myIP = message[3];
    }
    printf("%d: %s \n", message[1], &message[6]);
    uint8_t mtype = message[6];
    switch (mtype) {
        case 'q': //other user quit
            if (host == -1) {
                for (int i = 0; i < numPlayers; i++) {
                    sendFormatted(playerIDs[i], 'q', &message[1], 2);
                }
            }
            printf("User %d has quit the game, exiting...\n",&message[1]);
            exit(0);
            break;
        case 'p': //play card
            if (host == -1) {
                int card = cardsInPlay[0];
                if(message[7] == card) { //if the card played was correct
                    uint8_t card_msg[2];
                    card_msg[0] = message[1];
                    card_msg[1] = card;
                    for (int i = 0; i < numPlayers; i++) {
                        sendFormatted(playerIDs[i], 's', &card_msg, 2);
                    }
                    printf("User %d played %d\n", message[1], message[7]);
                    for(int i = 0; cardsInPlay[i] != 0; i++) {
                        cardsInPlay[i] = cardsInPlay[i+1];
                    }
                    if (cardsInPlay[0] == 0) { //round over
                        printf("All cards have successfully been played! ");
                        if(round == 10) {
                            //if this is the last round, send a different message, exit
                            printf("You have beaten the final round! Ending game.\n");
                            for (int i = 0; i < numPlayers; i++) {
                                sendFormatted(playerIDs[i], 'q', &message[1], 2);
                            }
                        }
                        else {
                            round++;
                            printf("Starting round %d...\n", round+1);
                            newRound();
                        }
                    }
                }
                else if (message[7] == 0) {
                    printf("User %d has an empty hand and must wait for the other players.\n", message[1]);
                }
                else {
                    uint8_t card_msg[3];
                    card_msg[0] = message[1];
                    card_msg[1] = message[7];
                    card_msg[2] = card;
                    for (int i = 0; i < numPlayers; i++) {
                        sendFormatted(playerIDs[i], 'f', &card_msg, 3);
                    }
                    printf("User %d played %d, but the next number should have been %d. Round over.\n", message[1], message[7], card);
                    roundRunning = 0;
                    newRound();
                }
            }
            break;
        case 's': //successful play
            printf("User %d played %d\n", message[7], message[8]);
            break;
        case 'f': //failed play
            printf("User %d played %d, but the next number should have been %d. Round over.\n", message[7], message[8], message[9]);
            roundRunning = 0;
            break;
        case 'j': //join game
            //TODO: no checking for max players or whether game is running
            if (host == -1) {
                playerIDs[numPlayers] = message[1];
                numPlayers++;
            }
            printf("Player %d has joined\n", message[1]);
            break;
        case 'h': //hand for this round - len then cards
            printf("A new round has begun! Round number: %d\n",message[7]);
            handSize = message[7];
            round = message[7];
            //sort hand
            for (int i = 0; i < round; ++i){
                for (int j = i + 1; j < round; ++j){
                    if (message[8+i] > message[8+j]){
                        int a = message[8+i];
                        message[8+i] = message[8+j];
                        message[8+j] = a;
                    }
                }
            }
            printf("Cards: ");
            int i;
            for (i = 0; i < round; i++) {
                printf("%d ", message[8+i]);
                hand[i] = message[8+i];
            }
            hand[i] = 0; //terminator to signal end of hand
            printf("\nType \"p\" and press Enter to play your lowest card\n");
            break;
        default:
            printf("Invalid or corrupt message\n");
    }

}

/**
 * Initialize the router connection and then listen for user input
 */ 
int main(){
    // init nic_app
    nic_app_init(APP_ID, receiveMessage);

    // game setup
    printf("Welcome to The Mind!\n Type h to host game, j to join: ");
        char input[4];
        while (!host) {
            fgets(input,1,stdin);
            // if player wants to host
            if (input[0] == 'h') {
                host = -1;
                printf("Waiting for players...once all have joined, press ENTER to start the game.\n");
                fgets(input, 1, stdin);
                newRound();
            }
            // if player wants to join pending game
            else if(input[0] == 'j') {
                printf("Enter address of host: ");
                fgets(input,3,stdin);
                host = atoi(input);
                uint8_t null = '\0';
                sendFormatted(host, 'j', &null, 1);
            }
            else {
                printf("Invalid input, please try again: ");
            }
        }

    // main game loop
    while (1){
        fgets(input, 1, stdin);
        if (input[0] == 'p') {
            if (host == -1) {
                //create a dummy message to pass to receiveMessage above
                uint8_t msg[8];
                msg[0] = 7;
                msg[1] = myIP;
                msg[2] = 0;
                msg[3] = myIP;
                msg[4] = 'a';
                msg[5] = APP_ID;
                msg[6] = 'p';
                msg[7] = hand[0];
                receiveMessage(&msg[0]);
            }
            else {
                sendFormatted(host, 'p', hand[0], 1);
            }
            for(int i = 0; hand[i] != 0; i++) {
                hand[i] = hand[i+1];
            }
            handSize--;
            printf("Remaining cards: ");
            for (int i = 0; i < handSize; i++) {
                printf("%d ", hand[i]);
            }
            printf("\n");
            printf("Enter \"p\" to play a card, if you have cards left\n");
        }
        
    }
    return 0;
}