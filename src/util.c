#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "reply.h"
#include "users.h"
#include "server_info.h"
#include "log.h"
#include "util.h"


// Attach nick name to user struct, send RPL messages if registered
void nick_fn(char* command_str, user* user, server_ctx* ctx) 
{
    char *nickname;
    struct user* user_list;
    user_list = ctx->user_list;
    struct user* u;
    char* error;

    // Order: command, nickname
    char** res = tokenize_message(command_str, " ", 2);
    nickname = res[1];
    
    
    // Here: If nickname is already in user list, send out error message
    for (u = user_list; u != NULL; u=u->hh.next)
    {
        if (!strcmp(nickname, u->nick)){
            error = construct_message(ERR_NICKNAMEINUSE, NULL, user, 
                nickname, NULL);
            send_message(error, user);
            free_tokens(res, 2);
            free(error);
            return;
        }
    }

    //Here: If nickname is null, send out error message
    if (nickname == NULL)
    {
        error = construct_message(ERR_NONICKNAMEGIVEN, NULL, user, NULL, NULL);
        send_message(error, user);
        free_tokens(res, 2);
        free(error);
        return;
    }

    chilog(INFO, "Nickname: %s registered", res[1]);

    user->nick = strdup(res[1]);
    free_tokens(res, 2);

    // Handle Registration
    if (user->username != NULL)
    {
        char* welcome = construct_message(RPL_WELCOME, NULL, user, NULL, NULL);
        char* your_host = construct_message(RPL_YOURHOST, NULL, user, NULL, NULL);
        char* created = construct_message(RPL_CREATED, NULL, user, NULL, NULL);
        char* my_info = construct_message(RPL_MYINFO, NULL, user, NULL, NULL);
        send_message(welcome, user);
        send_message(welcome, user);
        send_message(welcome, user);
        send_message(welcome, user);
        
    }

}


// Attach user name to user struct, send RPL messages if registered
void user_fn(char* command_str, user* user, server_ctx* ctx)
{   
    char *username, *full_name;
    struct user* u;

    if (user->username != NULL) //could also check registration flag?
    {
        //ERR_ALREADYREGISTRED
    }

    if (validate_parameters(command, 4) == -1)
    {
        //ERR_NEEDMOREPARAMS
    }

    // Order: command line, full name
    char** res1 = tokenize_message(command_str, ":", 2);
    // Order: command, username, ignore1, ignore2
    char** res2 = tokenize_message(res1[0], " ", 4);

    username = res2[1];
    full_name = res1[1];

    chilog(INFO, "Username: %s registered.", username);
    chilog(INFO, "Full name: %s registered.", full_name);

    user->username = strdup(res2[1]);
    user->full_name = strdup(res1[1]);
    free_tokens(res1, 2);
    free_tokens(res2, 4);
}


// Match message to a viable command
void match(char* command_str, user* user, server_ctx* ctx)
{
    char* saveptr1;
    char* temp_str = strdup(command_str);

    // Filling out command array (this will increased in Assignment 1b)
    cmd command_arr[2] = {
                {"NICK", nick_fn}, 
                {"USER", user_fn}
                };

    // Getting command from command_str
    char* command = strtok_r(temp_str, " ", &saveptr1);

    for(int i = 0; i < 2; i++)
    {
        if (!strcmp(command_arr[i].cmd_name, command)) 
        {
            free(temp_str);
            command_arr[i].execute_cmd(command_str, user, ctx);
        }
    }
}