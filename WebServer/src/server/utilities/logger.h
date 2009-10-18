/* Implementation of the logging facility. This file creates its own Thread
 * handling all the incoming logging requests issued by the server itself.
 * This decouples the logging from the real server functionalities in order
 * to be non blocking whenever logging could take more time. Sll this is
 * secured by Semaphores and Mutexes.
 * It offers error and access logging.
 *
 * Copyright (C) 2009 	Irene Moriggl (irenemoriggl at gmail dot com)
 * 						Hannes Tribus (hons82 at gmail dot com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef LOGGER_H_
#define LOGGER_H_

#include <syslog.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>

#include "configHandler.h"
#include "dateUtils.h"
#include "memoryUtils.h"

enum SEVERITY {EMERGENCY, ALERT, CRITICAL, ERROR, WARNING, NOTICE, INFO, DEBUG};

/*/
 * structure for info on access to server
 * ensure that ident, auth, request are possible to manipulate. Don't assign with " ".
/*/
struct access_log_entry {
	char* ip;
	char* ident;
	char* auth;
	char* action;
	char* request;
	char* code;
	char* bytes;
};
/*
 * This method shuts the logging thread down and cleans up the remaining memory
 */
void stop_logger(int sig_no);
/*
 * creates the logging thread and its semaphore
 * returns 1 on success, 0 otherwise
 */
int setup_logger();
/*
 * transforms the entry into an access log message (common log format)
 * lets the logging thread log the msg to the file or to syslog (if filename is NULL)
 * severity is used when logging to syslog
 */
void bth_log_access(struct access_log_entry entry, enum SEVERITY severity);
/*
 * lets the logging thread log an error message to the file or to syslog (if filename is NULL)
 * severity is used when logging to syslog
 */
void bth_log_error(char* ip, char* msg, enum SEVERITY severity);

#endif /* LOGGER_H_ */
