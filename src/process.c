/* 
 * process.c:
 * 
 * this module contains functions that parses messages received from the
 * socket, processing, and executing them. This, in addition to constructing 
 * messages to be sent back to the client.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "process.h"