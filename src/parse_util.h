/*
 * Message utility functions
 * 
 * This file will hold functions that are utilized by the message file. This will
 * hopefully increase the depth of modularity, as these functions are not 
 * directly accessed in order to send messages to user.
 * 
 */

#ifndef CHIRC_MESSAGE_UTIL_H_
#define CHIRC_MESSAGE_UTIL_H_


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>

#include "users.h"

#include "uthash.h"
#include "utlist.h"


/**************** Functions for Tokenizing Messages ****************/

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
int validate_parameters(char* command, int target_params, user* user);



#endif /* CHIRC_MESSAGE_UTIL_H_ */
