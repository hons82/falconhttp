/* The methods in this file read a request from a Network socket and invokes
 * the analysis and validation of the request.
 * It is responsible to send either the error or the response page to the
 * client
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
#ifndef REQUESTHANDLER_H_
#define REQUESTHANDLER_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h> // for file size
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "utilities/configHandler.h"
#include "utilities/headerHandler.h"
#include "utilities/logger.h"
#include "utilities/errors.h"
#include "utilities/memoryUtils.h"

/*
 * read a request, validate it and prepare the response
 */
int handle(int fdclient, struct sockaddr_in from,
		struct access_log_entry *entry, int *fdlocal);
/*
 * read a page from the local filesystem
 */
int getPage(int fdlocal, char** buf);
/*
 * Send the page to the client
 */
int sendPage(int fdclient, char *buf, struct access_log_entry *entry);
/*
 * Send the error response to the client
 */
void sendErrorResponse(int client, char* code, char* page, char* action);

#endif /* REQUESTHANDLER_H_ */
