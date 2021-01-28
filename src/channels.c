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
#include "channels.h"
#include "log.h"
#include "message.h"
#include "util.h"

#include "uthash.h"
#include "utlist.h"

/* Initializes a channel */
channel* channel_init(char* channel_name, int cd, channel** channel_list)
{
    channel* new = (channel*)malloc(sizeof(channel));

    new->channel_name = strdup(channel_name);
    new->channel_descriptor = cd;
    new->num_users = 0;

    HASH_ADD_INT(*channel_list, channel_descriptor, new);

    new->user_list = (user**)malloc(sizeof(user*));
    *new->user_list = NULL;

    new->operator_list = (user**)malloc(sizeof(user*));
    *new->operator_list = NULL;

    pthread_mutex_init(&new->channel_mutex, NULL);

    return new;
}


/* Deletes a channel from the server */
void channel_delchannel(channel* target, channel** channel_list)
{
    channel* temp;

    if (*channel_list == target)
    {
        *channel_list = target->hh.next;
    } 
    else 
    {
        HASH_DEL(*channel_list, target);
    }
    free(target->channel_name);
    free(target->user_list);
}


/* Looksup a channel by name */ 
channel* channel_lookup(char* channel_name, channel** channel_list)
{
    channel* c;
    channel* res = NULL;

    for (c = *(channel_list); c != NULL; c = c->hh.next)
    {
        if (!strcmp(channel_name, c->channel_name))
        {
            res = c;
        }
    }
    return res;
}


/* Verifies whether or not a user is in a channel */
bool channel_verifyuser(channel* channel, user* user)
{
    struct user* u = NULL;
    int len;
    for (u = *(channel->user_list); u != NULL; u=u->hh.next)
    {
        len = strnlen(u->nick, MAX_BUFF_SIZE);
        if (!strncmp(user->nick, u->nick, len))
        {
            return true;
        }
    }
    return false;
}


/* Verifies whether or not a user is an operator in a channel */
bool channel_verifyoperator(channel* channel, user* user)
{
    struct user* u = NULL;
    for (u = *(channel->operator_list); u != NULL; u=u->hh.next)
    {
        if (u == user)
        {
            return true;
        }
    }
    return false;
}


/* Adds a User to a Channel */
void channel_adduser(channel* channel, user* user)
{
    HASH_ADD_INT(*(channel->user_list), client_socket, user);
    channel->num_users++;
}

/* Adds a User to a Channel's Operators List */
void channel_addoperator(channel* channel, user* user)
{
    struct user* u = NULL;
    for (u = *(channel->operator_list); u != NULL; u=u->hh.next)
    {
        if (u == user)
        {
            return;
        }
    }
    HASH_ADD_INT(*(channel->operator_list), client_socket, user);
}


/* Removes a User from a Channel */
void channel_deluser(channel* channel, user* user)
{
    if (*(channel->user_list) == user)
    {
        *(channel->user_list) = (*(channel->user_list))->hh.next;
    } 
    else 
    {
        HASH_DEL(*(channel->user_list), user);
    }
    channel->num_users--;
}


/* Removes a User from a Channel's Operators List */
void channel_deop(channel* channel, user* user)
{
    struct user* u = NULL;
    for (u = *(channel->operator_list); u != NULL; u=u->hh.next)
    {
        if (u == user)
        {
            if (*(channel->operator_list) == user)
            {
                *(channel->operator_list) = (*(channel->operator_list))->hh.next;
            } 
            else 
            {
                HASH_DEL(*(channel->operator_list), user);
            }
            return;
        }
    } 
}