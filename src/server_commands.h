/*
 * 
 * Server command functions
 * 
 * This file is to help with commands that are received by the server
 * 
 * Credit: A friend of Lucy's, Ajay Chopra, who took this class previously, 
 * suggested the command struct when she was discussing about parsing through 
 * commands with him. 
 */

#ifndef CHIRC_SERVER_COMMANDS_H_
#define CHIRC_SERVER_COMMANDS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/**************** Structs for Parsing Commands ****************/
// User struct
typedef struct user {
    char* nick;
    char* user;
    char* full_name;
    int client_socket;
    char client_host[128];
    char client_service[128];

    // Flags
    int rpl_welcome;
} user;

// Command struct
typedef struct command {
    char* cmd_name;
    void (*execute_cmd)(char*, user*);
} cmd;


/**************** Functions for User Initialization ****************/

/*
 * user_init - Initialize a user
 * 
 * socket: the socket identifier for a client connected to the server
 * sa: the sockaddr info of the client
 * salen: the sockaddr len of the sa
 * 
 * Returns: User
 */
user* user_init(int socket, struct sockaddr* sa, socklen_t salen);


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
void nick_fn(char* command_str, user* user);

/*
 * user_fn - Attach USER to user
 * 
 * command_str: command string to be parsed
 * 
 * user: user who we are giving user name to
 * 
 * Returns: nothing (should only change user info)
 */
void user_fn(char* command_str, user* user);

/*
 * match - Match current command with a possible command and execute
 * 
 * command_str: command string to be checked and matched
 * 
 * user: user who we are executing functions for
 * 
 * Returns: nothing
 */
void match(char* command_str, user* user);

#endif /* CHIRC_SERVER_COMMANDS_H_ */