/*
 * 
 * Channel functions
 * 
 * This file is to help with managing channels on the server
 * 
 *  
 */

#ifndef CHIRC_CHANNELS_H_
#define CHIRC_CHANNELS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <pthread.h>

#include "uthash.h"
#include "utlist.h"
#include "users.h"

/* 
 * Channel Struct - Keeps track of channel information
 * 
 */
typedef struct channel {
    char* channel_name;
    int channel_descriptor;
    int num_users;

    user** user_list;

    // For uthash
    UT_hash_handle hh;
} channel;

/***************** Functions *****************/

/*
 * channel_init - Initialize a channel
 * 
 * 
 * Returns: (struct) User
 */
channel* channel_init(char* channel_name, int cd, channel** channel_list);

/*
 *
 * 
 */
channel* channel_lookup(char* channel_name, channel** channel_list);


/*
 *
 * 
 */
int channel_verifyuser(channel* channel, user* user);


/*
 *
 * 
 */
void channel_adduser(channel* channel, user* user);


/*
 *
 * 
 */
void channel_deluser(channel* channel, user* user);


#endif /* CHIRC_CHANNELS_H_ */