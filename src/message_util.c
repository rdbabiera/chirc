
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
#include "construct_msg.h"


/**************** Functions for Handling Commands ****************/

/* Attach nick name to user struct, send RPL messages if registered */
void nick_fn(char* command_str, user* user, server_ctx* ctx) 
{
    char *nickname;
    struct user* u;
    struct user** user_list;
    char* error;
    
    chilog(DEBUG, "BREAKPOINT 0");

    /* Order: command, nickname */
    char** res = tokenize_message(command_str, " ", 2);
    nickname = res[1];

    chilog(DEBUG, "|%s|", nickname);
    chilog(DEBUG, "BREAKPOINT NICK");


    /* If nickname is null, send out error message */
    if (nickname == NULL)
    {
        if (user->registered == true) 
        {
            error = construct_message(ERR_NONICKNAMEGIVEN, ctx, user, NULL, true);
            send_message(error, user);
            free_tokens(res, 2);
            free(error);
            return;
        } 
        else
        {
            user->nick = "*";
            error = construct_message(ERR_NONICKNAMEGIVEN, ctx, user, NULL, true);
            send_message(error, user);
            free_tokens(res, 2);
            free(error);
            user->nick = NULL;
            return;
        }
    }
    
    /* If nickname is already in user list, send out error message */
    for (u = *(ctx->user_list); u != NULL; u=u->hh.next)
    {
        chilog(DEBUG, "BREAKPOINT ASSIGNMENT");
        if ((u->registered) && (!strncmp(nickname, u->nick, MAX_BUFF_SIZE)))
        {
            if (user->nick == NULL)
            {
                user->nick = "*";
                error = construct_message(ERR_NICKNAMEINUSE, ctx, user, res, true);
                send_message(error, user);
                free_tokens(res, 2);
                free(error);
                user->nick = NULL;
                return;
            }
            else
            {
                error = construct_message(ERR_NICKNAMEINUSE, ctx, user, res, true);
                send_message(error, user);
                free_tokens(res, 2);
                free(error);
                return;
            }
        }
    }

    chilog(DEBUG, "BREAKPOINT 2");

    chilog(INFO, "Nickname: %s registered", res[1]);

    user->nick = strdup(res[1]);
    free_tokens(res, 2);

    /* Handle User Registration */
    char *welcome, *your_host, *created, *my_info, *motd;

    if ((user->username != NULL) && (user->registered == false))
    {

        welcome = construct_message(RPL_WELCOME, ctx, user, NULL, false);
        your_host = construct_message(RPL_YOURHOST, ctx, user, NULL, false);
        created = construct_message(RPL_CREATED, ctx, user, NULL, false);
        my_info = construct_message(RPL_MYINFO, ctx, user, NULL, false);
        motd = construct_message(ERR_NOMOTD, ctx, user, NULL, true);

        send_message(welcome, user);
        send_message(your_host, user);
        send_message(created, user);
        send_message(my_info, user);

        free(welcome);
        free(your_host);
        free(created);
        free(my_info);

        chilog(DEBUG, "Sent welcome messages\n");

        user->registered = true;

        lusers_fn("LUSERS", user, ctx);

        send_message(motd, user);
        free(motd);
        
        return;
    }
    else
    {
        return;
    }

}


