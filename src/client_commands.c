#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "log.h"
#include "reply.h"
#include "client_commands.h"
#include "server_commands.h"

// Construct message to be sent back to client
char* construct_message(char* command, user* user_src, user* user_dest, 
                        char* extra1, char* extra2)
{
    char* res = (char*)malloc(sizeof(char)*256);
    int status;
    if (!strcmp(command, RPL_WELCOME))
    {
        status = sprintf(res, 
            ":%s 001 %s :Welcome to the Internet Relay Network %s!%s@%s\r\n", 
            extra1, user_dest->nick, user_dest->nick, user_dest->user,
            user_dest->client_host);
    }
    return res;
}