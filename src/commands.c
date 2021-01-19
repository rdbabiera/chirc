#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "log.h"
#include "commands.h"



// Attach nick name to user struct
void nick_fn(char* command_str, user user) 
{
    char* command;
    char* nickname;
    char* saveptr1, saveptr2;

    command = strtok_r(command_str, " ", &saveptr1);
    nickname = strtok_r(NULL, " ", &saveptr2);

    chilog(INFO, "Nickname: %s registered", nickname);

    user.nick = nickname;
    
}

// Attach user name to user struct
void user_fn(char* command_str, user user)
{

    char* command_line, full_name, command, username, ignore1, ignore2, token;
    char* saveptr1, saveptr2;

    // Split the string by ":" to pull out command and the full name
    command_line = strtok_r(command_str, ":", &saveptr1);
    full_name = strtok_r(command_str, ":", &saveptr2);

    // Split the string by " " to get the username
    command = strtok_r(command_line, " ", &saveptr1);
    username = strtok_r(NULL, " ", &saveptr2);
    ignore1 = strtok_r(NULL, " ", &saveptr2);
    ignore2 = strtok_r(NULL, " ", &saveptr2);

    chilog(INFO, "Username: %s registered.", username);
    chilog(INFO, "Full name: %s registered.", full_name);

    user.user = username;
    user.full_name = full_name;


}

// Match message to a viable command
void match(char* command_str, user user)
{

    char* saveptr1;

    // Filling out command array (this will increased in Assignment 1b)
    cmd command_arr[2] = {
                {"NICK", nick_fn}, 
                {"USER", user_fn}
                };

    // Getting command from command_str
    char* command = strtok_r(command_str, " ", &saveptr1);


    for(int i = 0; i < 2; i++)
    {
        if (command_arr[i].cmd_name == command) {
            command_arr[i].execute_cmd(command_str, user);
        }
    }


}