/* Attach user name to user struct, send RPL messages if registered */
void user_fn(char* command_str, user* user, server_ctx* ctx)
{   
    char *username, *full_name;
    struct user* u;
    char* error;


    /* Order: command line, full name */
    char** res1 = tokenize_message(command_str, ":", 2);
    /* Order: command, username, ignore1, ignore2 */
    char** res2 = tokenize_message(res1[0], " ", 4);

    username = res2[1];
    full_name = res1[1];

    /* Error if user is already registered or not enough parameters */
    if (user->username != NULL) 
    {
        error = construct_message(ERR_ALREADYREGISTRED, ctx, user, NULL, true);
        send_message(error, user);
        free(error);
        free_tokens(res1, 2);
        free_tokens(res2, 4);
        return;
    }

    if (validate_parameters(command_str, 4, user, ctx) == -1)
    {
        error = construct_message(ERR_NEEDMOREPARAMS, ctx, user, res2, true);
        send_message(error, user);
        free(error);
        free_tokens(res1, 2);
        free_tokens(res2, 4);
        return;
    }


    chilog(INFO, "Username: %s registered.", username);
    chilog(INFO, "Full name: %s registered.", full_name);

    user->username = strdup(res2[1]);
    user->full_name = strdup(res1[1]);
    free_tokens(res1, 2);
    free_tokens(res2, 4);

    /* Handle user registration */
    char *welcome, *your_host, *created, *my_info, *motd;
    if ((user->nick != NULL) && (user->registered == false))
    {
        welcome = construct_message(RPL_WELCOME, ctx, user, NULL, false);
        your_host = construct_message(RPL_YOURHOST, ctx, user, NULL, false);
        created = construct_message(RPL_CREATED, ctx, user, NULL, false);
        my_info = construct_message(RPL_MYINFO, ctx, user, NULL, false);
        motd = construct_message(ERR_NOMOTD, ctx, user, NULL, true);

        send_message(welcome, user);
        send_message(your_host, user);
        send_message(created, user);
        send_message(my_info, user);

        free(welcome);
        free(your_host);
        free(created);
        free(my_info);

        chilog(DEBUG, "Sent welcome messages\n");

        user->registered = true;

        lusers_fn("LUSERS", user, ctx);
        send_message(motd, user);
        free(motd);
        
        return;
    }
    else
    {
        return;
    }
}


/* Quit out of the session */
void quit_fn(char* command_str, user* user, server_ctx* ctx)
{
    int status;
    char *msg_param, *msg;

    /* Order: Quit, parameter */
    char** res = tokenize_message(command_str, ":", 2);

    /* Assign quit parameter if there is one */
    msg_param = res[1];

    if (msg_param == NULL)
    {
        res[1] = "Client Quit";
    }

    /* Send message that user is quitting */
    char* new_msg = construct_message("QUIT", ctx, user, res, false);
    send_message(new_msg, user);
    free(new_msg);

    /* Remove user from server ctx */
    user_delete(ctx->user_list, user);
    chilog(INFO, "USER closed\n");
    pthread_exit(NULL);
}


/* Send a private message */
void privmsg_notice_fn(char* command_str, user* user, server_ctx* ctx)
{
    char* dst_nickname;
    struct user* dst_user;
    char *msg, *command, *error, *complete_msg;

    /* Order: command line, message */
    char** res1 = tokenize_message(command_str, ":", 2);
    /* Order: command, dst user */
    char** res2 = tokenize_message(res1[0], " ", 3);

    dst_nickname = res2[1];
    msg = res1[1];
    command = res2[0];
    
    /* Add in message to the parameter array */
    res2[2] = res1[1];

    /* Find desired user */
    dst_user = user_lookup(ctx->user_list, 0, dst_nickname, 0);

    /* If message is empty, error */
    if (msg == NULL) 
    {
        error = construct_message(ERR_NOTEXTTOSEND, ctx, user, NULL, true);
        send_message(error, user);
        free_tokens(res1, 2);
        free_tokens(res2, 3);
        free(error);
        return;
    }

    /* If no target is provided, error */
    if (dst_nickname == NULL)
    {
        error = construct_message(ERR_NORECIPIENT, ctx, user, res2, true);
        send_message(error, user);
        free_tokens(res1, 2);
        free_tokens(res2, 3);
        free(error);
        return;
    }
    
    /* If no user is found with given nickname, error */
    if (dst_user == NULL)
    {
        error = construct_message(ERR_NOSUCHNICK, ctx, user, res2, true);
        send_message(error, user);
        free_tokens(res1, 2);
        free_tokens(res2, 3);
        free(error);
        return;
    }


    // if (command == "NOTICE") 
    // {
    //     // no automatic reply allowed
    // } 
    // else 
    // {
    complete_msg = construct_message("PRIVMSG", ctx, user, res2, false);
    send_message(msg, dst_user);
    free_tokens(res1, 2);
    free_tokens(res2, 3);
    free(error);
    return;
    //}

}


/* Ping */
void ping_fn(char* command_str, user* user, server_ctx* ctx)
{
    // char* response = construct_message("PONG", NULL, NULL, ctx->server_name, NULL);
    char* response;
    response = construct_message("PONG", ctx, user, NULL, false);
    send_message(response, user);
    free(response);
}


/* Pong */
void pong_fn(char* command_str, user* user, server_ctx* ctx)
{
    return;
}


