/*
 * Client command functions
 * 
 * This file will hold all functions and structs that relate to commands that
 * send information back to the client. 
 * 
 */

#ifndef CHIRC_CLIENT_COMMANDs_H_
#define CHIRC_CLIENT_COMMANDS_H_

#include "server_commands.h"

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

#endif //CHIRC_CLIENT_COMMANDS_H_