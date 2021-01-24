#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "message.h"
#include "reply.h"
#include "users.h"

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
    pthread_mutex_lock(&(user_dest->socket_mutex));
}


// Construct message to be sent back to client depending on command
char* construct_message(char* command, user* user_src, user* user_dest, 
                        char* extra1, char* extra2)
{
    char* res = (char*)malloc(sizeof(char)*512);
    int status;
    if (!strcmp(command, RPL_WELCOME))
    {
        status = sprintf(res, 
            ":%s 001 %s :Welcome to the Internet Relay Network %s!%s@%s\r\n", 
            extra1, user_dest->nick, user_dest->nick, user_dest->username,
            user_dest->client_host);
    }

    else if (!strcmp(command, RPL_YOURHOST))
    {
        status = sprintf(res, "Your host is %s, running version 420.69\n", 
                        user_dest->client_host)) // Change ver later
    }

    else if (!strcmp(command, RPL_CREATED))
    {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t)
        status = sprintf(res, "This server was created %d-%02d-%02d at %02d:%02d:%02d\n",
                        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
                        tm.tm_min, tm.tm_sec);
    }
    
    else if (!strcmp(command, RPL_MYINFO))
    {
        status = sprintf(res, "%s 420.60 <modes> <available channel modes>")

    }

    else if (!strcmp(command, ERR_NICKNAMEINUSE))
    {
        status = sprintf(res, "ERR_NICKNAMEINUSE: The nickname %s is already\
                                in use.\n", extra1);
    }

    else if (!strcmp(command, ERR_NONICKNAMEGIVEN))
    {
        status = sprintf(res, "ERR_NONICKNAMEGIVEN: No nickname was given.\n");
    }

    return res;
}


char** tokenize_message(char* message, char* tokenizer, int num_tokens)
{
    char *saveptr, *curr_token;
    int i;

    char* temp_str = strdup(message);
    char** res = (char**)malloc(sizeof(char*) * num_tokens);

    for (i=0; i<num_tokens; i++)
    {
        if (i == 0)
        {
            curr_token = strtok_r(temp_str, tokenizer, &saveptr);
            res[0] = strdup(curr_token);
        } else
        {
            curr_token = strtok_r(NULL, tokenizer, &saveptr);
            res[i] = strdup(curr_token);
        }
    }
    free(temp_str);
    return res;
}


void free_tokens(char** tokens, int num_tokens)
{
    int i;
    for (i=0; i<num_tokens; i++){
        free(tokens[i]);
    }
    free(tokens);
}


int validate_parameters(char* command, int target_params)
{
    int count = -1;
    char *saveptr, *token;
    char* temp_str = strdup(command);

    token = strtok_r(temp_str, " ", &saveptr);
    while(token != NULL)
    {
        count++;
        token = strtok_r(NULL, " ", &saveptr);
    }
    if (count < target_params)
    {
        return -1;
    }
    return 0;
}