void lusers_fn(char* command_str, user* user, server_ctx* ctx)
{
    int i;
    int status;
    int user_count, unknown_count, channel_count, op_count;
    int service_count, server_count;
    
    user_count = service_count = op_count = channel_count = unknown_count = 0;
    server_count = 1;

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
        if (u->irc_operator)
        {
            op_count++;
        }
    }

    struct channel** c_list = ctx->channel_list;
    struct channel* c;
    for (c = *c_list; c != NULL; c=c->hh.next)
    {
        channel_count++;
    }

    char *client, *op, *unknown, *channels, *me;

    char** stats = (char**)malloc(sizeof(char*) * 6);
    for (i=0; i<6; i++)
    {
        stats[i] = (char*)malloc(sizeof(char) * 12);
    }

    status = sprintf(stats[0], "%d", user_count);
    status = sprintf(stats[1], "%d", unknown_count);
    status = sprintf(stats[2], "%d", channel_count);
    status = sprintf(stats[3], "%d", op_count);
    status = sprintf(stats[4], "%d", service_count);
    status = sprintf(stats[5], "%d", server_count);

    client = construct_message(RPL_LUSERCLIENT, ctx, user, stats, false);
    op = construct_message(RPL_LUSEROP, ctx, user, stats, false);
    unknown = construct_message(RPL_LUSERUNKNOWN, ctx, user, stats, false);
    channels = construct_message(RPL_LUSERCHANNELS, ctx, user, stats, false);
    me = construct_message(RPL_LUSERME, ctx, user, stats, false);

    send_message(client, user);
    send_message(op, user);
    send_message(unknown, user);
    send_message(channels, user);
    send_message(me, user);

    free_tokens(stats, 6);

    free(client);
    free(op);
    free(unknown);
    free(channels);
    free(me); //please

    return;
    
}

void whois_fn(char* command_str, user* user, server_ctx* ctx)
{
    // REPLIES: RPL_WHOISUSER, RPL_WHOISSERVER, RPL_ENDOFWHOIS
    // LATER: RPL_WHOISOPERATOR, RPL_WHOISCHANNELS, RPL_AWAY
    char* nick;
    char* error;

    if (validate_parameters(command_str, 1, user, ctx) == -1)
    {
        return;
    }

    // Order: Command, Nick Target
    char** res = tokenize_message(command_str, " ", 2);
    struct user* target = user_lookup(ctx->user_list, 0, res[1], 0);
    
    if (target == NULL)
    {
        error = construct_message(ERR_NOSUCHNICK, ctx, user, res, true);
        send_message(error, user);
        free_tokens(res, 2);
        free(error);
        return;
    }
    
    char *whois_user, *server, *end;
    
    whois_user = construct_message(RPL_WHOISUSER, ctx, user, res, false);
    server = construct_message(RPL_WHOISSERVER, ctx, user, res, false);
    end = construct_message(RPL_ENDOFWHOIS, ctx, user, res, false);
    send_message(whois_user, user);
    send_message(server, user);
    send_message(end, user);
    free_tokens(res, 2);
    free(whois_user);
    free(server);
    free(end);
    return;

}

void join_fn(char* command_str, user* user, server_ctx* ctx)
{
    if (validate_parameters(command_str, 1, user, ctx) == -1)
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

    bool part_msg_present = true;
    char* message = strdup(user->nick);

    if (validate_parameters(command_str, 2, user, ctx) == -1)
    {
        part_msg_present = false;
        if (validate_parameters(command_str, 1, user, ctx) == -1)
        {
            // ERR_NEEDMOREPARAMS
        }
    }

    char** res0 = tokenize_message(command_str, ":", 2);
    /* Order: COMMAND, CHANNEL, MESSAGE */
    char** res1 = tokenize_message(command_str, " ", 3);
    char* channel_name = res1[1];
    if (part_msg_present)
    {
        char* temp = message;
        message = res0[1];
        free(temp);
    }
    res1[2] = message;

    channel* c = channel_lookup(channel_name, ctx->channel_list);
    if (c == NULL)
    {
        // ERR_NOSUCHCHANNEL
    }
    if (channel_verifyuser(c, user) == -1)
    {
        // ERR_NOTONCHANNEL
    }
    channel_deluser(c, user);
    if (c->num_users == 0)
    {
        channel_delchannel(c, ctx->channel_list);      
    }
    else
    {
        // Message: "<prefix for leaving user> PART <channel> :msg"
        // can be declared outside
        // message is res1[1]
        char* parting_message; //= construct_message(parameters);

        for (c=*(ctx->channel_list); c != NULL; c=c->hh.next)
        {   
            // send_message(parting_message, c); <- wrong        
            // send each user an f in chat for their lad
        }
        free(parting_message);
    }
    free_tokens(res0, 2);
    free_tokens(res1, 3);

}

