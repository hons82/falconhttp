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
#include "requestHandler.h"

int handle(int fdclient, struct sockaddr_in from,
		struct access_log_entry *entry, int *fdlocal) {
	char request[1024], buf[8192];
	struct req tokens;
	int ret = 1;

	memset(entry, 0, sizeof(struct access_log_entry));
	entry->ip = strdup(inet_ntoa(from.sin_addr));

	if (atoi(getConfigParameter(VERBOSE))) {
		printf("Handling started...\n");
	}

	memset(request, 0, sizeof(request));
	memset(buf, 0, sizeof(buf));
	/* Read the request (a file name) from the client. */
	if (recv(fdclient, request, sizeof(request)-1, 0) == -1) {
		bth_log_error(inet_ntoa(from.sin_addr), "Error in receiving request",
				ERROR);
		return (-1);
	}

	if (atoi(getConfigParameter(VERBOSE))) {
		printf("Request received...\n");
	}

	if (reqTokenize(&tokens, request, entry) && validate(&tokens, entry)) {
		if (atoi(getConfigParameter(VERBOSE))) {
			printf("Open File: <%s>\n", tokens.file);
		}

		/* Open the file, send the headers to the client. */
		*fdlocal = open(tokens.file, O_RDONLY);
		if (*fdlocal == 0) {
			entry->code = strdup("500"); // internal server error (file = validated but not readable)
			bth_log_access(*entry, CRITICAL);
			char msg[1024];
			sprintf(msg, "Could not open file for validated request: %s",
					tokens.file);
			bth_log_error(inet_ntoa(from.sin_addr), msg, CRITICAL);
			sendErrorResponse(fdclient, "500", entry->request, entry->action);
			if (atoi(getConfigParameter(VERBOSE))) {
				printf("File not readable!\n");
			}
			// signal error to caller
			ret = -1;
		} else {
			// get content length
			char length[16];
			char* header = NULL;
			struct stat file_stat;
			memset(length, 0, 16);
			if (fstat(*fdlocal, &file_stat) < 0) {
				char msg[1024];
				sprintf(msg, "Could not get file stats %25s", tokens.file);
				bth_log_error(inet_ntoa(from.sin_addr), msg, WARNING);
				// signal error to caller
				//ret = -1;
			} else {
				int l = file_stat.st_size;
				snprintf(length, 15, "%i", l);
				entry->bytes = strdup(length);
			}
			// get the file extension
			char* extension = NULL;
			char* save_ptr = NULL;
			char* tmp = strtok_r(tokens.file, ".", &save_ptr);
			while (tmp != NULL) {
				extension = tmp;
				tmp = strtok_r(NULL, ".", &save_ptr);
			}
			// prepare header to send
			make_header("200", extension, length, &header);
			entry->code = strdup("200");

			// send header
			if (send(fdclient, header, strlen(header), 0) == -1) {
				bth_log_error(inet_ntoa(from.sin_addr),
						"Could not send header", CRITICAL);
				safeFree(entry->code);
				entry->code = strdup("500");
				// signal error to caller
				ret = -1;
			}
			safeFree(header);
		}
	} else {
		sendErrorResponse(fdclient, entry->code, entry->request, entry->action);
		// signal error to caller
		ret = -1;
	}
	safeFree(tokens.action);
	safeFree(tokens.file);
	safeFree(tokens.protocol);
	//safeFree(tokens.parameter);

	if (tokens.parameter != NULL) {
		hashmap_delete(tokens.parameter);
		//saveFree(tokens.parameter);
	}
	return ret;
}

int getPage(int fdlocal, char** buf) {
	char tmp[8192];
	memset(&tmp,0,sizeof(tmp));
	if (read(fdlocal, tmp, sizeof(tmp)-1) <= 0) {
		if (errno == EWOULDBLOCK) {
			if (atoi(getConfigParameter(VERBOSE))) {
				printf("Errno = EWOULDBLOCK\n");
			}
			return 0;
		}
		return -1;
	}
	*buf = strdup(tmp);
	return 1;

}

int sendPage(int fdclient, char *buf, struct access_log_entry *entry) {
	if (buf != NULL) {
		if (send(fdclient, buf, strlen(buf), 0) == -1) {
			bth_log_error(entry->ip, "Could not send content", CRITICAL);
			bth_log_access(*entry, ALERT);
			return (-1);
		}
	}
	return 1;
}

void sendErrorResponse(int client, char* code, char* page, char* action) {
	char* header = NULL;
	char body[1024];
	char length[4];
	memset(body, 0, sizeof(body));
	memset(length, 0, sizeof(length));
	// html for 400, 403, 404, 500, 501, 503
	int c = atoi(code);
	switch (c) {
	case 400: {
		(page == NULL ? sprintf(length, "%i", 382) : sprintf(length, "%i", 382
				+ strlen(page)));
		make_header("400", "html", length, &header);
		strcat(body, E_400_1);
		strcat(body, page);
		strcat(body, E_400_2);
		break;
	}
	case 403: {
		(page == NULL ? sprintf(length, "%i", 273) : sprintf(length, "%i", 273
				+ strlen(page)));
		make_header("403", "html", length, &header);
		strcat(body, E_403_1);
		strcat(body, page);
		strcat(body, E_403_2);
		break;
	}
	case 404: {
		(page == NULL ? sprintf(length, "%i", 359) : sprintf(length, "%i", 359
				+ strlen(page)));
		make_header("404", "html", length, &header);
		strcat(body, E_404_1);
		strcat(body, page);
		strcat(body, E_404_2);
		break;
	}
	case 500: {
		(page == NULL ? sprintf(length, "%i", 363) : sprintf(length, "%i", 363
				+ strlen(page)));
		make_header("500", "html", length, &header);
		strcat(body, E_500_1);
		strcat(body, page);
		strcat(body, E_500_2);
		break;
	}
	case 501: {
		(page == NULL ? sprintf(length, "%i", 304) : sprintf(length, "%i", 304
				+ strlen(page)));
		make_header("501", "html", length, &header);
		strcat(body, E_501_1);
		strcat(body, page);
		strcat(body, E_501_2);
		break;
	}
	case 503: {
		make_header("503", "html", "384", &header);
		strcat(body, E_503);
		break;
	}
	}
	// send header
	if (send(client, header, strlen(header), 0) == -1) {
		bth_log_error("localhost", "Could not send error-response header",
				CRITICAL);
	}
	safeFree(header);
	// send body if "GET"
	if (strcmp(action, "GET") == 0) {
		if (send(client, body, strlen(body), 0) == -1) {
			bth_log_error("localhost", "Could not send error-response body",
					CRITICAL);
		}
	}
	return;
}
