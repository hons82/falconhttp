/* This methods implement all the different types of servers that are
 * implemented within this project. Such as "fork", "thread" and
 * "IO-multiplexing"
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
#ifndef SOCKET_SERVER_H_
#define SOCKET_SERVER_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <poll.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "requestHandler.h"
#include "utilities/configHandler.h"

#define MAX_THREADS 50
#define THREAD_BUCKETS 10

#define INFTIM  -1
#define MAX_OPEN 1024
/*
 * common setup method for all the servers
 */
int setup();
/*
 * Just one process which handles one request at a time
 */
void server_simple();
/*
 * Whenever a request enters, the process forks itself.
 * The child handles the request, while the main process accepts new connections
 */
void server_fork();
/*
 * Whenever a request enters a new thread will be created
 * which handles the request
 */
void server_thread();
/*
 * All the requests are queued into a polling queue.
 * Whenever a one of these connections requires attention it becomes "scheduled"
 * In this way we need only one thread to handle multiple requests without blocking
 */
void server_multi();

#endif /* SOCKET_SERVER_H_ */
