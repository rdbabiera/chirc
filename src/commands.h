/*
 * 
 * Command functions
 * 
 * This file is to help with parsing and executing commands. Also includes a struct
 * to keep track of user information. 
 * 
 * Credit: A friend of Lucy's, Ajay Chopra, who took this class previously, 
 * suggested the command struct when she was discussing about parsing through 
 * commands with him. 
 */

#ifndef CHIRC_COMMANDS_H_
#define CHIRC_COMMANDS_H_

/**************** Structs for Parsing Commands ****************/
// User struct
typedef struct user {
    char* nick;
    char* user;
    char* full_name;
} user;

// Command struct
typedef struct command {
    char* cmd_name;
    void (*execute_cmd)(char*, user);
} cmd;


/**************** Functions for Parsing Commands ****************/

/*
 * nick_fn - Attach NICK to user
 * 
 * command_str: command string to be parsed
 * 
 * user: user who we are giving nickname to
 * 
 * Returns: nothing (should only change user info)
 */
void nick_fn(char* command_str, user user);

/*
 * user_fn - Attach USER to user
 * 
 * command_str: command string to be parsed
 * 
 * user: user who we are giving user name to
 * 
 * Returns: nothing (should only change user info)
 */
void user_fn(char* command_str, user user);

/*
 * match - Match current command with a possible command and execute
 * 
 * command_str: command string to be checked and matched
 * 
 * user: user who we are executing functions for
 * 
 * Returns: nothing
 */
void match(char* command_str, user user);

#endif /* CHIRC_COMMANDS_H_ */