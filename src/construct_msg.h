/*
 * This file is to solely hold construct_message() to prevent clutter
 */

#ifndef CHIRC_CONSTRUCT_MSG_H_
#define CHIRC_CONSTRUCT_MSG_H_

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
 * error: A boolean that tells whether the message is an error or message
 *          (true = error, false = RPL message)
 * 
 * Returns: char*
 * 
 */
char* construct_message(char* msg, server_ctx* ctx, user* user, char** params,
                        bool error);

#endif /* CHIRC_CONSTRUCT_MSG_H_ */