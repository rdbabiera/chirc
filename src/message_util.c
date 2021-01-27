#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <pthread.h>

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
    struct user* u;
    struct user** user_list;
    char* error;

    user_list = ctx->user_list;
    
    // Order: command, nickname
    char** res = tokenize_message(command_str, " ", 2);

    chilog(DEBUG, "BREAKPOINT 1");

    // If nickname is already in user list, send out error message
    for (u = *user_list; u != NULL; u=u->hh.next)
    {
        if (!strcmp(nickname, u->nick))
        {
            if (user->nick == NULL)
            {
                user->nick = "*";
                error = construct_message(ERR_NICKNAMEINUSE, ctx, user, res, false);
                send_message(error, user);
                free_tokens(res, 2);
                free(error);
                user->nick = NULL;
                return;
            }
            else
            {
                error = construct_message(ERR_NICKNAMEINUSE, ctx, user, res, false);
                send_message(error, user);
                free_tokens(res, 2);
                free(error);
                return;
            }
        }
    }

    chilog(DEBUG, "BREAKPOINT 2");

    //If nickname is null, send out error message
    if (nickname == NULL)
    {
        error = construct_message(ERR_NONICKNAMEGIVEN, ctx, user, NULL, false);
        send_message(error, user);
        free_tokens(res, 2);
        free(error);
        return;
    }

    chilog(INFO, "Nickname: %s registered", res[1]);

    user->nick = strdup(res[1]);
    free_tokens(res, 2);

    // Handle User Registration
    char* welcome, your_host, created, my_info;

    if ((user->username != NULL) && (user->registered == false))
    {

        welcome = construct_message(RPL_WELCOME, ctx, user, NULL, true);
        your_host = construct_message(RPL_YOURHOST, ctx, user, NULL, true);
        created = construct_message(RPL_CREATED, ctx, user, NULL, true);
        my_info = construct_message(RPL_MYINFO, ctx, user, NULL, true);

        send_message(welcome, user);
        send_message(your_host, user);
        send_message(created, user);
        send_message(my_info, user);

        free(welcome);
        free(your_host);
        free(created);
        free(my_info);

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
    int status;

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
        char* welcome = (char*)malloc(sizeof(char)*512);
        status = sprintf(welcome, 
                ":%s %s %s :Welcome to the Internet Relay Network %s!%s@%s\r\n", 
                ctx->server_name, RPL_WELCOME, user->nick, user->nick, 
                user->username, user->client_host);
        
        char* your_host = (char*)malloc(sizeof(char)*512);
        status = sprintf(your_host, 
                ":%s %s %s :Your host is %s, running version 420.69\r\n", 
                ctx->server_name, RPL_YOURHOST, user->nick, ctx->server_name);

        char* created = (char*)malloc(sizeof(char)*512);
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        status = sprintf(created, 
                ":%s %s %s :This server was created %d-%02d-%02d at %02d:%02d:%02d\r\n",
                ctx->server_name, RPL_CREATED, user->nick, tm.tm_year + 1900, 
                tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        
        char* my_info = (char*)malloc(sizeof(char)*512);
        status = sprintf(my_info, 
                ":%s %s %s :%s 420.60 ao mtov\r\n", 
                ctx->server_name, RPL_MYINFO, user->nick, ctx->server_name);

        send_message(welcome, user);
        send_message(your_host, user);
        send_message(created, user);
        send_message(my_info, user);

        free(welcome);
        free(your_host);
        free(created);
        free(my_info);
        user->registered = true;
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
    int status;
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
    char* new_msg = (char*)malloc(sizeof(char)*512);
    status = sprintf(new_msg, 
            ":%s %s %s ERROR :Closing Link: %s %s\r\n", 
            ctx->server_name, "QUIT", user->nick, 
            user->client_host, msg);

    send_message(new_msg, user);
    free(new_msg);

    // Ri, pls remove user from user_list and server_ctx
    user_delete(*(ctx->user_list), user);
    chilog(INFO, "USER closed\n");
    pthread_exit(NULL);
    // it is done
}


// Send a private message
void privmsg_notice_fn(char* command_str, user* user, server_ctx* ctx)
{
    char* dst_nickname;
    struct user* dst_user;

    char* msg;
    char* command;

    // Order: command line, message
    char** res1 = tokenize_message(command_str, ":", 2);
    // Order: command, dst user
    char** res2 = tokenize_message(res1[0], " ", 2);

    dst_nickname = res2[1];
    msg = res1[1];
    command = res2[0];

    // find desired user
    dst_user = user_lookup(ctx->user_list, 0, dst_nickname, 0);

    // If message is empty, error
    if (msg == NULL) 
    {
        //error handle ERR_NOTEXTTOSEND
    }

    // If no target is provided, error
    if (dst_nickname == NULL)
    {
        //error handle ERR_NORECIPIENT
        return;
    }
    
    // If no user is found with given nickname, error
    if (dst_user == NULL)
    {
        //error handle ERR_NOSUCHNICK
        return;
    }
    send_message(msg, dst_user);

    if (command == "NOTICE") 
    {
        // no automatic reply allowed
    }
    

}


// ping
void ping_fn(char* command_str, user* user, server_ctx* ctx)
{
    // char* response = construct_message("PONG", NULL, NULL, ctx->server_name, NULL);
    int status;
    char* response = (char*)malloc(sizeof(char)*512);
    status = sprintf(response, ":%s PONG %s :%s\r\n", 
            ctx->server_name, user->nick, ctx->server_name);

    send_message(response, user);
    free(response);
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
    int status;
    int user_count, service_count, server_count, op_count;
    int unknown_count, channel_count;
    user_count = service_count = op_count = 0;
    unknown_count = channel_count = 0;
    server_count = 1;
    // ERRORS: RPL_LUSERCLIENT, RPL_LUSEROP, RPL_LUSERUNKNOWN,
    // RPL_LUSERCHANNELS, RPL_LUSERME
    struct user** user_list = ctx->user_list;
    struct user* u;
    for (u = *user_list; u != NULL; u=u->hh.next)
    {
        user_count++;
        if ((u->nick == NULL) && (u->username == NULL))
        {
            unknown_count++;
            user_count--;
        }
        if (u->operator)
        {
            op_count++;
        }
    }

    // This isnt done ^

    // RPL_USERCLIENT
    // ":%s 251 %s :There are x users and x services on 1 servers"

    // RPL_LUSEROP
    // ":%s 252 %s x :operator(s) online\r\n"

    // RPL_LUSERUNKNOWN
    // ":%s 253 %s x :unknown connection(s)\r\n"

    // RPL_LUSERCHANNELS
    // ":%s 254 %s x :channels formed\r\n"

    // RPL_LUSERME
    // ":%s 255 %s :I have x clients and x servers"
}

void whois_fn(char* command_str, user* user, server_ctx* ctx)
{
    // REPLIES: RPL_WHOISUSER, RPL_WHOISSERVER, RPL_ENDOFWHOIS
    // LATER: RPL_WHOISOPERATOR, RPL_WHOISCHANNELS, RPL_AWAY
    char* nick;

    if (validate_parameters(command_str, 1, user) == -1)
    {
        return;
    }

    // Order: Command, Nick Target
    char** res = tokenize_message(command_str, " ", 2);
    struct user* target = user_lookup(ctx->user_list, 0, res[1], 0);
    if (target == NULL)
    {
        // ERR_NOSUCHNICK
        return;
    }
    
    // RPL_WHOISUSER - server_name, user->nick, target->nick
    // target->user, target->fullname
    // ":%s 311 %s %s %s %s * :%s"
    // send message, free it

    // RPL_WHOISSERVER - first two usual, target->nick, servername
    // arbitrary. lets say last one is "big chungus"
    // ":%s 312 %s %s %s :%s"
    // send message, free it

    // RPL_ENDOFWHOIS - usual2 + target->nick
    // ":%s 318 %s %s :End of WHOIS list"
    // send message, free it

}

void join_fn(char* command_str, user* user, server_ctx* ctx)
{
    if (validate_parameters(command_str, 1, user) == -1)
    {
        // ERR_NEEDMOREPARAMS
    }

    // Order: COMMAND, CHANNEL
    char** res1 = tokenize_message(command_str, " ", 2);
    /*
    if (res1[1][0] != '#')
    {
        // ERR_NEEDMOREPARAMS
    }
    char** res2 = tokenize_message(res1[1], "#", 1);
    */
    char* channel_name = res1[1];
    channel* c = channel_lookup(channel_name, ctx->channel_list);
    if (c == NULL)
    {
        ctx->channel_count++;
        c = channel_init(channel_name, ctx->channel_count, ctx->channel_list);
    }
    channel_adduser(c, user);

    // Send confirmation to all other users (@sign before channel operators)


    // Send confirmation to user


    // RPL_NAMREPLY


    // RPL_ENDOFNAMES

}


void part_fn(char* command_str, user* user, server_ctx* ctx)
{
    /* Part Algorithm:
     * Check to see if there is two parameters, and then one. If none, exit
     * Check to see if that channel exists. If not, return error.
     * Check to see if user is in that channel. If not, return error.
     * Remove user from channel + channel list.
     * If channel is empty, destroy it.
     */

    bool part_msg = true;
    int tokens = 3;

    if (validate_parameters(command_str, 2, user) == -1)
    {
        part_msg = false;
        tokens = 2;
        if (validate_parameters(command_str, 1, user) == -1)
        {
            // ERR_NEEDMOREPARAMS
        }
    }

    // Order: COMMAND, CHANNEL, MESSAGE
    char** res1 = tokenize_message(command_str, " ", tokens);
    char* channel_name = res1[1];
    if (part_msg)
    {
        char** res2 = tokenize_message(res1[2], ":", 1);
        char* message = res2[0];
    }

    channel* c = channel_lookup(channel_name, ctx->channel_list);
    if (c == NULL)
    {
        // ERR_NOSUCHCHANNEL
    }
    if (channel_verifyuser(c, user) == -1)
    {

    }

}

void list_fn(char* command_str, user* user, server_ctx* ctx)
{


}

void mode_fn(char* command_str, user* user, server_ctx* ctx)
{


}

void oper_fn(char* command_str, user* user, server_ctx* ctx)
{


}



/**************** Functions for Constructing a Message ****************/

// Construct message to be sent back to client depending on command
char* construct_message(char* msg, server_ctx* ctx, user* user, char** params,
                        bool RPL)
{
    char* res = (char*)malloc(sizeof(char)*512);
    int status;

    // Errors
    if (RPL == false) 
    {

        // Nickname errors
        if (!strcmp(msg, ERR_NICKNAMEINUSE))
        {
            status = sprintf(res, ":%s %s %s: %s :Nickname already in use.\r\n",
                            ctx->server_name, ERR_NICKNAMEINUSE, user->nick, params[1]);
        }
        else if (!strcmp(msg, ERR_NONICKNAMEGIVEN))
        {
            status = sprintf(res, "%s %s %s: No nickname given.\r\n", 
                            ctx->server_name,ERR_NONICKNAMEGIVEN, user->nick);
        }
    }

    // RPL messages
    else
    {
        // Welcome messages
        if (!strcmp(msg, RPL_WELCOME))
        {
            status = sprintf(res, ":%s %s %s :Welcome to the Internet Relay Network \
                            %s!%s@%s\r\n", ctx->server_name, RPL_WELCOME, 
                            user->nick, user->nick, user->username, user->client_host);
        }
        else if (!strcmp(msg, RPL_YOURHOST))
        {
            status = sprintf(res, ":%s %s %s :Your host is %s, running version \
                            420.69\r\n", ctx->server_name, RPL_YOURHOST, user->nick, 
                            ctx->server_name);
        }
        else if (!strcmp(msg, RPL_CREATED))
        {
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            status = sprintf(res, ":%s %s %s :This server was created \
                            %d-%02d-%02d at %02d:%02d:%02d\r\n",
                            ctx->server_name, RPL_CREATED, user->nick, 
                            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, 
                            tm.tm_hour, tm.tm_min, tm.tm_sec);
        }
        else if (!strcmp(msg, RPL_MYINFO))
        {
            status = sprintf(res, ":%s %s %s :%s 420.60 ao mtov\r\n", 
                            ctx->server_name, RPL_MYINFO, user->nick, 
                            ctx->server_name);
        }
    }

    else if (!strcmp(msg, ERR_ALREADYREGISTRED))
    {
        status = sprintf(res, ":%s %s %s :Unauthorized command \
                        (already registered)\r\n", ctx->server_name, 
                        ERR_ALREADYREGISTRED, user->nick);
    }
    // ERR_NEEDMOREPARAMS
    else if (!strcmp(msg, ERR_NEEDMOREPARAMS))
    {
        status = sprintf(res, "%s : Not enough parameters\r\n", extra1);
    }
    // Quit
    else if (!strcmp(msg, "QUIT"))
    {
        status = sprintf(res, "Closing Link: %s %s\r\n", extra1, extra2);
    }
    else if (!strcmp(msg, ERR_NOTREGISTERED))
    {
        status = sprintf(res, "ERR_NOTREGISTERED: You have not registered\r\n");
    }
    else if (!strcmp(msg, ERR_UNKNOWNCOMMAND))
    {
        status = sprintf(res, "ERR_UNKNOWNCOMMAND: %s : Unknown command\r\n", extra1);
    }
    else if (!strcmp(msg, "PONG"))
    {
        status = sprintf(res, "PONG :%s\r\n", extra1);
    }
    else if (!strcmp(msg, RPL_LUSERCLIENT))
    {
        status = sprintf(res, 
            ":There are %s users and %s services on integer servers\r\n", 
            extra1, extra2);
    }
    else if (!strcmp(msg, RPL_LUSEROP))
    {
        status = sprintf(res, "%s :operator(s) online\r\n", extra1);
    }
    else if (!strcmp(msg, RPL_LUSERUNKNOWN))
    {
        status = sprintf(res, "%s :unknown connection(s)\r\n", extra1);
    }
    else if (!strcmp(msg, RPL_LUSERCHANNELS))
    {
        status = sprintf(res, "%s :channels formed\r\n", extra1);
    }
    else if (!strcmp(msg, RPL_LUSERME))
    {
        status = sprintf(res, ":I have %s clients and %s servers\r\n", 
                        extra1, extra2);
    }
    return res;
}
