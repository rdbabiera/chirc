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


/* Server context struct */
typedef struct server_ctx {
    user** user_list;
    channel** channel_list;
    int channel_count;
    char* server_name;
    char* operator_password;
} server_ctx;


/* Server context worker arguments struct */
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