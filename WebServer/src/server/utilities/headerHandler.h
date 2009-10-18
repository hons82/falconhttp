/* All the functionality to parse, validate and create the headers
 * either for the request or the response
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
#ifndef HEADERHANDLER_H_
#define HEADERHANDLER_H_

#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include "configHandler.h"
#include "hashmap.h"
#include "stringUtils.h"
#include "dateUtils.h"
#include "logger.h"

#define VERSION		"HTTP/1.0 "
#define BAD_REQ		" Bad Request"
#define FORB		" Forbidden"
#define NOT_FOUND	" Page Not Found"
#define SERV_ERR	" Internal Server Error"
#define NOT_IMPL	" Method Not Implemented"
#define UNAVAIL		" Service Temporarily Unavailable"
#define OK			" OK"
#define SERV_DATE	"\nServer: FALCON_HTTP\nDate: "
#define CONT_TYPE	"\nContent-Type: "
#define CHAR_SET	"; charset=UTF-8"
#define CONT_LEN	"\nContent-Length: "

struct req {
	char* action;
	char* file;
	char* protocol;
	struct hashmap_s* parameter;
};

/*
 * Tokenize the request into the parts needed to validate it and formulize the
 * response.
 */
int reqTokenize(struct req *result, char* request, struct access_log_entry *entry);
/*
 * validate the request string for correctness and to prevent possible attacks
 */
int validate(struct req *request, struct access_log_entry *entry);
/*
 * Creates header with given code, mime for extension, length and places result into dest.
 * (*dest) can be null but must be freed by caller.
 */
void make_header(char* code, char* mime, char* len, char** dest);
/*
 * Fills hashmap with known mime types from mime.type.txt.
 * Returns 1 on success.
 */
int setup_header_handler();
/*
 * Cleanup the remaining memory at shutdown
 */
void removeMimeTypes();

#endif /* HEADERHANDLER_H_ */
