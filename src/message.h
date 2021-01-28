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
#include "server_info.h"

#define MAX_BUFF_SIZE 513

/*
 * match - Match current command with a possible command and execute
 * 
 * command_str: command string to be checked and matched
 * 
 * user: user who we are executing functions for
 * 
 * Returns: nothing
 * 
 */
void match(char* command_str, user* user, server_ctx* ctx);


/*
 * send_message - sends a message to a client
 * 
 * message: message to be sent
 * 
 * user_dest: destination user of the message
 * 
 * Returns: nothing
 * 
 */
void send_message(char* message, user* user_dest);


/*
 * send_message - sends a message to a channel's members
 * 
 * message: message to be sent
 * 
 * channel: channel to be sent to
 * 
 * user: user sending the message
 * 
 * Returns: nothing
 * 
 */
void send_message_tochannel(char* msg, channel* channel, user* user);


/*
 * send_message - sends a message to all of a user's channels
 * 
 * message: message to be sent
 * 
 * channel_list: list of channels
 * 
 * user: user sending the message
 * 
 * Returns: nothing
 * 
 */
void send_message_alluserchannels(char* msg, channel** channel_list, user* user);


#endif /* CHIRC_MESSAGE_H_ */