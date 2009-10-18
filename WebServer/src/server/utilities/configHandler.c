/* Handles all the operations needed to read, store and retrieve the
 * parameters specified in the config file
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
#include "configHandler.h"

hashmap_t configparameter;

void removeConfiguration() {
	// free the memory occupied by the hashmap
	hashmap_delete(configparameter);
}

void setDefaults() {
	// Add default values to the configstore
	hashmap_insert(configparameter, SERVER_TYPE, "simple", 7);
	hashmap_insert(configparameter, SERVER_PORT, "33333", 6);
	hashmap_insert(configparameter, OUT_VERBOSE, "0", 2);
	hashmap_insert(configparameter, DAEMON, "0", 2);
	hashmap_insert(configparameter, JAIL, "0", 2);
	hashmap_insert(configparameter, HOME_DIR, "/", 2);
}

void* getConfigParameterKey(const char* key) {
	char* data;
	int rc;
	// Query the parameter store
	if ((rc = hashmap_entry_by_key(configparameter, key, (void **) &data)) > 0) {
		return data;
	} else {
		return NULL;
	}
}

void* getConfigParameter(int option) {
	if (option == EXECUTIONTYPE) {
		return getConfigParameterKey(SERVER_TYPE);
	} else if (option == PORTNR) {
		return getConfigParameterKey(SERVER_PORT);
	} else if (option == VERBOSE) {
		return getConfigParameterKey(OUT_VERBOSE);
	} else if (option == LOGFNAME) {
		return getConfigParameterKey(LOGFILE);
	}
	return NULL;
}

void setConfigParameterKey(char* key, char *data) {
	strtrim(key);
	strtrim(data);
	// delete all entries with that key (if there)
	hashmap_remove(configparameter, key);
	// insert the new entry
	hashmap_insert(configparameter, key, data, strlen(data) + 1);
}

int readConfig() {
	FILE* fsconfig;
	char buf[1024], key[50], data[256];
	char* p;

	// allocate Parameter Hashmap
	configparameter = hashmap_create(5);
	if (configparameter == NULL) {
		return -1;
	}
	// set default Values
	setDefaults();

	fsconfig = fopen(CONFIGFILE, "r");
	if (fsconfig == 0) {
		printf("ConfigFile not Found!\n");
		exit(-1);
	} else {
		// parse the config file
		while (fgets(buf, sizeof(buf), fsconfig) != NULL) {
			// skip comments
			if ((p = strstr(buf, "#"))) {
				buf[p - buf] = '\0';
			}
			// get all parameters of the form "key=value"
			if (strlen(buf) > 0 && strstr(buf, "=") != NULL) {
				sscanf(buf, "%50[^=]=%250s", key, data);
				if (strlen(key) > 0 && strlen(data) > 0) {
					setConfigParameterKey(key, data);
				}
			}
		}
		fclose(fsconfig);
	}
	return 0;
}
