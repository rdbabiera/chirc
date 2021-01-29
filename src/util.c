
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
    
    chilog(DEBUG, "BREAKPOINT 0");

    /* Order: command, nickname */
    char** res = tokenize_message(command_str, " ", 2);
    nickname = res[1];
    if (user->nick != NULL)
    {
        temp = res[0];
        res[0] = strdup(user->nick);
        free(temp);
    }

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

    if (!validate_parameters(command_str, 4))
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
    char** res1 = (char**)malloc(sizeof(char*)*2);

    if(res[1] == NULL)
    {
        res1[0] = strdup("Client Quit");
    }
    else
    {
        res1[0] = strdup(res[1]);
    }
    
    /* Send message that user is quitting */
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


/* Send a private message */
void privmsg_notice_fn(char* command_str, user* user, server_ctx* ctx)
{
    chilog(INFO, "PRIVMSG B1\n");
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
    chilog(INFO, "PRIVMSG B2\n");

    dst_nickname = res2[1];
    command = res2[0];

    /* Order: command, dst user, me */
    char** params = (char**)malloc(sizeof(char*) * 3);
    params[0] = strdup(res2[0]);
    params[1] = strdup(res2[1]);
    params[2] = strdup(res1[1]);

    chilog(INFO, "PRIVMSG B2.2\n");

    /*Check if NOTICE or PRIVMSG*/
    chilog(INFO, "command: %s\n", command);
    if (!strncmp(command, "NOTICE", 6))
    {
        is_notice = true;
    }
        
    
    if (res2[1][0] == '#')
    {
        chilog(INFO, "PRIVMSG B2.3\n");
        /*Channel name not found*/
        channel* c;
        c = channel_lookup(dst_nickname, ctx->channel_list);
        chilog(INFO, "PRIVMSG B3\n");

        if (c == NULL)
        {
            error = construct_message(ERR_NOSUCHNICK, ctx, user, params, true);
            send_message(error, user);
            free_tokens(res1, 2);
            free_tokens(res2, 3);
            free_tokens(params, 3);
            free(error);
            return;
        }

        on_channel = channel_verifyuser(c, user);

        /*Have not joined channel*/
        if (!on_channel)
        {
            if(!is_notice)
            {
                error = construct_message(ERR_CANNOTSENDTOCHAN, ctx, user, params, true);
                send_message(error, user);
                free(error);
            }
            free_tokens(res1, 2);
            free_tokens(res2, 3);
            free_tokens(params, 3);
            return;
        }
        chilog(INFO, "PRIVMSG B4\n");

        complete_msg = construct_message(command, ctx, user, params, false);
        chilog(INFO, "PRIVMSG B5\n");
        send_message_tochannel(complete_msg, c, user);
        chilog(INFO, "PRIVMSG B6\n");
        free(complete_msg);
        free_tokens(res1, 2);
        free_tokens(res2, 3);
        free_tokens(params, 3);
        chilog(INFO, "PRIVMSG B7\n");
        return;
    }
    else
    {
        chilog(INFO, "PRIVMSG F1\n");
        /* IF no recipient, error */
        if (!validate_parameters(command_str, 1))
        {
            if (!is_notice)
            {
                error = construct_message(ERR_NORECIPIENT, ctx, user, params, true);
                send_message(error, user);
                free_tokens(res1, 2);
                free_tokens(res2, 3);
                free_tokens(params, 3);
                free(error);
            }  
            return;
        }
        /* If message is empty, error */
        if (msg == NULL) 
        {
            if (!is_notice)
            {
                error = construct_message(ERR_NOTEXTTOSEND, ctx, user, NULL, true);
                send_message(error, user);
                free_tokens(res1, 2);
                free_tokens(res2, 3);
                free_tokens(params, 3);
                free(error);
            }
            return;
        }

        /* If no target is provided, error */
        if (dst_nickname == NULL)
        {
            if (!is_notice)
            {
                error = construct_message(ERR_NORECIPIENT, ctx, user, params, true);
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
                error = construct_message(ERR_NOSUCHNICK, ctx, user, params, true);
                send_message(error, user);
                free_tokens(res1, 2);
                free_tokens(res2, 3);
                free_tokens(params, 3);
                free(error);
            }
            return;
        }


        complete_msg = construct_message(command, ctx, user, params, false);
        send_message(complete_msg, dst_user);

        chilog(INFO, "%s messaged %s : %s\n", user->nick, dst_user->nick, msg);

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
    free(me); //please

    return;
    
}


void whois_fn(char* command_str, user* user, server_ctx* ctx)
{
    // REPLIES: RPL_WHOISUSER, RPL_WHOISSERVER, RPL_ENDOFWHOIS
    // LATER: RPL_WHOISOPERATOR, RPL_WHOISCHANNELS, RPL_AWAY
    char* nick;
    char* error;

    if (!validate_parameters(command_str, 1))
    {
        return;
    }

    /* Order: Command, Nick if target */
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
    char** params = (char**)malloc(sizeof(char*)*6);
    params[0] = strdup(target->nick);
    params[1] = strdup(target->username);
    params[2] = strdup(target->client_host);
    params[3] = strdup(target->full_name);
    params[4] = "";//channel list
    
    char *whois_user, *server, *end, *whois_op, *whois_ch;
    
    whois_user = construct_message(RPL_WHOISUSER, ctx, user, params, false);
    server = construct_message(RPL_WHOISSERVER, ctx, user, res, false);
    end = construct_message(RPL_ENDOFWHOIS, ctx, user, res, false);
     
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
    if (c != NULL)
    {
        chilog(INFO, "Channel: %s", c->channel_name);
        if (channel_verifyuser(c, user))
        {
            free(channel_name);
            free_tokens(res1, 2);
            return;
        }
    }
    else if (c == NULL)
    {
        ctx->channel_count++;
        c = channel_init(channel_name, ctx->channel_count, ctx->channel_list);
    }
    channel_adduser(c, user);

    
    char* join_msg = construct_message("JOIN", ctx, user, res1, false);
    
    // Send confirmation to all users
    char list_names[c->num_users * MAX_NICK_SIZE];
    memset(list_names, 0, (c->num_users * MAX_NICK_SIZE));
    struct user* u;
    int status;
    for (u=*c->user_list; u != NULL; u=u->hh.next)
    {
        if (channel_verifyoperator(c, u))
        {
            status = sprintf(list_names + strlen(list_names), "@%s", u->nick);
        }
        else
        {
            status = sprintf(list_names + strlen(list_names), "%s", u->nick);
        }
        if (u->hh.next != NULL)
        {
            status = sprintf(list_names + strlen(list_names), " ");
        }
        send_message(join_msg, u);
    }

    char** params = (char**)malloc(sizeof(char*) * 2);
    params[0] = strdup(channel_name);
    params[1] = strdup(list_names);

    char* reply = construct_message(RPL_NAMREPLY, ctx, user, params, false);
    char* end = construct_message(RPL_ENDOFNAMES, ctx, user, params, false);


    // RPL_NAMREPLY (@sign before channel operators)
    send_message(reply, user);

    // RPL_ENDOFNAMES
    send_message(end, user);

    free_tokens(params, 2);
    free_tokens(res1, 2);
    free(channel_name);
    free(join_msg);
    free(reply);
    free(end);
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
    char** res0 = tokenize_message(command_str, ":", 2);
    /* Order: COMMAND, CHANNEL, MESSAGE */
    char** res1 = tokenize_message(command_str, " ", 3);

    if (!validate_parameters(command_str, 2))
    {
        part_msg_present = false;
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

    char* channel_name = res1[1];
    if (part_msg_present)
    {
        char* temp = message;
        message = res0[1];
        free(temp);
    }
    res1[2] = message;

    /* Channel error checks */
    channel* c = channel_lookup(channel_name, ctx->channel_list);
    if (c == NULL)
    {
        char* error = construct_message(ERR_NOSUCHCHANNEL, ctx, user, res1, true);
        send_message(error, user);

        free_tokens(res0, 2);
        free_tokens(res1, 3);
        free(message);
        free(error);
        return;
    }
    if (channel_verifyuser(c, user) == -1)
    {
        char* error = construct_message(ERR_NOTONCHANNEL, ctx, user, res1, true);
        send_message(error, user);
        
        free_tokens(res0, 2);
        free_tokens(res1, 3);
        free(message);
        free(error);
        return;
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
        // need list of channels
        char* parting_message = construct_message("PART", ctx, user, res1, false);

        for (c=*(ctx->channel_list); c != NULL; c=c->hh.next)
        {   
            // send_message(parting_message, c); <- wrong        
            // send each user an f in chat for their lad
            // F - Lucy
            send_message_tochannel(parting_message, c, user);
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

    if (validate_parameters(command_str, 1))
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
            char* message = construct_message(RPL_LIST, ctx, user, res, false);
            free_tokens(res, 2);
            free(message);
            return;
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
        char* message = construct_message(RPL_LIST, ctx, user, res, false);
        free_tokens(res, 2);
        free(message);
        return;
    }
    // RPL_LISTEND
    // :<servername> 323 <nick> :End of LIST
    char* error = construct_message(RPL_LISTEND, ctx, user, res, false);
    free_tokens(res, 2);
    free(error);
    return;

}


void mode_fn(char* command_str, user* user, server_ctx* ctx)
{

    bool ischannelop = false;
    int mode = 0;
    struct user* temp;

    /* Order: COMMAND, CHANNEL, MODE, NICK */
    char** res1 = tokenize_message(command_str, " ", 4);
    channel* c = channel_lookup(res1[1], ctx->channel_list);

    /* ERRORS: ERR_NOSUCHCHANNEL, ERR_CHANOPRIVSNEEDED,
     * ERR_UNKNOWNMODE, ERR_USERNOTINCHANNEL
     */

    /* Error checks */
    if (!validate_parameters(command_str, 3))
    {
        char* error = construct_message(ERR_NOSUCHCHANNEL, ctx, user, res1, true);
        send_message(error, user);
        free_tokens(res1, 4);
        free(error);
        return;  
    }
    if (c == NULL)
    {
        char* error = construct_message(ERR_NOSUCHCHANNEL, ctx, user, res1, true);
        send_message(error, user);
        free_tokens(res1, 4);
        free(error);
        return;
    }

    struct user* u = user_lookup(ctx->user_list, 0, res1[3], 0);
    if ((u == NULL) || (channel_verifyuser(c, u) == -1))
    {
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
        // send to all the channel homies
        char* message = construct_message("MODE", ctx, user, res1, false);
        // :<user-nick> MODE <channel> <target-nick> :<mode>
        // user->nick, res1[1], res1[3], res1[2]
        // send_message
        for(temp = *c->user_list; temp != NULL; temp=temp->hh.next)
        {
            send_message(message, temp);
        }
        /// free message
        free(message);
        free_tokens(res1, 4);
        return;
    }
    else
    {
        // ERR_CHANOPRIVSNEEDED 
        char* error = construct_message(ERR_CHANOPRIVSNEEDED, ctx, user, res1, true);
        send_message(error, user);
        free_tokens(res1, 4);
        free(error);
        return;
    }
    free_tokens(res1, 4);

}

void oper_fn(char* command_str, user* user, server_ctx* ctx)
{
    
    /* Order: COMMAND, USER, PASSWORD */
    char** res1 = tokenize_message(command_str, " ",3);
    char* password = res1[2];
    bool correct_pass = false;

    if (!validate_parameters(command_str, 2))
    {
        // ERR_NEEDMOREPARAMS         
        char* error = construct_message(ERR_NEEDMOREPARAMS, ctx, user, res1, true); 
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
        
        // :<nick> MODE <nick> :+o

        // RPL_YOUREOPER
        // ":<servername> 381 <nick> :You are now an IRC operator"
        char* msg = construct_message(RPL_YOUREOPER, ctx, user, res1, false);
        send_message(msg, user);
        free_tokens(res1, 3);
        free(msg);
        return;
    }
    else
    {
        // ERR_PASSWDMISMATCH
        char* error = construct_message(ERR_PASSWDMISMATCH, ctx, user, NULL, true);
        send_message(error, user);
        free_tokens(res1, 3);
        free(error);
        return;
        // ":<servername> 464 <nick> :Password incorrect"
    }


}


