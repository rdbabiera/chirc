/*
 * Client message manager
 * 
 * This file will hold all functions and structs that relate to constructing messages
 * that send information back to the client or between clients.
 * 
 */

#ifndef CHIRC_MESSAGE_H_
#define CHIRC_MESSAGE_H_

#include "users.h"

/*
 * construct_message - constructs messages to be sent back to clients
 * 
 * command: the socket identifier for a client connected to the server
 * 
 * user_src: info for user sending messages
 * 
 * user_dest: info for user receiving messages
 * 
 * extra1 & extra2: extra input
 * 
 * Returns: Char*
 * 
 */
char* construct_message(char* command, user* user_src, user* user_dest,
                        char* extra1, char* extra2);

#endif /* CHIRC_MESSAGE_H_ */