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

    new->user_list = (channel_user**)malloc(sizeof(channel_user*));
    *new->user_list = NULL;

    pthread_mutex_init(&new->channel_mutex, NULL);

    return new;
}


/* Deletes a channel from the server */
void channel_delchannel(channel* target, channel** channel_list)
{
    if (*channel_list == target)
    {
        *channel_list = target->hh.next;
    } 
    else 
    {
        HASH_DEL(*channel_list, target);
    }

    channel_user* cu;
    channel_user* temp = NULL;
    for (cu = *(target->user_list); cu != NULL; cu=cu->hh.next)
    {
        free(temp);
        temp = cu;
    }
    free(temp);
    free(target->channel_name);
    free(target->user_list);
    free(target);
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
    chilog(INFO, "VERIFYING for %s\n", channel->channel_name);
    struct user* u = NULL;
    channel_user* cu = NULL;
    for (cu = *(channel->user_list); cu != NULL; cu=cu->hh.next)
    {
        u = cu->user;
        if (u == user)
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
    channel_user* cu = NULL;
    for (cu = *(channel->user_list); cu != NULL; cu=cu->hh.next)
    {
        u = cu->user;
        if ((u == user) && cu->is_operator)
        {
            return true;
        }
    }
    return false;
}


/* Adds a User to a Channel */
void channel_adduser(channel* channel, user* user)
{
    channel_user* new_user = (channel_user*)malloc(sizeof(channel_user));
    new_user->user = user;
    new_user->is_operator = false;
    new_user->member_no = channel->num_users;
    HASH_ADD_INT(*(channel->user_list), member_no, new_user);
    channel->num_users++;
}


/* Adds a User to a Channel's Operators List */
void channel_addoperator(channel* channel, user* user)
{
    struct user* u = NULL;
    channel_user* cu = NULL;
    for (cu = *(channel->user_list); cu != NULL; cu=cu->hh.next)
    {
        u = cu->user;
        if (u == user)
        {
            cu->is_operator = true;
        }
    }
}


/* Removes a User from a Channel */
void channel_deluser(channel* channel, user* user)
{
    struct user* u = NULL;
    channel_user* cu = NULL;
    channel_user* target = NULL;

    for (cu = *(channel->user_list); cu != NULL; cu=cu->hh.next)
    {
        u = cu->user;
        if (u == user)
        {
            target = cu;
            break;
        }
    }
    HASH_DEL(*(channel->user_list), target);
    free(target);
}


/* Removes a User from a Channel's Operators List */
void channel_deop(channel* channel, user* user)
{
    struct user* u = NULL;
    channel_user* cu = NULL;

    for (cu = *(channel->user_list); cu != NULL; cu=cu->hh.next)
    {
        u = cu->user;
        if (u == user)
        {
            cu->is_operator = false;
        }
    }

}