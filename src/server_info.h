/*
 * Server Info
 * 
 * This file will hold all functions and structs that relate to global server
 * context.
 * 
 */

#ifndef CHIRC_SERVER_INFO_H_
#define CHIRC_SERVER_INFO_H_

#include "users.h"
#include "channels.h"

#define MAX_BUFF_SIZE 513
#define ERROR_SIZE 3
#define MAX_COMM_SIZE 128
#define MAX_NICK_SIZE 12


/* 
 * server_ctx - Server context struct 
 * 
 * user_list: list of users on the server
 * 
 * channel_lsit: list of channels of the server
 * 
 * channel_count: number of channels on the server
 * 
 * server_name: name of the server
 * 
 * operator_password: password needed for operator
 * 
 */
typedef struct server_ctx {
    user** user_list;
    channel** channel_list;
    int channel_count;
    char* server_name;
    char* operator_password;
} server_ctx;


/* 
 * worker_args - Server context worker arguments struct 
 * 
 * curr_user: current user
 * 
 * server_ctx: server context
 * 
 */
typedef struct worker_args {
    user* curr_user;
    server_ctx* server_ctx;
} worker_args;


/**************** Functions for Tokenizing Messages ****************/

/*
 * service_single_client - Function used by spawned worker thread to manage
 * a single client
 * 
 * *args: consist of a worker_args struct, used by function to import
 * runtime info
 * 
 * Returns: nothing
 */
void *service_single_client(void *args);

#endif /* CHIRC_SERVER_INFO_H_ */