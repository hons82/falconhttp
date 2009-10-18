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
#include "headerHandler.h"

#define MIMEFILE "config/mime.types.txt"

hashmap_t known_mimes;

int reqTokenize(struct req *result, char* request,
		struct access_log_entry *entry) {
	char action[32], file[1024], protocol[1024];
	char key[64], data[1024];

	// clean the request
	strreplall(request, "\r", "\n");
	strreplall(request, "\t", "\n");

	// get the first line (Special --> is the request itself)
	char* line = strtok(request, "\n");
	if (line == NULL || strlen(line) == 0) {
		entry->code = strdup("400"); // bad request
		bth_log_access(*entry, ERROR);
		return 0;
	}
	memset(action, 0, sizeof(action));
	memset(file, 0, sizeof(file));
	memset(protocol, 0, sizeof(protocol));
	sscanf(line, "%31s %1023s %1023s", action, file, protocol);
	result->action = strdup(action);
	result->file = strdup(file);
	result->protocol = strdup(protocol);

	memset(line, 0, sizeof(line));
	line = strtok(NULL, "\n");

	// parse the rest of the Parameters
	hashmap_t hm = hashmap_create(5);

	while (line != NULL) {
		// if in the form xxx:yyyyy
		if (strstr(line, ":") != NULL) {
			memset(key, 0, sizeof(key));
			memset(data, 0, sizeof(data));
			sscanf(line, "%63[^:]:%1023s", key, data);
			strtrim(key);
			strtrim(data);
			hashmap_insert(hm, key, data, strlen(data));
		}
		memset(line, 0, sizeof(line));
		line = strtok(NULL, "\n");
	}

	result->parameter = hm;

	if (atoi(getConfigParameter(VERBOSE))) {
		printf("(%s(%i)%s(%i)%s(%i))\n", result->action,
				strlen(result->action), result->file, strlen(result->file),
				result->protocol, strlen(result->protocol));
	}

	return 1;
}

int validate(struct req *request, struct access_log_entry *entry) {
	char tmp[2048];
	char *resolvedPath = NULL;
	char *p = NULL;

	// save the original request for logging
	entry->request = strdup(request->file);
	entry->action = strdup(request->action);
	// Check the filename
	if (request->file == NULL) {
		entry->code = strdup("400"); // bad request
		bth_log_access(*entry, ERROR);
		return 0;
	}

	if (strcmp(request->action, "GET") != 0 && strcmp(request->action, "HEAD")
			!= 0) {
		// if not HEAD and not GET
		entry->code = (strcmp(request->action, "PUT") != 0 && strcmp(
				request->action, "POST") != 0 && strcmp(request->action,
				"DELETE") != 0 ? strdup("400") : strdup("501"));
		bth_log_access(*entry, ERROR);
		return 0;
	}
	// create absolute path: home + "/" + request->file
	memset(&tmp, 0, sizeof(tmp));
	if (!atoi(getConfigParameterKey(JAIL))) {
		strcat(tmp, (char *) getConfigParameterKey("home"));
	}
	strcat(tmp, "/");
	strcat(tmp, request->file);
	// if incoming filename already started with a "/"
	strreplall(tmp, "//", "/");

	// check if the path is valid
	if ((resolvedPath = realpath(tmp, NULL)) == NULL) {
		entry->code = strdup("404"); // page not found on this path
		bth_log_access(*entry, ERROR);
		return 0;
	}

	// if the resolved path does not contain the home directory anymore
	// or if the homedir is in the path, it is not at the beginning of the string
	// (if it is at the beginning: home == p -> true)
	if (!atoi(getConfigParameterKey(JAIL)) && (!(p = strstr(resolvedPath,
			(char *) getConfigParameterKey("home"))) || strcmp(resolvedPath, p)
			!= 0)) {
		entry->code = strdup("403"); // forbidden request
		bth_log_access(*entry, ERROR);
		// log possible attack
		bth_log_error(entry->ip,
				"Requested File not under homeDir. Possible Attack", INFO);
		safeFree(resolvedPath);
		return 0;
	}
	safeFree(request->file);
	request->file = strdup(resolvedPath);
	safeFree(resolvedPath);
	if (atoi(getConfigParameter(VERBOSE))) {
		printf("resolved: <%s>\n", request->file);
	}
	// everything seems to be fine, so proceed
	return 1;
}

/*
 * Creates header with given code, mime for extension, length and places result into dest.
 * (*dest) can be null but must be freed by caller.
 */
void make_header(char* code, char* extension, char* len, char** dest) {
	char tmp[1024];
	memset(&tmp, 0, sizeof(tmp));

	// concatenate the header fields and their values
	strcat(tmp, VERSION);
	strcat(tmp, code);
	int c = atoi(code);
	switch (c) {
	case 200:
		strcat(tmp, OK);
		break;
	case 400:
		strcat(tmp, BAD_REQ);
		break;
	case 403:
		strcat(tmp, FORB);
		break;
	case 404:
		strcat(tmp, NOT_FOUND);
		break;
	case 500:
		strcat(tmp, SERV_ERR);
		break;
	case 501:
		strcat(tmp, NOT_IMPL);
		break;
	case 505:
		strcat(tmp, UNAVAIL);
		break;
	}
	strcat(tmp, SERV_DATE);
	char* date = (char*) safeCalloc(32, sizeof(char));
	formatHeaderTime(date);
	strcat(tmp, date);
	safeFree(date);
	// add content type and it's encoding
	// only if extension given & mime known
	if (extension != NULL && strlen(extension) > 0) {
		char* mime;
		if (hashmap_entry_by_key(known_mimes, extension, (void **) &mime) > 0) {
			strcat(tmp, CONT_TYPE);
			strcat(tmp, mime);
			strcat(tmp, CHAR_SET);
		}
	}
	// add content length, if known
	if (len != NULL && strlen(len) > 0) {
		strcat(tmp, CONT_LEN);
		strcat(tmp, len);
	}
	strcat(tmp, "\n");
	safeFree(*dest);
	*dest = strdup(tmp);
}

void removeMimeTypes() {
	hashmap_delete(known_mimes);
}

/*
 * Fills hashmap with known mime types from mime.type.txt.
 * Returns 1 on success.
 */
int setup_header_handler() {
	if (known_mimes != NULL) {
		return 1;
	}
	// prepare hashmap and mime.type.txt file for reading
	known_mimes = hashmap_create(50);
	FILE* mfile = fopen(MIMEFILE, "r");
	if (mfile == 0) {
		bth_log_error("localhost", "MimeFile not Found", ERROR);
		return 0;
	}
	// for each line, get mime type and its extensions (if line not commented or empty)
	char buf[1024], type[128], ext[128];
	memset(buf, 0, 1024);
	memset(type, 0, 128);
	memset(ext, 0, 128);
	while (fgets(buf, sizeof(buf), mfile) != NULL) {
		if (buf[0] != '#' && buf[0] != '\n') {
			sscanf(buf, "%s %127c", type, ext);
			// if mime type has extensions, add each to hashmap as key
			// value for all extensions is the current mime type
			if (ext[0] != 0) {
				char key[32], tmp[128];
				do {
					memset(key, 0, 32);
					memset(tmp, 0, 128);
					sscanf(ext, "%s %127c", key, tmp);
					hashmap_remove(known_mimes, key);
					hashmap_insert(known_mimes, key, type, strlen(type) + 1);
					strcpy(ext, tmp);
				} while (tmp[0] != 0);
			}
			memset(type, 0, 128);
			memset(ext, 0, 128);
		}
	}
	fclose(mfile);
	return 1;
}
