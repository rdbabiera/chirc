#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "message.h"
#include "message_util.h"
#include "reply.h"
#include "users.h"



// Match message to a viable command
void match(char* command_str, user* user, server_ctx* ctx)
{
    char* saveptr1;
    char* temp_str = strdup(command_str);
    bool matched;

    matched == false;

    // Filling out command array 
    cmd command_arr[5] = {
                {"NICK", nick_fn}, 
                {"USER", user_fn},
                {"QUIT", quit_fn},
                {"PRIVMSG", privmsg_notice_fn},
                {"NOTICE", privmsg_notice_fn},
                {"PING", ping_fn},
                {"PONG", pong_fn},
                {"LUSERS", lusers_fn},
                {"WHOIS", whois_fn}
                };

    // Getting command from command_str
    char* command = strtok_r(temp_str, " ", &saveptr1);


    for(int i = 0; i < 5; i++)
    {
        if ((user->registered == false) && (i > 1)) 
        {
            char* error = construct_message(ERR_NOTREGISTERED, NULL, user, NULL, NULL);
            send_message(error,user);
            return;
        }
        if (!strcmp(command_arr[i].cmd_name, command)) 
        {
            free(temp_str);
            command_arr[i].execute_cmd(command_str, user, ctx);
            return;
        }
    }

    char* error = construct_message(ERR_UNKNOWNCOMMAND, NULL, user, command, NULL);
    send_message(error, user);
    return;
}


// Sends message to a client
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
