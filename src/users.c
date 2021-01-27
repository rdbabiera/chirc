#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#include "users.h"
#include "log.h"
#include "message.h"



// Initialize a user
user* user_init(int socket, struct sockaddr* sa, socklen_t salen)
{
    int status;
    user* new_user = (user*)malloc(sizeof(user));
    new_user->nick = NULL;
    new_user->username = NULL;
    new_user->client_socket = socket;
    status = getnameinfo(sa, salen, new_user->client_host, 128, 
        new_user->client_service, 128, 0);
    
    // User Statuses + Mutex
    pthread_mutex_init(&new_user->socket_mutex, NULL);
    new_user->registered = false;
    new_user->irc_operator = false;
    return new_user;
}

// Lookup a user
user* user_lookup(user** user_list, int type, char* parameter, int parameter2)
{
    user* res = NULL;
    user* u;

    //Types: 0 = nickname, 1 = username, 2 = socket
    if (type == 0)
    {
        for (u = *user_list; u != NULL; u=u->hh.next)
        {
            if (!strcmp(parameter, u->nick)){
                res = u;
                return res;
            }
        }
    } else if (type == 1)
    {
        for (u = user_list; u != NULL; u=u->hh.next)
        {
            if (!strcmp(parameter, u->username)){
                res = u;
                return res;
            }
        }
    } else if (type == 2)
    {
        for (u = user_list; u != NULL; u=u->hh.next)
        {
            if (parameter2 == u->client_socket)
            {
                res = u;
                return res;
            }
        }
    }

    return res;
}


// Delete a user from a userlist
void user_delete(user** user_list, user* target)
{
    user* temp;
    if (*user_list == target)
    {
        *user_list = target->hh.next;
    } 
    else 
    {
        HASH_DEL(*user_list, target);
    }
    close(target->client_socket);
    pthread_mutex_destroy(&target->socket_mutex);
    free(target->nick);
    free(target->username);
    free(target->full_name);
    free(target);
}