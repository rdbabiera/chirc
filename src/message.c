#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "log.h"
#include "message.h"
#include "util.h"
#include "parse_util.h"
#include "reply.h"
#include "users.h"
#include "channels.h"
#include "construct_msg.h"



/* Match message to a viable command */
void match(char* command_str, user* user, server_ctx* ctx)
{
    char* saveptr1;
    char* temp_str = strdup(command_str);
    bool matched;

    matched == false;

    if(command_str == NULL)
    {
        return;
    }

    /* Filling out command array */
    cmd command_arr[14] = {
                {"NICK", nick_fn}, 
                {"USER", user_fn},
                {"QUIT", quit_fn},
                {"PRIVMSG", privmsg_notice_fn},
                {"NOTICE", privmsg_notice_fn},
                {"PING", ping_fn},
                {"PONG", pong_fn},
                {"LUSERS", lusers_fn},
                {"WHOIS", whois_fn},
                {"JOIN", join_fn},
                {"PART", part_fn},
                {"LIST", list_fn},
                {"MODE", mode_fn},
                {"OPER", oper_fn},
                };

    /* Getting command from command_str */
    char* command = strtok_r(temp_str, " ", &saveptr1);
    

    for(int i = 0; i < 14; i++)
    {
        if (!strcmp(command_arr[i].cmd_name, command)) 
        {   
            /* Not registered errors */
            if ((user->registered == false) && (i > 1)) 
            {   
                if (user->nick == NULL) 
                {
                    user->nick = "*";
                    char* error = construct_message(ERR_NOTREGISTERED, ctx, user, NULL, true);
                    send_message(error,user);
                    free(error);
                    user->nick = NULL;
                    return;
                }
                else
                {
                    char* error = construct_message(ERR_NOTREGISTERED, ctx, user, NULL, true);
                    send_message(error,user);
                    free(error);
                    return;
                }
            }
            free(temp_str);
            command_arr[i].execute_cmd(command_str, user, ctx);
            return;
        }
    }

    char** invalid_command = (char**)malloc(sizeof(char*));
    invalid_command[0] = command;
    if (user->registered == true)
    {
        char* error = construct_message(ERR_UNKNOWNCOMMAND, ctx, user, invalid_command, true);
        send_message(error, user);
        free_tokens(invalid_command, 1);
        free(error);
        return;
    } 
    else
    {
        return;
    }
    
}


/* Sends message to a client */
void send_message(char* message, user* user_dest)
{
    pthread_mutex_lock(&(user_dest->socket_mutex));
    if (send(user_dest->client_socket, message, strlen(message), 0) == -1)
    {
        perror("Socket send() failed");
        pthread_mutex_unlock(&(user_dest->socket_mutex));
        return;
    }
    pthread_mutex_unlock(&(user_dest->socket_mutex));
}


/* Send a message to a channel */
void send_message_tochannel(char* msg, channel* channel, user* user)
{
    pthread_mutex_lock(&(channel->channel_mutex));
    struct user* u;
    for (u=*channel->user_list; u != NULL; u=u->hh.next)
    {
        if (u != user)
        {
            send_message(msg, u);
        }
    }
    pthread_mutex_unlock(&(channel->channel_mutex));
}


/* Send a message to all channels a user is in */
void send_message_alluserchannels(char* msg, channel** channel_list, user* user)
{
    struct channel* c;
    for (c=*channel_list; c != NULL; c=c->hh.next)
    {
        if (channel_verifyuser(c, user))
        {
            send_message_tochannel(msg, c, user);
        }
    }
}