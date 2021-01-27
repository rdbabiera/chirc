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
    user** operator_list;

    // For uthash
    UT_hash_handle hh;
} channel;

/***************** Functions *****************/

/*
 * channel_init - Initialize a channel
 * 
 * channel_name: name of the channel
 * 
 * cd: channel descriptor
 * 
 * chanel_list: list of channels
 * 
 * Returns: (struct) channel
 */
channel* channel_init(char* channel_name, int cd, channel** channel_list);


/*
 * channel_delchannel - Delete a channel
 * 
 * target: channel to be deleted
 * 
 * channel_list: list of channels
 * 
 * Returns: nothing
 * 
 */
void channel_delchannel(channel* target, channel** channel_list);


/*
 * channel_lookup - Lookup a certain channel
 * 
 * channel_name: name of channel to look up
 * 
 * channel_list: list of channels
 *
 * Returns: (struct) channel
 * 
 */
channel* channel_lookup(char* channel_name, channel** channel_list);


/*
 * channel_verifyuser - verify user is on a specific channel
 * 
 * channel: channel to check for user
 * 
 * user: user to check channel for
 * 
 * Returns: boolean
 * 
 */
bool channel_verifyuser(channel* channel, user* user);


/*
 * channel_verify - verify operator is on a specific channel
 * 
 * channel: channel to check if user is operator for
 * 
 * user: user to check 
 * 
 * Returns: boolean
 */
bool channel_verifyoperator(channel* channel, user* user);


/*
 * channel-adduser - Add a user to a channel
 * 
 * channel: channel to add user to
 * 
 * user: user to add to channel
 * 
 * Returns: nothing
 * 
 */
void channel_adduser(channel* channel, user* user);


/*
 * channel_addoperator - Add an operator to a channel
 * 
 * channel: channel to add operator to
 * 
 * user: user to make operator
 * 
 * Returns: nothing
 * 
 */
void channel_addoperator(channel* channel, user* user);


/*
 * channel_deluser - Delete a user from a channel
 * 
 * channel: channel to delete user from
 * 
 * user: user to delete from channel
 * 
 * Returns: nothing
 * 
 */
void channel_deluser(channel* channel, user* user);


/*
 * channel_deop - De-op a user in a channel
 * 
 * channel: channel to de-op user from
 * 
 * user: user to de-op
 * 
 * Returns: nothing
 * 
 */
void channel_deop(channel* channel, user* user);


#endif /* CHIRC_CHANNELS_H_ */