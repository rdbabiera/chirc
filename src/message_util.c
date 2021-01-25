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
#include "message_util.h"
#include "parse_util.h"
#include "message.h"


/**************** Functions for Handling Commands ****************/

// Attach nick name to user struct, send RPL messages if registered
void nick_fn(char* command_str, user* user, server_ctx* ctx) 
{
    char *nickname;
    struct user* user_list;
    user_list = ctx->user_list;
    struct user* u;
    char* error;
    bool previous;

    // Boolean to check if nickname was already present
    previous = false;
    if (user->registered == true)
    {
        previous = true;
    }

    // Order: command, nickname
    char** res = tokenize_message(command_str, " ", 2);
    nickname = res[1];
    
    
    // If nickname is already in user list, send out error message
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

    //If nickname is null, send out error message
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
    if ((user->username != NULL) && (previous == false))
    {
        char* welcome = construct_message(RPL_WELCOME, NULL, user, NULL, NULL);
        char* your_host = construct_message(RPL_YOURHOST, NULL, user, NULL, NULL);
        char* created = construct_message(RPL_CREATED, NULL, user, NULL, NULL);
        char* my_info = construct_message(RPL_MYINFO, NULL, user, NULL, NULL);
        send_message(welcome, user);
        send_message(your_host, user);
        send_message(created, user);
        send_message(my_info, user);
        user->registered = true;
        return;
    }
    else
    {
        return;
    }

}


