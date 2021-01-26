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

/**************** Functions for Constructing a Message ****************/

/*
 * construct_message - constructs messages to be sent back to clients based on
 *                     command inputted 
 * 
 * error: error to be evaluated
 * 
 * user: info of user that the message is for
 * 
 * ctx: server context
 * 
 * params: list of extra parameters as needed for the error
 * 
 * RPL: A boolean that tells whether the message is an RPL message or an error
 * 
 * Returns: char*
 * 
 * Credit: After speaking with multiple people within Networks about how they were
 * handling errors, we decided it would be best to change our previous 2 extra
 * parameters, to a string array for extra parameters to make things cleaner. 
 * 
 */
char* construct_message(char* msg, server_ctx* ctx, user* user, char** params,
                        bool RPL);



#endif /* CHIRC_UTIL_H_ */