void list_fn(char* command_str, user* user, server_ctx* ctx)
{
    bool all_channels = true;
    channel* c;

    if (validate_parameters(command_str, 1, user, ctx))
    {
        all_channels = false;
    }
    /* Order: COMMAND, CHANNEL*/
    char** res = tokenize_message(command_str, " ", 2);

    if (all_channels)
    {
        for (c=*ctx->channel_list; c != NULL; c=c->hh.next)
        {
            // RPL_LIST
            //msg format: :<servername> 322 <nick> <channel> <channel->num_users> :<topic>
            // we dont support topic, so leave it blank.
        }
    }
    else
    {
        c = channel_lookup(res[1], ctx->channel_list);
        if (c == NULL)
        {
            // We're not supposed to support this error
        }
        // RPL_LIST
        // :<servername> 322 <nick> <channel> <channel->num_users> :<topic>
    }
    // RPL_LISTEND
    // :<servername> 323 <nick> :End of LIST

}

void mode_fn(char* command_str, user* user, server_ctx* ctx)
{

    bool ischannelop = false;
    int mode = 0;
    struct user* temp;

    // ERRORS: ERR_NOSUCHCHANNEL, ERR_CHANOPRIVSNEEDED,
    // ERR_UNKNOWNMODE, ERR_USERNOTINCHANNEL
    if (validate_parameters(command_str, 3, user, ctx) == -1)
    {
        //ERR_NOSUCHCHANNEL   
        return;  
    }
    /* Order: COMMAND, CHANNEL, MODE, NICK */
    char** res1 = tokenize_message(command_str, " ", 4);
    channel* c = channel_lookup(res1[1], ctx->channel_list);
    if (c == NULL)
    {
        // ERR_NOSUCHCHANNEL
        free_tokens(res1, 4);
        return;
    }
    struct user* u = user_lookup(ctx->user_list, 0, res1[3], 0);
    if ((u == NULL) || (channel_verifyuser(c, u) == -1))
    {
        // ERR_USERNOTINCHANNEL
        free_tokens(res1, 4);
        return;
    }
    if (channel_verifyoperator(c, user) == 1)
    {
        ischannelop = true;
    }

    if (!strcmp(res1[2], "+o"))
    {
        mode = 1;
    }
    else if (!strcmp(res1[2], "-o"))
    {
        mode = 2;
    }
    else
    {
        // ERR_UNKNOWNMODE
        free_tokens(res1, 4);
        return;
    }
    
    if (user->irc_operator || (ischannelop == true))
    {
        if (mode == 1)
        {
            channel_addoperator(c, u);
        }
        else if (mode == 2)
        {
            channel_deop(c, u);
        }
        // send to all the channel homies
        char* message = construct_message("MODE", ctx, user, res1, false);
        // :<user-nick> MODE <channel> <target-nick> :<mode>
        // user->nick, res1[1], res1[3], res1[2]
        // send_message
        for(temp = *c->user_list; temp != NULL; temp=temp->hh.next)
        {
            // send_message
        }
        /// free message
        free(message);
        free_tokens(res1, 4);
    }
    else
    {
        // ERR_CHANOPRIVSNEEDED 
        free_tokens(res1, 4);
        return;
    }
    free_tokens(res1, 4);

}

void oper_fn(char* command_str, user* user, server_ctx* ctx)
{
    
    if (validate_parameters(command_str, 2, user, ctx) == -1)
    {
        // ERR_NEEDMOREPARAMS              
    }

    /* Order: COMMAND, USER, PASSWORD */
    char** res1 = tokenize_message(command_str, " ",3);
    char* password = res1[2];
    bool correct_pass = false;
    if (!strcmp(password, ctx->operator_password))
    {
        correct_pass = true;
    }

    if (correct_pass)
    {
        user->irc_operator = true;
        
        // :<nick> MODE <nick> :+o

        // RPL_YOUREOPER
        // ":<servername> 381 <nick> :You are now an IRC operator"

    }
    else
    {
        // ERR_PASSWDMISMATCH
        // ":<servername> 464 <nick> :Password incorrect"
    }


}


