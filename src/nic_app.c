#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/poll.h>
#include "nic_app.h"

// socket to communicate with router
int client_socket;

// the callback for a received message
call_back message_received;

/**
 * Poll socket for messages
*/
void* pollSocket() {
	// read
    struct pollfd pollfds[1];
    //listen for regular and high priority data
    pollfds[0].fd = client_socket;
    pollfds[0].events = POLLIN | POLLPRI;
    while(1){
        if (poll(pollfds, 1, -1)<0) {
            // couldn't poll
            perror("Error on poll\n");
            exit(1);
        }else{
            // check for data to read
            if (pollfds[0].revents & POLLIN) {
                uint8_t buf[SIZE+5]; //enough for app id + max msg
                int bufSize = read(pollfds[0].fd, buf, SIZE + 5);
                if ((bufSize == -1) | (bufSize ==0)) {
                    perror("Connection to net router lost. Exiting.\n");
                    exit(1);
                }
                message_received(buf);
            }
        }
    }
}


/**
 * initialize the app net library on its own thread (ever-running)
 * @param app_id: the id of the app to receive messages for
 * @param callback: callback for when a message is received for the app
 */ 
void nic_app_init(int app_id, call_back callback){
    message_received = callback;

    // initialize the socket
    client_socket = socket(AF_LOCAL, SOCK_STREAM, 0);

    // set up the address
    struct sockaddr_un addr;
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, PORT_PATH);

    // connect to the router
    int server_socket = connect(client_socket, (struct sockaddr *)&addr, sizeof(addr));
    // check for error
    if (server_socket < 0) {
        perror("Connect result");
        exit(1);
    }

    // start reading thread
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, pollSocket, NULL);

    // tell router we're listening and who we are
    uint8_t msg[1] = {app_id};
    write(client_socket, msg, 1);
    
}

/**
 * @param dest_id: the destination you want to send your message to
 * @param app_id: the destination app_id you want to send your message to
 * @param message: the data you want to send; max size 122
 * @param length: how long the message is
 */ 
void send_message(uint8_t dest_id, uint8_t app_id, uint8_t* message, uint8_t length){
    // make a buffer with size of message size + dest_id + app_id
    char buf[SIZE+1] = {0};
    buf[0] = dest_id;
    buf[1] = app_id;
    // copy over the message
    for(int i=0;i<length;i++){
        buf[i+2]=message[i];
    }
    // send to router
    write(client_socket, buf, length+2);
}