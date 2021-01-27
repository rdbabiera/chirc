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
    user* u = NULL;
    for ()
}


//
void channel_adduser(channel* channel, user* user)
{
    HASH_ADD_INT(*(channel->user_list), client_socket, user);
    channel->num_users++;
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



