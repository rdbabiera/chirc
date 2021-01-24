#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#include "users.h"
#include "log.h"
#include "util.h"
#include "message.h"


// Initialize a user
user* user_init(int socket, struct sockaddr* sa, socklen_t salen)
{
    int status;
    user* new_user = (user*)malloc(sizeof(user));
    new_user -> nick = NULL;
    new_user -> username = NULL;
    new_user -> client_socket = socket;
    status = getnameinfo(sa, salen, new_user->client_host, 128, 
        new_user->client_service, 128, 0);
    
    // User Statuses + Mutex
    pthread_mutex_init(&new_user->socket_mutex, NULL);
    new_user->registered = false;
    return new_user;
}