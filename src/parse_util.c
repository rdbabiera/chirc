#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#include "reply.h"
#include "users.h"
#include "log.h"
#include "parse_util.h"
#include "util.h"
#include "channels.h"
#include "server_info.h"
#include "construct_msg.h"


/**************** Functions for Tokenizing Messages ****************/

/* Tokenize message by splitting with tokenizer */
char** tokenize_message(char* message, char* tokenizer, int num_tokens)
{
    char *saveptr, *curr_token;
    int i;

    char* temp_str = strdup(message);
    char** res = (char**)malloc(sizeof(char*) * num_tokens);

    for (i=0; i<num_tokens; i++)
    {
        if (i == 0)
        {
            curr_token = strtok_r(temp_str, tokenizer, &saveptr);
            res[0] = strdup(curr_token);
        } 
        else
        {
            curr_token = strtok_r(NULL, tokenizer, &saveptr);
            if (curr_token != NULL)
            {
                res[i] = strdup(curr_token);
            }
            else
            {
                res[i] = NULL;
            }
            
        }
    }
    free(temp_str);
    return res;
}


/* Free memory that was held for tokens */
void free_tokens(char** tokens, int num_tokens)
{
    int i;
    for (i=0; i<num_tokens; i++){
        free(tokens[i]);
    }
    free(tokens);
}


/* Validate that the number of parameters is correct */
bool validate_parameters(char* command, int target_params)
{
    int count = -1;
    char *saveptr, *token;
    char* temp_str = strdup(command);

    token = strtok_r(temp_str, " ", &saveptr);
    while(token != NULL)
    {
        count++;
        token = strtok_r(NULL, " ", &saveptr);
    }
    if (count < target_params)
    {
        return false;
    }
    return true;
}


