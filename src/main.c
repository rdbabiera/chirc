/*
 *
 *  chirc: a simple multi-threaded IRC server
 *
 *  This module provides the main() function for the server,
 *  and parses the command-line arguments to the chirc executable.
 *
 */

/*
 *  Copyright (c) 2011-2020, The University of Chicago
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or withsend
 *  modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  - Neither the name of The University of Chicago nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software withsend specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY send OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "log.h"
#include "commands.h"


int main(int argc, char *argv[])
{
    int opt;
    char *port = "6667", *passwd = NULL, *servername = NULL, *network_file = NULL;
    int verbosity = 0;

    while ((opt = getopt(argc, argv, "p:o:s:n:vqh")) != -1)
        switch (opt)
        {
        case 'p':
            port = strdup(optarg);
            break;
        case 'o':
            passwd = strdup(optarg);
            break;
        case 's':
            servername = strdup(optarg);
            break;
        case 'n':
            if (access(optarg, R_OK) == -1)
            {
                printf("ERROR: No such file: %s\n", optarg);
                exit(-1);
            }
            network_file = strdup(optarg);
            break;
        case 'v':
            verbosity++;
            break;
        case 'q':
            verbosity = -1;
            break;
        case 'h':
            printf("Usage: chirc -o OPER_PASSWD [-p PORT] [-s SERVERNAME] [-n NETWORK_FILE] [(-q|-v|-vv)]\n");
            exit(0);
            break;
        default:
            fprintf(stderr, "ERROR: Unknown option -%c\n", opt);
            exit(-1);
        }

    if (!passwd)
    {
        fprintf(stderr, "ERROR: You must specify an operator password\n");
        exit(-1);
    }

    if (network_file && !servername)
    {
        fprintf(stderr, "ERROR: If specifying a network file, you must also specify a server name.\n");
        exit(-1);
    }

    /* Set logging level based on verbosity */
    switch(verbosity)
    {
    case -1:
        chirc_setloglevel(QUIET);
        break;
    case 0:
        chirc_setloglevel(INFO);
        break;
    case 1:
        chirc_setloglevel(DEBUG);
        break;
    case 2:
        chirc_setloglevel(TRACE);
        break;
    default:
        chirc_setloglevel(TRACE);
        break;
    }

    /* Your code goes here */
    /**************** Functions for Handling Sockets ****************/
    int passive_socket, active_socket;
    struct addrinfo hints, *res, *p;
    struct sockaddr_in server_addr, client_addr;
    char hostname[128];
    int yes = 1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port, &hints, &res) != 0) {
        perror("getaddrinfo() failed");
        exit(-1);
    }

    for (p = res; p != NULL; p = p->ai_next) {
        if ((passive_socket = socket(p->ai_family, p->ai_socktype, 
                p->ai_protocol)) == -1) {
            perror("Could not open socket");
            continue;
        }

        if (bind(passive_socket, p->ai_addr, p->ai_addrlen) == -1) {
            close(passive_socket);
            perror("Could not bind");
            continue;
        }

        break;
    }

    free(res);

    if (p == NULL) {
        fprintf(stderr, "failed to bind socket\n");
        exit(2);
    }

    gethostname(hostname, sizeof hostname);

    if (listen(passive_socket, 5) == -1){
        perror("Socket listen() failed");
        close(passive_socket);
        exit(-1);
    }

    chilog(INFO, "Waiting for a connection...\n");

    socklen_t sin_size = sizeof(struct sockaddr_in);
    active_socket = accept(passive_socket, (struct sockaddr *) &client_addr, 
            &sin_size);

    if (active_socket == -1) {
        perror("Socket accept() failed");
        close(passive_socket);
        exit(-1);
    }

    /**************** Functions for Handling Commands ****************/

    /* RD's Parsing Algorithm:
     * - Add buffer to message array
     * - Search through message array to see if there is "\r\n"
     * - If "\r\n" is present, process message immediately
     * - Continue to check if current message has any other completed commands
     * - Repeat
     */

    char buff[513];
    char msg[513]; // needs to be copied, from tokens.
    char** tokens;
    char *saveptr, *saveptr_2;
    int recv_status;
    char* carr_found;
    int token_count;
    int msg_offset = 0;

    char command_current[128];
    int command_length = 0;
    int remaining_length = 0;

    token_count = 1;

    while(1){
        if ((recv_status = recv(active_socket, buff, 512, 0)) == -1) {
            perror("Socket recv() failed");
            close(active_socket);
            close(passive_socket);
            exit(-1);
        }

        strncpy(msg + msg_offset, buff, recv_status);
        msg_offset += recv_status;

        carr_found = strstr(msg, "\r\n");
        while (carr_found != NULL){
            command_length = carr_found - msg;
            strncpy(command_current, msg, command_length);
            remaining_length = msg_offset - command_length - 2;
            memmove(msg, msg + command_length + 2, remaining_length);
            memset(msg + remaining_length, 0, 513 - remaining_length);


            // Essentially what we have now is command_current holding a 
            // ready to process command, and msg pointing to the current rest of the buffer.

            // Here: Process Message. It gets cleared after processing.
            // process_message() - does not exist. try to think of a message processing 
            // algorithm and how you think we should implement it in terms of architecture
            // as well.

            //Process message
            match(msg, curr_user)


            memset(command_current, 0, 128);
            // Here: rerun strstr to find next valid command.
            carr_found = strstr(msg, "\r\n");
            msg_offset = remaining_length;
        }

    }

    char* message = "temp\n";
    if (send(active_socket, message, strlen(message), 0) == -1){
        perror("Socket send() failed");
        close(active_socket);
        close(passive_socket);
        exit(-1);
    }

    close(active_socket);

    close(passive_socket);

    return 0;
}

