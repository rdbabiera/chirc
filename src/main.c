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
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>


#include "log.h"
#include "reply.h"
#include "client_commands.h"
#include "server_commands.h"



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

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Getting address information
    if (getaddrinfo(NULL, port, &hints, &res) != 0) 
    {
        perror("getaddrinfo() failed");
        exit(-1);
    }

    // Iterates through sockets and binds to the first one available
    for (p = res; p != NULL; p = p->ai_next)
    {
        if ((passive_socket = socket(p->ai_family, p->ai_socktype, 
             p->ai_protocol)) == -1)
        {
            perror("Could not open socket");
            continue;
        }

        if (bind(passive_socket, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(passive_socket);
            perror("Could not bind");
            continue;
        }

        break;
    }

    free(res);

    if (p == NULL)
    {
        fprintf(stderr, "failed to bind socket\n");
        exit(2);
    }

    // Get hostname for the server
    gethostname(hostname, sizeof hostname);

    // Listen to socket
    if (listen(passive_socket, 5) == -1)
    {
        perror("Socket listen() failed");
        close(passive_socket);
        exit(-1);
    }

    chilog(INFO, "Waiting for a connection...\n");

    // Create an active socket 
    socklen_t sin_size = sizeof(struct sockaddr_in);
    active_socket = accept(passive_socket, (struct sockaddr *) &client_addr, 
            &sin_size);

    if (active_socket == -1)
    {
        perror("Socket accept() failed");
        close(passive_socket);
        exit(-1);
    }

    // Initialize user
    user* curr_user = user_init(active_socket, (struct sockaddr*) &client_addr, sin_size);
    

    /**************** Functions for Handling Commands ****************/

    /* RD and Lucy's Parsing Algorithm:
     * - Add buffer to message array
     * - Search through message array to see if there is "\r\n"
     * - If "\r\n" is present, process message immediately
     * - Continue to check if current message has any other completed commands
     * - Repeat
     */

    char buff[513]; // buffer for messages
    char msg[513]; 
    int recv_status;
    char* carr_found;
    long msg_offset = 0;

    char command_current[128];
    long command_length = 0;
    long remaining_length = 0;


    // Continue receiving from client until loop is borken
    chilog(INFO, "Waiting to receive...\n");

    while(1)
    {
        if ((recv_status = recv(active_socket, buff, 512, 0)) == -1)
        {
            perror("Socket recv() failed");
            close(active_socket);
            close(passive_socket);
            exit(-1);
        }

        // Transfer buffer to message
        strncpy(msg + msg_offset, buff, recv_status);
        msg_offset += ((long)recv_status);

        // Check message for carriage return ("\r\n")
        carr_found = strstr(msg, "\r\n");

        while (carr_found != NULL)
        {
            /* If a \r\n is found within the message, take that out for
             * processing. After taking it out, shift the pointer for the 
             * beginning of "msg" to be at the location right after the \r\n
             * that was found 
             */
            command_length = carr_found - msg;
            strncpy(command_current, msg, command_length);
            command_current[command_length] = 0;
            remaining_length = msg_offset - command_length - 2;
            memmove(msg, msg + command_length + 2, remaining_length);
            memset(msg + remaining_length, 0, 513 - remaining_length);

            //Process message
            match(command_current, curr_user);

            //Clean out message
            memset(command_current, 0, sizeof command_current);

            // Run strstr again to find the next command, if there is one
            carr_found = strstr(msg, "\r\n");
            msg_offset = remaining_length;
        }

        // If the wecome message has not been sent, send it. 
        if (curr_user->rpl_welcome == false)
        {
            if ((curr_user->user != NULL) && (curr_user->nick != NULL))
            {
                curr_user->rpl_welcome = true;
                char* welcome = construct_message(RPL_WELCOME, NULL, curr_user, 
                                hostname, NULL);
                if (send(active_socket, welcome, strlen(welcome), 0) == -1)
                {
                    perror("Socket send() failed");
                    close(active_socket);
                    close(passive_socket);
                    exit(-1);
                }
                free(welcome);
            }
        }
    }

    // Closing sockets
    close(active_socket);
    close(passive_socket);

    return 0;
}

