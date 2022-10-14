#ifndef NIC_APP
#define NIC_APP

#include <stdint.h>
#define PORT_PATH "/tmp/nic_app"
#define SIZE 123 // buf size of 128--(header size)+

/**
 * message callback function
 * @param uint8_t* the message string
 */ 
typedef void (*call_back) (uint8_t*);

/**
 * initialize the app net library on its own thread (ever-running)
 * @param app_id: the id of the app to receive messages for
 * @param message_received: callback for when a message is received for the app
 */ 
void nic_app_init(int app_id, call_back message_received);

/**
 * @param dest_id: the destination you want to send your message to
 * @param app_id: the destination app_id you want to send your message to
 * @param message: the data you want to send; max size 122
 * @param length: how long the message is
 */ 
void send_message(uint8_t dest_id, uint8_t app_id, uint8_t* message, uint8_t length);

#endif