// Attach user name to user struct, send RPL messages if registered
void user_fn(char* command_str, user* user, server_ctx* ctx)
{   
    char *username, *full_name;
    struct user* u;
    char* error;

    if (user->username != NULL) 
    {
        error = construct_message(ERR_ALREADYREGISTRED, NULL, user, NULL, NULL);
        send_message(error, user);
    }

    if (validate_parameters(command_str, 4, user) == -1)
    {
       return;
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

    // Handle registration
    if ((user->nick != NULL) && (user->registered == false))
    {
        char* welcome = construct_message(RPL_WELCOME, NULL, user, NULL, NULL);
        char* your_host = construct_message(RPL_YOURHOST, NULL, user, NULL, NULL);
        char* created = construct_message(RPL_CREATED, NULL, user, NULL, NULL);
        char* my_info = construct_message(RPL_MYINFO, NULL, user, NULL, NULL);
        send_message(welcome, user);
        send_message(your_host, user);
        send_message(created, user);
        send_message(my_info, user);
        user->registered = true;
        free(welcome);
        free(your_host);
        free(created);
        free(my_info);
        return;
    }
    else
    {
        return;
    }
}


// Quit out of the session
void quit_fn(char* command_str, user* user, server_ctx* ctx)
{
    char *msg_param, *msg;

    // Order: Quit, parameter
    char** res = tokenize_message(command_str, ":", 2);

    // Assign quit parameter if there is one
    msg_param = res[1];

    if (msg_param != NULL)
    {
        msg = msg_param;
    }
    else
    {
        msg = "Client Quit";
    }

    // Send message that user is quitting
    char* new_msg = construct_message("QUIT", NULL, user, user->client_host, msg);
    send_message(new_msg, user);

    // Ri, pls remove user from user_list and server_ctx
    user_delete(ctx->user_list, user);
    // it is done
}


// Send a private message
void privmsg_fn(char* command_str, user* user, server_ctx* ctx)
{
    char* dst_nickname;
    struct user* dst_user;

    char* msg;

    // Order: command line, destination user
    char** res1 = tokenize_message(command_str, ":", 2);
    // Order: 
    char** res2 = tokenize_message(res1[0], " ", 2);

    dst_nickname = res2[1];
    msg = res1[1];
    if (msg == NULL)

    // find desired user
    dst_user = user_lookup(ctx->user_list, 0, dst_nickname, 0);
    if (dst_user == NULL)
    {
        //error handle ERR_NOSUCHNICK
    }
    send_message(msg, dst_user);
    

}


// ping
void ping_fn(char* command_str, user* user, server_ctx* ctx)
{
    char* response = construct_message("PONG", NULL, NULL, ctx->server_name, NULL);
    send_message(response, user);
}


// pong
void pong_fn(char* command_str, user* user, server_ctx* ctx)
{
    /*
    // Dont actually think we do anything but drop the message tbh
    int param_check = validate_parameters(command_str, 2, user);
    if (param_check != -1)
    {
        // Order: command, src, dest
        char** res = tokenize_message(command_str, " ", 3);
        char* src_nickname = res[1];
        char* dst_nickname = res[2];
        char* response = construct_message("PONG2", NULL, NULL, , NULL);
        send_message(response, user);
    }
    */
}


void lusers_fn(char* command_str, user* user, server_ctx* ctx)
{

}

void whois_fn(char* command_str, user* user, server_ctx* ctx)
{

}



/**************** Functions for Constructing a Message ****************/

// Construct message to be sent back to client depending on command
char* construct_message(char* command, user* user_src, user* user_dest, 
                        char* extra1, char* extra2)
{
    char* res = (char*)malloc(sizeof(char)*512);
    int status;
    // RPL_WELCOME
    if (!strcmp(command, RPL_WELCOME))
    {
        status = sprintf(res, 
            ":%s 001 %s :Welcome to the Internet Relay Network %s!%s@%s\r\n", 
            extra1, user_dest->nick, user_dest->nick, user_dest->username,
            user_dest->client_host);
    }
    // RPL_YOURHOST
    else if (!strcmp(command, RPL_YOURHOST))
    {
        status = sprintf(res, "Your host is %s, running version 420.69\r\n", 
                        user_dest->client_host); // Change ver later
    }
    // RPL_CREATED
    else if (!strcmp(command, RPL_CREATED))
    {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        status = sprintf(res, "This server was created %d-%02d-%02d at %02d:%02d:%02d\r\n",
                        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
                        tm.tm_min, tm.tm_sec);
    }
    // RPL_MYINFO
    else if (!strcmp(command, RPL_MYINFO))
    {
        status = sprintf(res, "%s 420.60 ao mtov", extra1);

    }
    // ERR_NICKNAMEINUSE
    else if (!strcmp(command, ERR_NICKNAMEINUSE))
    {
        status = sprintf(res, "ERR_NICKNAMEINUSE: The nickname %s is already \
                                in use\r\n", extra1);
    }
    // ERR_NONICKNAMEGIVEN
    else if (!strcmp(command, ERR_NONICKNAMEGIVEN))
    {
        status = sprintf(res, "ERR_NONICKNAMEGIVEN: No nickname given\r\n");
    }
    // ERR_ALREADYREGISTERED
    else if (!strcmp(command, ERR_ALREADYREGISTRED))
    {
        status = sprintf(res, "ERR_ALREADYREGISTERED: Unauthorized command \
                                (already registered)\r\n");
    }
    // ERR_NEEDMOREPARAMS
    else if (!strcmp(command, ERR_NEEDMOREPARAMS))
    {
        status = sprintf(res, "%s : Not enough parameters\r\n", extra1);
    }
    // Quit
    else if (!strcmp(command, "QUIT"))
    {
        status = sprintf(res, "Closing Link: %s %s\r\n", extra1, extra2);
    }
    else if (!strcmp(command, ERR_NOTREGISTERED))
    {
        status = sprintf(res, "ERR_NOTREGISTERED: You have not registered\r\n");
    }
    else if (!strcmp(command, ERR_UNKNOWNCOMMAND))
    {
        status = sprintf(res, "ERR_UNKNOWNCOMMAND: %s : Unknown command\r\n", extra1);
    }
    else if (!strcmp(command, "PONG"))
    {
        status = sprintf(res, "PONG :%s\r\n", extra1);
    }
    else if (!strcmp(command, RPL_LUSERCLIENT))
    {
        status = sprintf(res, ":There are %s users and %s services on 
                        <integer> servers\r\n", extra1, extra2);
    }
    else if (!strcmp(command, RPL_LUSEROP))
    {
        status = sprintf(res, "%s :operator(s) online\r\n", extra1);
    }
    else if (!strcmp(command, RPL_LUSERUNKNOWN))
    {
        status = sprintf(res, "%s :unknown connection(s)\r\n", extra1);
    }
    else if (!strcmp(command, RPL_LUSERCHANNELS))
    {
        status = sprintf(res, "%s :channels formed\r\n", extra1);
    }
    else if (!strcmp(command, RPL_LUSERME))
    {
        status = sprintf(res, ":I have %s clients and %s servers\r\n", 
                        extra1, extra2);
    }
    return res;
}
