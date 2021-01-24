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

// Server context struct
typedef struct server_ctx {
    user* user_list;
} server_ctx;


// Server context worker arguments struct
typedef struct worker_args {
    user* curr_user;
    server_ctx* server_ctx;
} worker_args;

/* Functions */
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