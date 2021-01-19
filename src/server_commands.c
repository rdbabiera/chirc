#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "log.h"
#include "server_commands.h"


// Initialize a user
user* user_init(int socket, struct sockaddr* sa, socklen_t salen)
{
    int status;
    user* new_user = (user*)malloc(sizeof(user));
    new_user -> nick = NULL;
    new_user -> user = NULL;
    new_user -> client_socket = socket;
    status = getnameinfo(sa, salen, new_user->client_host, 128, 
        new_user->client_service, 128, 0);

    new_user -> rpl_welcome = 0;
    return new_user;
}


// Attach nick name to user struct
void nick_fn(char* command_str, user* user) 
{
    char* command;
    char* nickname;
    char* saveptr;

    char* temp_str = strdup(command_str);
    command = strtok_r(temp_str, " ", &saveptr);
    nickname = strtok_r(NULL, " ", &saveptr);

    chilog(INFO, "Nickname: %s registered", nickname);

    user->nick = strdup(nickname);
    free(temp_str);
    
}

// Attach user name to user struct
void user_fn(char* command_str, user* user)
{
    
    char *command_line, *full_name, *command, *username, *ignore1, *ignore2, *token;
    char *saveptr1, *saveptr2;

    char* temp_str = strdup(command_str);

    // Split the string by ":" to pull out command and the full name
    command_line = strtok_r(temp_str, ":", &saveptr1);
    full_name = strtok_r(NULL, ":", &saveptr1);
    

    // Split the string by " " to get the username
    command = strtok_r(command_line, " ", &saveptr2);
    username = strtok_r(NULL, " ", &saveptr2);
    ignore1 = strtok_r(NULL, " ", &saveptr2);
    ignore2 = strtok_r(NULL, " ", &saveptr2);

    chilog(INFO, "Username: %s registered.", username);
    chilog(INFO, "Full name: %s registered.", full_name);

    user->user = strdup(username);
    user->full_name = strdup(full_name);
    free(temp_str);

}

// Match message to a viable command
void match(char* command_str, user* user)
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
            command_arr[i].execute_cmd(command_str, user);
        }
    }


}