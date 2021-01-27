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
#include "parse_util.h"
#include "message.h"
#include "construct_msg.h"


/**************** Functions for Constructing a Message ****************/

/* Construct message to be sent back to client depending on command */
char* construct_message(char* msg, server_ctx* ctx, user* user, char** params,
                        bool error)
{
    char* res = (char*)malloc(sizeof(char)*MAX_BUFF_SIZE);
    int status;

    /* Errors */
    if (error == true) 
    {

        /* Nickname errors */
        if (!strncmp(msg, ERR_NICKNAMEINUSE, ERROR_SIZE))
        {
            status = sprintf(res, ":%s %s %s %s :Nickname is already in use\r\n",
                            ctx->server_name, ERR_NICKNAMEINUSE,user->nick, params[1]);
        }
        else if (!strncmp(msg, ERR_NONICKNAMEGIVEN, ERROR_SIZE))
        {
            status = sprintf(res, ":%s %s %s :No nickname given\r\n", 
                            ctx->server_name, ERR_NONICKNAMEGIVEN,user->nick);
        }

        /* Username errors */
        else if (!strncmp(msg, ERR_ALREADYREGISTRED, ERROR_SIZE))
        {
            status = sprintf(res, ":%s %s %s :Unauthorized command (already "
                            "registered)\r\n", ctx->server_name, ERR_ALREADYREGISTRED,
                           user->nick);
        }
        
        /* Privmsg and notice errors */
        else if (!strncmp(msg, ERR_NOTEXTTOSEND, ERROR_SIZE))
        {
            status = sprintf(res, ":%s %s %s :No text to send. \r\n", 
                            ctx->server_name, ERR_NOTEXTTOSEND, user->nick);
        }
        else if (!strncmp(msg, ERR_NORECIPIENT, ERROR_SIZE))
        {
            status = sprintf(res, ":%s %s %s :No recipient given %s \r\n", 
                            ctx->server_name, ERR_NORECIPIENT, user->nick, 
                            params[0]);
        }
        else if (!strncmp(msg, ERR_NOSUCHNICK, ERROR_SIZE))
        {
            status = sprintf(res, ":%s %s %s : %s :No such nick/channel\r\n",
                            ctx->server_name, ERR_NOSUCHNICK, user->nick, params[1]);
        }

        /* General errors */
        else if (!strncmp(msg, ERR_NEEDMOREPARAMS, ERROR_SIZE))
        {
            status = sprintf(res, ":%s %s %s %s :Not enough parameters\r\n",
                            ctx->server_name, ERR_NEEDMOREPARAMS,user->nick, 
                            params[0]);
        }
        else if (!strncmp(msg, ERR_NOTREGISTERED, ERROR_SIZE))
        {
            status = sprintf(res, ":%s %s %s :You have not registered\r\n",
                            ctx->server_name, ERR_NOTREGISTERED, user->nick);
        }
        else if (!strncmp(msg, ERR_UNKNOWNCOMMAND, ERROR_SIZE))
        {
            status = sprintf(res, "%s %s %s %s :Unknown command\r\n",
                            ctx->server_name, ERR_UNKNOWNCOMMAND, user->nick, 
                            params[0]);
        }

        /* MOTD error */
        else if (!strncmp(msg, ERR_NOMOTD, ERROR_SIZE))
        {
            status = sprintf(res, ":%s %s %s :MOTD File is missing\r\n",
                            ctx->server_name, ERR_NOMOTD, user->nick);
        }
    }

    /* General messages */
    else
    {
        /* Welcome messages*/
        if (!strncmp(msg, RPL_WELCOME, ERROR_SIZE))
        {
            status = sprintf(res, ":%s %s %s :Welcome to the Internet Relay Network "
                            "%s!%s@%s\r\n", ctx->server_name, RPL_WELCOME, 
                           user->nick,user->nick, user->username, user->client_host);
        }
        else if (!strncmp(msg, RPL_YOURHOST, ERROR_SIZE))
        {
            status = sprintf(res, ":%s %s %s :Your host is %s, running version "
                            "420.69\r\n", ctx->server_name, RPL_YOURHOST,user->nick, 
                            ctx->server_name);
        }
        else if (!strncmp(msg, RPL_CREATED, ERROR_SIZE))
        {
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            status = sprintf(res, ":%s %s %s :This server was created "
                            "%d-%02d-%02d at %02d:%02d:%02d\r\n",
                            ctx->server_name, RPL_CREATED,user->nick, 
                            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, 
                            tm.tm_hour, tm.tm_min, tm.tm_sec);
        }
        else if (!strncmp(msg, RPL_MYINFO, ERROR_SIZE))
        {
            status = sprintf(res, ":%s %s %s %s 420.60 ao mtov\r\n", 
                            ctx->server_name, RPL_MYINFO, user->nick, 
                            ctx->server_name);
        }

        /* Quit messages */
        else if (!strncmp(msg, "QUIT", 4))
        {
            status = sprintf(res, "ERROR :Closing Link: %s (%s)\r\n", 
                            user->client_host, params[1]);
        }

        /* Private message and notice messages */
        else if (!strncmp(msg, "PRIVMSG", 7))
        {
            status = sprintf(res, ":%s!%s@%s PRIVMSG %s : %s\r\n", 
                            user->nick, user->username, user->client_host, 
                            params[1], params[2]);
        }

        /* Ping and pong messages */
        else if (!strncmp(msg, "PONG", 4))
        {
            status = sprintf(res, ":%s PONG %s :%s\r\n", 
                            ctx->server_name, user->nick, ctx->server_name);
        }

        
        /* LUSERS messages */
        else if (!strncmp(msg, RPL_LUSERCLIENT, ERROR_SIZE))
        {
            status = sprintf(res, ":%s %s %s :There are %s users and %s services "
                            "on 1 server.\r\n", ctx->server_name, RPL_LUSERCLIENT, 
                            user->nick, params[0], params[4]);
        }
        else if (!strncmp(msg, RPL_LUSEROP, ERROR_SIZE))
        {
            status = sprintf(res, ":%s %s %s %s :operator(s) online\r\n", 
                            ctx->server_name, RPL_LUSEROP, user->nick, params[3]);
     
        }
        else if (!strncmp(msg, RPL_LUSERUNKNOWN, ERROR_SIZE))
        {
            status = sprintf(res, ":%s %s %s %s :unknown connection(s)\r\n",
                            ctx->server_name, RPL_LUSERUNKNOWN, user->nick, 
                            params[1]);
        }
        else if (!strncmp(msg, RPL_LUSERCHANNELS, ERROR_SIZE))
        {
            status = sprintf(res, ":%s %s %s %s :channels formed\r\n", 
                            ctx->server_name, RPL_LUSERCHANNELS, user->nick,
                            params[2]);
        }
        else if (!strncmp(msg, RPL_LUSERME, ERROR_SIZE))
        {
            status = sprintf(res, ":%s %s %s :I have %s clients and %s "
                            "servers\r\n", ctx->server_name, RPL_LUSERME,
                            user->nick, params[0], params[5]);
        }

        /* WHOIS messages */
        else if (!strncmp(msg, RPL_WHOISUSER, ERROR_SIZE))
        {
            status = sprintf(res, "%s %s %s %s %s %s * :%s\r\n", ctx->server_name,
                            RPL_WHOISUSER, user->nick, user->nick, user->username,
                            user->client_host, user->full_name);
        }
        else if (!strncmp(msg, RPL_WHOISSERVER, ERROR_SIZE))
        {
            status = sprintf(res, "%s %s %s %s %s :%s\r\n", ctx->server_name, 
                            RPL_WHOISSERVER, user->nick, params[1], ctx->server_name,
                            "dog");
        }
        else if (!strncmp(msg, RPL_ENDOFWHOIS, ERROR_SIZE))
        {
            status = sprintf(res, "%s %s %s %s :End of WHOIS list\r\n", 
                            ctx->server_name, RPL_ENDOFWHOIS, user->nick, params[1]);
        }
    }

    return res;
}
