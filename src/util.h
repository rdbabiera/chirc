/*
 * 
 * Server command functions
 * 
 * This file is to help with commands that are received by the server
 * 
 * Credit: A friend of Lucy's, Ajay Chopra, who took this class previously, 
 * suggested the command struct when she was discussing about parsing through 
 * commands with him. 
 */

#ifndef CHIRC_UTIL_H_
#define CHIRC_UTIL_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>

#include "users.h"
#include "server_info.h"
#include "message.h"

#include "uthash.h"
#include "utlist.h"

/**************** Structs for Parsing Commands ****************/

/*
 * Command struct - Attaches command function to command name
 * 
 * - cmd_name: Command's name in a string format
 * 
 * - execute_cmd: Command's handler function
 * 
 */
typedef struct command {
    char* cmd_name;
    void (*execute_cmd)(char*, user*, server_ctx*);
} cmd;

/**************** Functions for Handling Commands ****************/

/*
 * nick_fn - Attach NICK to user
 * 
 * command_str: command string to be parsed
 * 
 * user: user who we are giving nickname to
 * 
 * Returns: nothing (should only change user info)
 */
void nick_fn(char* command_str, user* user, server_ctx* ctx);


/*
 * user_fn - Attach USER to user
 * 
 * command_str: command string to be parsed
 * 
 * user: user who we are giving user name to
 *
 * ctx: server context
 * 
 * Returns: nothing (should only change user info)
 */
void user_fn(char* command_str, user* user, server_ctx* ctx);


/*
 * quit_fn - Quit from the session
 *
 * command: command to be parsed
 *
 * user: user who wants to quit
 *
 * ctx: server context
 *
 * Returns: nothing
 */
void quit_fn(char* command_str, user* user, server_ctx* ctx);

/*
 * privmsg_fn - Send a message to designated location
 * 
 * command: command line to be parsed
 * 
 * user: destination user/channel for message
 * 
 * ctx: server ocntext
 * 
 * Returns: nothing
 */
void privmsg_notice_fn(char* command_str, user* user, server_ctx* ctx);


/*
 * ping_fn - Processes ping and sends a pong back to the client
 * 
 * user: destination user/channel for message
 * 
 * ctx: server ocntext
 * 
 * Returns: nothing
 */
void ping_fn(char* command_str, user* user, server_ctx* ctx);


/*
 * pong_fn - Drops pong message from client
 * 
 * user: destination user/channel for message
 * 
 * ctx: server ocntext
 * 
 * Returns: nothing
 */
void pong_fn(char* command_str, user* user, server_ctx* ctx);


/*
 * lusers_fn - Drops pong message from client
 * 
 * user: destination user/channel for message
 * 
 * ctx: server ocntext
 * 
 * Returns: nothing
 */
void lusers_fn(char* command_str, user* user, server_ctx* ctx);


/*
 * whois_fn - Drops pong message from client
 * 
 * user: destination user/channel for message
 * 
 * ctx: server context
 * 
 * Returns: nothing
 */
void whois_fn(char* command_str, user* user, server_ctx* ctx);


/*
 * join_fn - Allows client to join/create a channel
 * 
 * user: destination user/channel for message
 * 
 * ctx: server context
 * 
 * Returns: nothing
 */
void join_fn(char* command_str, user* user, server_ctx* ctx);


/*
 * part_fn - Allows client to leave/delete a channel
 * 
 * user: destination user/channel for message
 * 
 * ctx: server context
 * 
 * Returns: nothing
 */
void part_fn(char* command_str, user* user, server_ctx* ctx);


/*
 * list_fn - Lists channels, amount of users in a channel, and topic
 * 
 * user: destination user/channel for message
 * 
 * ctx: server context
 * 
 * Returns: nothing
 */
void list_fn(char* command_str, user* user, server_ctx* ctx);


/*
 * mode_fn - Allows user to assign channel operator modes to users
 * 
 * user: destination user/channel for message
 * 
 * ctx: server context
 * 
 * Returns: nothing
 */
void mode_fn(char* command_str, user* user, server_ctx* ctx);


/*
 * oper_fn - Allows client to become an irc operator
 * 
 * user: destination user/channel for message
 * 
 * ctx: server context
 * 
 * Returns: nothing
 */
void oper_fn(char* command_str, user* user, server_ctx* ctx);


#endif /* CHIRC_UTIL_H_ */