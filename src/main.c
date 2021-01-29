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
#include <pthread.h>

#include "users.h"
#include "channels.h"
#include "server_info.h"
#include "message.h"
#include "reply.h"
#include "log.h"

#include "uthash.h"
#include "utlist.h"



/* Function that runs in each thread to service a single client */
void *service_single_client(void *args)
{
    worker_args *wa;
    user* curr_user;
    server_ctx* ctx;

    /* Unpack arguemnts */
    wa = (worker_args*) args;
    curr_user = wa->curr_user;
    ctx = wa->server_ctx;

    /*  Detach Thread */
    pthread_detach(pthread_self());
    chilog(INFO, "Socket connected\n");

    /* Parameters for Receiving Data */
    char buff[MAX_BUFF_SIZE]; 
    char msg[MAX_BUFF_SIZE]; 
    int recv_status;
    char* carr_found;
    long msg_offset = 0;

    char command_current[MAX_COMM_SIZE];
    long command_length = 0;
    long remaining_length = 0;


    while(1)
    {
        chilog(INFO, "Waiting for a message...\n");
        if ((recv_status = recv(curr_user->client_socket, buff, 512, 0)) == -1)
        {
            perror("Socket recv() failed");
            close(curr_user->client_socket);
            free(wa);
            pthread_exit(NULL);
        }

        /* Transfer buffer to message */
        strncpy(msg + msg_offset, buff, recv_status);
        msg_offset += ((long)recv_status);

        /* Check message for carriage return ("\r\n") */
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

            /*Process message*/
            chilog(INFO, "%d Sent Command: %s\n", curr_user->client_socket, command_current);
            match(command_current, curr_user, ctx);

            /* Clean out message */
            memset(command_current, 0, sizeof command_current);

            /* Run strstr again to find the next command, if there is one */
            carr_found = strstr(msg, "\r\n");
            msg_offset = remaining_length;
        }

        
    }

        pthread_exit(NULL);
}


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
    /**************** Code to manage Server Context *****************/
    server_ctx* server_ctx = (struct server_ctx*)malloc(sizeof(struct server_ctx));
    server_ctx->user_list = (user**)malloc(sizeof(user*));
    *server_ctx->user_list = NULL;
    server_ctx->channel_list = (channel**)malloc(sizeof(channel*));
    *server_ctx->channel_list = NULL;
    server_ctx->channel_count = 0;
    server_ctx->operator_password = passwd;

    /**************** Functions for Handling Sockets ****************/
    int passive_socket, active_socket;
    pthread_t worker_thread;
    struct addrinfo hints, *res, *p;
    struct sockaddr_in *server_addr, *client_addr;
    socklen_t sin_size = sizeof(struct sockaddr_in);
    struct worker_args *wa;
    int yes = 1;
    user* curr_user = NULL;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    char hostname[128];

    /* Getting address information */
    if (getaddrinfo(NULL, port, &hints, &res) != 0) 
    {
        perror("getaddrinfo() failed");
        exit(-1);
    }

    /* Iterates through sockets and binds to the first one available */
    for (p = res; p != NULL; p = p->ai_next)
    {
        if ((passive_socket = socket(p->ai_family, p->ai_socktype, 
            p->ai_protocol)) == -1)
        {
            perror("Could not open socket");
            continue;
        }

        if (setsockopt(passive_socket, SOL_SOCKET, SO_REUSEADDR, 
            &yes, sizeof(int)) == -1)
        {
            perror("Socket setsockopt() failed");
            close(passive_socket);
            continue;
        }

        if (bind(passive_socket, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(passive_socket);
            perror("Could not bind");
            continue;
        }

        if (listen(passive_socket, 20) == -1)
        {
            perror("Socket listen() failed");
            close(passive_socket);
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

    /* Get hostname for the server */
    int host_check = gethostname(hostname, sizeof hostname);
    if (host_check == -1) 
    {
        perror("gethostname() failed");
        exit(2);
    }

    server_ctx->server_name = hostname;

    chilog(INFO, "Waiting for a connection...\n");

    while (1)
    {
        /* Create an active socket */
        client_addr = calloc(1, sin_size);
        active_socket = accept(passive_socket, (struct sockaddr *) client_addr, 
            &sin_size);

        if (active_socket == -1)
        {
            free(client_addr);
            perror("Could not accept() connection");
            continue;
        }

        user* curr_user = user_init(active_socket, (struct sockaddr*) client_addr, 
                                    sin_size);

        /* add user management to server here */
        chilog(INFO, "About to add\n");
        HASH_ADD_INT(*server_ctx->user_list, client_socket, curr_user);
        chilog(INFO, "Added\n");
        
        wa = calloc(1, sizeof(worker_args));
        /* add worker args here */
        wa->curr_user = curr_user;
        wa->server_ctx = server_ctx;

        if (pthread_create(&worker_thread, NULL, service_single_client, wa) != 0)
        {
            perror("Could not create a worker thread");
            free(client_addr);
            free(wa);
            HASH_DEL(*(server_ctx->user_list), curr_user);
            close(active_socket);
            close(passive_socket);
            return EXIT_FAILURE;
        }

    }

    return EXIT_SUCCESS;
}
