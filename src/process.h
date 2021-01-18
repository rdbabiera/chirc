


/*
 * Parsing and processing command functions
 * 
 * 
 * We use these functions to parse messages that are received
 * from the socket, and process and execute them. This includes
 * building messages to be sent back to the client. 
 */

#ifndef CHIRC_PROCESS_H_
#define CHIRC_PROCESS_H_

/* command_count() - returns how many completed commands are found in the */
int command_count(char* buff, int len);