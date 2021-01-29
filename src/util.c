
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
#include "util.h"
#include "parse_util.h"
#include "message.h"
#include "construct_msg.h"
#include "channels.h"


/**************** Functions for Handling Commands ****************/

/* Attach nick name to user struct, send RPL messages if registered */
void nick_fn(char* command_str, user* user, server_ctx* ctx) 
{
    char *nickname;
    struct user* u;
    struct user** user_list;
    char* error;
    char* temp;

    /* Order: command, nickname */
    char** res = tokenize_message(command_str, " ", 2);
    nickname = res[1];
    if (user->nick != NULL)
    {
        temp = res[0];
        res[0] = strdup(user->nick);
        free(temp);
    }

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

    user->nick = strdup(res[1]);

    /* Handle User Registration */
    char *welcome, *your_host, *created, *my_info, *motd;

    if ((user->username != NULL) && (user->registered == false))
    {

        /* Send welcome message */
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

        free_tokens(res, 2);

        chilog(DEBUG, "Sent welcome messages\n");

        user->registered = true;

        lusers_fn("LUSERS", user, ctx);

        send_message(motd, user);

        free(motd);
        
        return;
    }
    else if (user->registered == true)
    {
        /* Reassign NICK if one was previously there */
        chilog(DEBUG, "nick to channel");

        char *message = construct_message("NEW_NICK", ctx, user, res, false);

        send_message(message, user);
        send_message_alluserchannels(message, ctx->channel_list, user);
        
        free(message);
        free_tokens(res, 2);

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
    
    /* Error if you need more parameters */
    if (!validate_parameters(command_str, 4))
    {
        error = construct_message(ERR_NEEDMOREPARAMS, ctx, user, res2, true);
        send_message(error, user);
        free(error);
        free_tokens(res1, 2);
        free_tokens(res2, 4);
        return;
    }


    chilog(DEBUG, "Username: %s registered.", username);
    chilog(DEBUG, "Full name: %s registered.", full_name);

    user->username = strdup(res2[1]);
    user->full_name = strdup(res1[1]);
    free_tokens(res1, 2);
    free_tokens(res2, 4);

    /* Handle user registration */
    char *welcome, *your_host, *created, *my_info, *motd;
    if ((user->nick != NULL) && (user->registered == false))
    {
        /* Send welcome message */
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
    char** res1 = (char**)malloc(sizeof(char*)*2);

    /* Check is user added a quit message */
    if(res[1] == NULL)
    {
        res1[0] = strdup("Client Quit");
    }
    else
    {
        res1[0] = strdup(res[1]);
    }
    
    /* Send message that user is quitting to themselves and channels they're in */
    char* new_msg = construct_message("QUIT", ctx, user, res1, false);
    char* ch_msg = construct_message("Q_CHANNEL", ctx, user, res1, false);
    
    send_message(new_msg, user);
    send_message_alluserchannels(ch_msg, ctx->channel_list, user);
    free(new_msg);
    free(ch_msg);
    free_tokens(res, 2);
    free_tokens(res1,1);

    /* Remove user from server ctx */
    user_delete(ctx->user_list, user);
    chilog(INFO, "USER closed\n");
    pthread_exit(NULL);
}


/* Send a private message or a notice */
void privmsg_notice_fn(char* command_str, user* user, server_ctx* ctx)
{
    char* dst_nickname;
    struct user* dst_user;
    char *msg, *command, *error, *complete_msg;
    bool is_notice = false;
    bool on_channel = false;

    /* Order: command line, message */
    char** res1 = tokenize_message(command_str, ":", 2);
    msg = res1[1];

    /* Order: command, dst user */
    char* temp = strdup(res1[0]);
    char** res2 = tokenize_message(temp, " ", 3);
    free(temp);

    dst_nickname = res2[1];
    command = res2[0];

    /*Check if NOTICE or PRIVMSG*/
    chilog(DEBUG, "command: %s, %d\n", command, strlen(command));
    if (!strncmp(command, "NOTICE", 6))
    {
        is_notice = true;
    }
    
    /* If no recipient, error */
    if (res2[1] == NULL)
    {
        if (!is_notice)
        {
            error = construct_message(ERR_NORECIPIENT, ctx, user, res1, true);
            send_message(error, user);
            free_tokens(res1, 2);
            free_tokens(res2, 3);
            free(error);
        }  
        return;
    }

    if (res1[1] == NULL)
    {
        /* If message is empty, error */
        if (!is_notice)
        {
            error = construct_message(ERR_NOTEXTTOSEND, ctx, user, NULL, true);
            send_message(error, user);
            free_tokens(res1, 2);
            free_tokens(res2, 3);
            free(error);
        }
        return;
    }
    

    /* Order: command, dst user, me */
    char** params = (char**)malloc(sizeof(char*) * 3);
    params[0] = strdup(res2[0]);
    params[1] = strdup(res2[1]);
    params[2] = strdup(res1[1]);    

    /* Check if channel or nick */
    if (res2[1][0] == '#')
    {
        /*Channel name not found*/
        channel* c;
        c = channel_lookup(dst_nickname, ctx->channel_list);

        /* If channel not found, error */
        if (c == NULL)
        {
            if (!is_notice)
            {
                error = construct_message(ERR_NOSUCHNICK, ctx, user, params, true);
                send_message(error, user);
                free(error);
            }
            free_tokens(res1, 2);
            free_tokens(res2, 3);
            free_tokens(params, 3);
            return;
        }

        /* Verify the user is on the channel they want to send a message to */
        on_channel = channel_verifyuser(c, user);

        /*If user hasn't joined channel, error */
        if (!on_channel)
        {
            if(!is_notice)
            {
                error = construct_message(ERR_CANNOTSENDTOCHAN, ctx, user, 
                                            params, true);
                send_message(error, user);
                free(error);
            }
            free_tokens(res1, 2);
            free_tokens(res2, 3);
            free_tokens(params, 3);
            return;
        }

        /* Send the message */
        complete_msg = construct_message(command, ctx, user, params, false);
        send_message_tochannel(complete_msg, c, user);
        free(complete_msg);
        free_tokens(res1, 2);
        free_tokens(res2, 3);
        free_tokens(params, 3);
        return;
    }
    else
    {  

        /* If no target is provided, error */
        if (dst_nickname == NULL)
        {
            if (!is_notice)
            {
                error = construct_message(ERR_NORECIPIENT, ctx, user, params, 
                                            true);
                send_message(error, user);
                free_tokens(res1, 2);
                free_tokens(res2, 3);
                free_tokens(params, 3);
                free(error);
            }
            return;
        }

        /* Find desired user */
        dst_user = user_lookup(ctx->user_list, 0, dst_nickname, 0);
        
        /* If no user is found with given nickname, error */
        if (dst_user == NULL)
        {
            if (!is_notice)
            {
                error = construct_message(ERR_NOSUCHNICK, ctx, user, params, 
                                            true);
                send_message(error, user);
                free_tokens(res1, 2);
                free_tokens(res2, 3);
                free_tokens(params, 3);
                free(error);
            }
            return;
        }

        /* Send message */
        complete_msg = construct_message(command, ctx, user, params, false);
        send_message(complete_msg, dst_user);

        chilog(DEBUG, "%s messaged %s : %s\n", user->nick, dst_user->nick, msg);

        free_tokens(res1, 2);
        free_tokens(res2, 3);
        free_tokens(params, 3);
        free(complete_msg);
        return;
    }

}


/* Ping */
void ping_fn(char* command_str, user* user, server_ctx* ctx)
{
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


/* List users function */
void lusers_fn(char* command_str, user* user, server_ctx* ctx)
{
    int i;
    int status;
    int user_count, unknown_count, channel_count, op_count;
    int service_count, server_count, client_count;
    
    /* Counting things */
    user_count = service_count = op_count = channel_count = unknown_count = 0;
    client_count = 0;
    server_count = 1;

    struct user** user_list = ctx->user_list;
    struct user* u;
    for (u = *user_list; u != NULL; u=u->hh.next)
    {
        client_count++;
        user_count++;
        if ((u->nick == NULL) && (u->username == NULL))
        {
            unknown_count++;
            user_count--;
            client_count--;
        }
        else if (!u->registered)
        {
            user_count--;
        }

        if (u->irc_operator)
        {
            op_count++;
        }
    }

    /* Double check number channels (doing this for thread safety) */
    struct channel** c_list = ctx->channel_list;
    struct channel* c;
    for (c = *c_list; c != NULL; c=c->hh.next)
    {
        channel_count++;
    }

    char *client, *op, *unknown, *channels, *me;

    char** stats = (char**)malloc(sizeof(char*) * 7);
    for (i=0; i<7; i++)
    {
        stats[i] = (char*)malloc(sizeof(char) * 12);
    }

    status = sprintf(stats[0], "%d", user_count);
    status = sprintf(stats[1], "%d", unknown_count);
    status = sprintf(stats[2], "%d", channel_count);
    status = sprintf(stats[3], "%d", op_count);
    status = sprintf(stats[4], "%d", service_count);
    status = sprintf(stats[5], "%d", server_count);
    status = sprintf(stats[6], "%d", client_count);

    /* Send out message */
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

    free_tokens(stats, 7);

    free(client);
    free(op);
    free(unknown);
    free(channels);
    free(me);

    return;
    
}


/* Find out information of user */
void whois_fn(char* command_str, user* user, server_ctx* ctx)
{
    
    char* nick;
    char* error;

    if (!validate_parameters(command_str, 1))
    {
        return;
    }

    /* Order: Command, Nick if target */
    char** res = tokenize_message(command_str, " ", 2);
    struct user* target = user_lookup(ctx->user_list, 0, res[1], 0);
    
    /* If no target user, error */
    if (target == NULL)
    {
        error = construct_message(ERR_NOSUCHNICK, ctx, user, res, true);
        send_message(error, user);
        free_tokens(res, 2);
        free(error);
        return;
    }

    char** params = (char**)malloc(sizeof(char*)*6);
    params[0] = strdup(target->nick);
    params[1] = strdup(target->username);
    params[2] = strdup(target->client_host);
    params[3] = strdup(target->full_name);
    params[4] = ""; /* channel list */
    
    char *whois_user, *server, *end, *whois_op, *whois_ch;
    
    whois_user = construct_message(RPL_WHOISUSER, ctx, user, params, false);
    server = construct_message(RPL_WHOISSERVER, ctx, user, res, false);
    end = construct_message(RPL_ENDOFWHOIS, ctx, user, res, false);
     
    /* If target user is an operator, send extra messages */
    if (target->irc_operator == true) 
    {
        whois_op = construct_message(RPL_WHOISOPERATOR, ctx, user, params, false);
        whois_ch = construct_message(RPL_WHOISCHANNELS, ctx, user, params, false);

        send_message(whois_user, user);
        send_message(whois_ch, user);
        send_message(server, user);
        send_message(whois_op, user);
        send_message(end, user);

        free_tokens(res, 2);
        free_tokens(params, 4);
        free(whois_user);
        free(server);
        free(end);
        free(whois_op);
        free(whois_ch);
        return;
    }
    else
    {
        /* Send messages */
        send_message(whois_user, user);
        send_message(server, user);
        send_message(end, user);
    }
    
    free_tokens(res, 2);
    free_tokens(params, 4);
    
    free(whois_user);
    free(server);
    free(end);
    return;

}


/* Join a channel */
void join_fn(char* command_str, user* user, server_ctx* ctx)
{
    char* error;

    /* Order: Command, channel */
    char** res1 = tokenize_message(command_str, " ", 2);

    if (!validate_parameters(command_str, 1))
    {
        error = construct_message(ERR_NEEDMOREPARAMS, ctx, user, res1, true);
        send_message(error, user);
        free_tokens(res1, 2);
        free(error);
        return;    
    }

    char* channel_name = strdup(res1[1]);
    channel* c = channel_lookup(channel_name, ctx->channel_list);

    /* If channel exists, add user */
    if (c != NULL)
    {
        if (channel_verifyuser(c, user))
        {
            free(channel_name);
            free_tokens(res1, 2);
            return;
        }
        channel_adduser(c, user);
    }
    /* If channel is missing, make it and add user*/
    else if (c == NULL)
    {
        ctx->channel_count++;
        c = channel_init(channel_name, ctx->channel_count, ctx->channel_list);
        channel_adduser(c, user);
        channel_addoperator(c, user);
    }
    
    char* join_msg = construct_message("JOIN", ctx, user, res1, false);
    
    /* Send confirmation to all users */
    char list_names[c->num_users * MAX_NICK_SIZE];
    memset(list_names, 0, (c->num_users * MAX_NICK_SIZE));
    struct channel_user* cu;
    int status;
    
    for (cu=*c->user_list; cu != NULL; cu=cu->hh.next)
    {
        if (channel_verifyoperator(c, cu->user))
        {
            status = sprintf(list_names + strlen(list_names), "@%s", 
                            cu->user->nick);
        }
        else
        {
            status = sprintf(list_names + strlen(list_names), "%s", 
                            cu->user->nick);
        }
        if (cu->user->hh.next != NULL)
        {
            status = sprintf(list_names + strlen(list_names), " ");
        }
        send_message(join_msg, cu->user);
    }

    char** params = (char**)malloc(sizeof(char*) * 2);
    params[0] = strdup(channel_name);
    params[1] = strdup(list_names);

    char* reply = construct_message(RPL_NAMREPLY, ctx, user, params, false);
    char* end = construct_message(RPL_ENDOFNAMES, ctx, user, params, false);


    /* RPL_NAMREPLY (@sign before channel operators) */
    send_message(reply, user);

    /* RPL_ENDOFNAMES */
    send_message(end, user);

    free_tokens(params, 2);
    free_tokens(res1, 2);
    free(channel_name);
    free(join_msg);
    free(reply);
    free(end);
}


/* Leave a channel */
void part_fn(char* command_str, user* user, server_ctx* ctx)
{
    /* Part Algorithm:
     * Check to see if there is two parameters, and then one. If none, exit
     * Check to see if that channel exists. If not, return error.
     * Check to see if user is in that channel. If not, return error.
     * Remove user from channel + channel list.
     * If channel is empty, destroy it.
     */

    char* parting_message;
    int param_tokens = 3;
    bool part_msg_present = true;
    char** res0;
    char** res1;
    
    char* message = strdup(user->nick);
    /* Order: COMMAND, CHANNEL, MESSAGE */
    res0 = tokenize_message(command_str, ":", 2);

    if (!validate_parameters(command_str, 2))
    {
        part_msg_present = false;
        param_tokens = 2;
        res1 = tokenize_message(command_str, " ", param_tokens);

        /* Need more parameter error */
        if (!validate_parameters(command_str, 1))
        {
            char* error = construct_message(ERR_NEEDMOREPARAMS, ctx, user,
                                            res1, true);
            send_message(error, user);

            free(message);
            free_tokens(res0, 2);
            free_tokens(res1, 3);
            free(error);
            return;
        }
    }

    res1 = tokenize_message(command_str, " ", param_tokens);

    char** params = (char**)malloc(sizeof(char*) * 3);
    params[0] = strdup(res1[0]);
    params[1] = strdup(res1[1]);

    if (part_msg_present)
    {
        params[2] = strdup(res0[1]);
    }

    char* channel_name = res1[1];

    /* Channel error checks */
    channel* c = channel_lookup(channel_name, ctx->channel_list);
    if (c == NULL)
    {
        /* If channel doesn't exist, error */
        char* error = construct_message(ERR_NOSUCHCHANNEL, ctx, user, params, 
                                        true);
        send_message(error, user);

        free_tokens(res0, 2);
        free_tokens(res1, 3);
        free(message);
        free(error);
        return;
    }
    if (!channel_verifyuser(c, user))
    {
        /* If user not on channel, error */
        char* error = construct_message(ERR_NOTONCHANNEL, ctx, user, params, 
                                        true);
        send_message(error, user);
        
        free_tokens(res0, 2);
        free_tokens(res1, 3);
        free(message);
        free(error);
        return;
    }

    /* Change message based off the type of part */
    if (c->num_users == 1)
    {
        channel_deluser(c, user);
        channel_delchannel(c, ctx->channel_list);
        if (part_msg_present)
        {
            parting_message = construct_message("PAR_M", ctx, user, params, 
                                                false);
        }
        else
        {
            parting_message = construct_message("PART", ctx, user, params, 
                                                false);
        }      
    }
    else
    {
        /* Message: "<prefix for leaving user> PART <channel> :msg"
         * can be declared outside
         * message is res1[1]
         * need list of channels 
        */
        if (part_msg_present)
        {
            parting_message = construct_message("PAR_M", ctx, user, params, 
                                                false);
        }
        else
        {
            parting_message = construct_message("PART", ctx, user, params, 
                                                false);
        }

        for (c=*(ctx->channel_list); c != NULL; c=c->hh.next)
        {         
            /* send each user an f in chat for their lad leaving */
            send_message_tochannel(parting_message, c, user);
            channel_deluser(c, user);
        }
    }

    send_message(parting_message, user);

    free(parting_message);
    free_tokens(res0, 2);
    free_tokens(res1, 3);
    free_tokens(params, 3);
}


/* List channels and their info */
void list_fn(char* command_str, user* user, server_ctx* ctx)
{
    bool all_channels = true;
    channel* c;
    int status;
    char** channel_params = (char**)malloc(sizeof(char*) * 2);
    channel_params[0] = (char*)malloc(sizeof(char*) * MAX_BUFF_SIZE);
    channel_params[1] = (char*)malloc(sizeof(char*) * MAX_BUFF_SIZE);
    char* message;

    if (validate_parameters(command_str, 1))
    {
        all_channels = false;
    }
    /* Order: COMMAND, CHANNEL*/
    char** res = tokenize_message(command_str, " ", 2);

    /* Send RPL message */
    if (ctx->channel_count == 0)
    {
        char* msg = construct_message(RPL_LISTEND, ctx, user, NULL, false);
        send_message(msg, user);
        free_tokens(channel_params, 2);
        free(msg);
        return;
    }

    if (all_channels)
    {
        for (c=*ctx->channel_list; c != NULL; c=c->hh.next)
        {
            /* RPL_LIST message*/
            status = sprintf(channel_params[0], "%s", c->channel_name);
            status = sprintf(channel_params[1], "%d", c->num_users);

            message = construct_message(RPL_LIST, ctx, user, channel_params,
                            false);

            send_message(message, user);
            free(message);
            memset(channel_params[0], 0, MAX_BUFF_SIZE);
            memset(channel_params[1], 0, MAX_BUFF_SIZE);
        }

        free_tokens(res, 2);
        free_tokens(channel_params, 2);
    }
    else
    {
        c = channel_lookup(res[1], ctx->channel_list);
        if (c == NULL)
        {
            /* We're not supposed to support this error */
        }

        /* RPL_LIST */
        status = sprintf(channel_params[0], "%s", c->channel_name);
        status = sprintf(channel_params[1], "%d", c->num_users);
        message = construct_message(RPL_LIST, ctx, user, channel_params, false);

        send_message(message, user);
        free_tokens(res, 2);
        free(message);
        free_tokens(channel_params, 2);
    }

    /* RPL_LISTEND */
    char* error = construct_message(RPL_LISTEND, ctx, user, NULL, false);
    send_message(error, user);
    free(error);
    return;

}


/* Change modes */
void mode_fn(char* command_str, user* user, server_ctx* ctx)
{

    bool ischannelop = false;
    int mode = 0;
    struct channel_user* temp;

    /* Order: COMMAND, CHANNEL, MODE, NICK */
    char** res1 = tokenize_message(command_str, " ", 4);
    channel* c = channel_lookup(res1[1], ctx->channel_list);


    /* Error checks */
    if (!validate_parameters(command_str, 3))
    {
        /* No channel error */
        char* error = construct_message(ERR_NOSUCHCHANNEL, ctx, user, res1, true);
        send_message(error, user);
        free_tokens(res1, 4);
        free(error);
        return;  
    }
    if (c == NULL) 
    {
        /* No channel error */
        char* error = construct_message(ERR_NOSUCHCHANNEL, ctx, user, res1, true);
        send_message(error, user);
        free_tokens(res1, 4);
        free(error);
        return;
    }

    struct user* u = user_lookup(ctx->user_list, 0, res1[3], 0);
    if ((u == NULL) || (channel_verifyuser(c, u) == -1))
    {
        /* User not in channel */
        char* error = construct_message(ERR_USERNOTINCHANNEL, ctx, user, res1, true);
        send_message(error, user);
        free_tokens(res1, 4);
        free(error);
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
        /* an unknown mode error */
        char* error = construct_message(ERR_UNKNOWNMODE, ctx, user, res1, true);
        send_message(error, user);
        free_tokens(res1, 4);
        free(error);
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
        /* send to all the channel homies */
        char* message = construct_message("MODE", ctx, user, res1, false);
        /* user->nick, res1[1], res1[3], res1[2] */
        for(temp = *c->user_list; temp != NULL; temp=temp->hh.next)
        {
            send_message(message, temp->user);
        }

        free(message);
        free_tokens(res1, 4);
        return;
    }
    else
    {
        /* Need operator permissions error */
        char* error = construct_message(ERR_CHANOPRIVSNEEDED, ctx, user, res1, true);
        send_message(error, user);
        free_tokens(res1, 4);
        free(error);
        return;
    }
    free_tokens(res1, 4);

}


/* Op user function */
void oper_fn(char* command_str, user* user, server_ctx* ctx)
{
    
    /* Order: COMMAND, USER, PASSWORD */
    char** res1 = tokenize_message(command_str, " ",3);
    char* password = res1[2];
    bool correct_pass = false;

    if (!validate_parameters(command_str, 2))
    {
        /* Need more param error */
        char* error = construct_message(ERR_NEEDMOREPARAMS, ctx, user, res1, 
                                        true); 
        send_message(error, user);

        free_tokens(res1, 2);
        free(error);

        return;    
    }

    if (!strcmp(password, ctx->operator_password))
    {
        correct_pass = true;
    }

    if (correct_pass)
    {
        user->irc_operator = true;
        

        /* RPL_YOUREOPER
         * ":<servername> 381 <nick> :You are now an IRC operator"
         */
        char* msg = construct_message(RPL_YOUREOPER, ctx, user, res1, false);
        send_message(msg, user);
        free_tokens(res1, 3);
        free(msg);
        return;
    }
    else
    {
        /* If password doesn't match, error */
        char* error = construct_message(ERR_PASSWDMISMATCH, ctx, user, NULL, true);
        send_message(error, user);
        free_tokens(res1, 3);
        free(error);
        return;
    }


}


