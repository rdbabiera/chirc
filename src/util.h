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

/**************** Functions for Parsing Commands ****************/

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
 * Returns: nothing (should only change user info)
 */
void user_fn(char* command_str, user* user, server_ctx* ctx);


/*
 * match - Match current command with a possible command and execute
 * 
 * command_str: command string to be checked and matched
 * 
 * user: user who we are executing functions for
 * 
 * Returns: nothing
 */
void match(char* command_str, user* user, server_ctx* ctx);


#endif /* CHIRC_UTIL_H_ */