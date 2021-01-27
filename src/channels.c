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
#include "message_util.h"

#include "uthash.h"
#include "utlist.h"

// 
channel* channel_init(char* channel_name, int cd, channel** channel_list)
{
    channel* new = (channel*)malloc(sizeof(channel));
    new->channel_descriptor = cd;
    new->num_users = 0;
    HASH_ADD_INT(*channel_list, channel_descriptor, new);
    return new;
}



//
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


// 
channel* channel_lookup(char* channel_name, channel** channel_list)
{
    channel* c = NULL;
    int len = strlen(channel_name);
    for (c = *channel_list; c != NULL; c = c->hh.next)
    {
        if (!strncmp(channel_name, c->channel_name, len))
        {
            return c;
        }
    }
}


//
int channel_verifyuser(channel* channel, user* user)
{
    struct user* u = NULL;
    int len;
    for (u = *(channel->user_list); u != NULL; u=u->hh.next)
    {
        len = strnlen(u->nick, MAX_BUFF_SIZE);
        if (!strncmp(user->nick, u->nick, len))
        {
            return 1;
        }
    }
    return -1;
}

//
int channel_verifyoperator(channel* channel, user* user)
{
    struct user* u = NULL;
    for (u = *(channel->operator_list); u != NULL; u=u->hh.next)
    {
        if (u == user)
        {
            return 1;
        }
    }
    return -1;
}


//
void channel_adduser(channel* channel, user* user)
{
    HASH_ADD_INT(*(channel->user_list), client_socket, user);
    channel->num_users++;
}


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


//
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


// 
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