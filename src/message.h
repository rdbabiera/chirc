/*
 * Client message manager
 * 
 * This file will hold all functions and structs that relate to constructing messages
 * that send information back to the client or between clients.
 * 
 */

#ifndef CHIRC_MESSAGE_H_
#define CHIRC_MESSAGE_H_

#include "users.h"

/*
 * send_message - sends a message to a client
 * 
 * 
 */
void send_message(char* message, user* user_dest);


/*
 * construct_message - constructs messages to be sent back to clients based on
 *                     command inputted 
 * 
 * command: the socket identifier for a client connected to the server
 * 
 * user_src: info for user sending messages
 * 
 * user_dest: info for user receiving messages
 * 
 * extra1 & extra2: extra input
 * 
 * Returns: Char*
 * 
 */
char* construct_message(char* command, user* user_src, user* user_dest,
                        char* extra1, char* extra2);


/*
 * tokenize message - strtoks a string and returns results in an array of 
 * strings
 * 
 * message: message as string to be tokenized
 * 
 * tokenizer: character used to split up string
 * 
 * tokens: tokens as a result of strtoking the message with the tokenizer
 * 
 * Returns: A list of tokens as a result of strtok
 * 
 */
char** tokenize_message(char* message, char* tokenizer, int tokens);


/*
 * free_tokens - frees all tokens from tokenize_message's resulting array
 * 
 * tokens: list of tokens to be freed
 * 
 * num_tokens: number of tokens that need to be freed
 * 
 * Returns: Nothing
 * 
 */
void free_tokens(char** tokens, int num_tokens);


/*
 * validate_parameters - validates the number of parameters in a command
 * 
 */
int validate_parameters(char* command, int target_params);



#endif /* CHIRC_MESSAGE_H_ */