/*
 * 
 * User command functions
 * 
 * This file is to help with managing users connected to the server
 * 
 *  
 */

#ifndef CHIRC_USERS_H_
#define CHIRC_USERS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <pthread.h>

#include "uthash.h"
#include "utlist.h"

/* 
 * User Struct - Keeps track of user information
 *  
 * - nick: User's nickname
 * 
 * - user: User's username
 * 
 * - full_name: User's full name
 * 
 * - client_socket: Socket that the user is attached to
 * 
 * - client_host: Hostname of host that user is attached to
 * 
 * - client_service: Service provider of host that user is attached to
 * 
 * - rpl_welcome: Bool to see if the welcome message has been sent
 * 
 */
typedef struct user {
    char* nick;
    char* username;
    char* full_name;
    int client_socket;
    char client_host[128];
    char client_service[128];

    // Hash needed for server context
    UT_hash_handle hh;

    // Mutex that allows only one command to be sent to the user at a time
    pthread_mutex_t socket_mutex;

    // Statuses
    bool registered;
} user;

/***************** Functions *****************/

/*
 * user_init - Initialize a user
 * 
 * socket: the socket identifier for a client connected to the server
 * 
 * sa: the sockaddr info of the client
 * 
 * salen: the sockaddr len of the sa
 * 
 * Returns: (struct) User
 */
user* user_init(int socket, struct sockaddr* sa, socklen_t salen);


#endif /* CHIRC_USERS_H_ */