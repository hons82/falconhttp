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
#ifndef CONFIG_H_
#define CONFIG_H_

#include <string.h>
#include "hashmap.h"
#include "stringUtils.h"

#define CONFIGFILE "config/server.conf"
// configfile parameters
#define SERVER_TYPE "executiontype"
#define SERVER_PORT "port"
#define HOME_DIR "home"
#define OUT_VERBOSE "verbose"
#define DAEMON "daemon"
#define JAIL "jail"
#define LOGFILE "logfile"

enum OPTIONS
{
	EXECUTIONTYPE, PORTNR, VERBOSE, LOGFNAME
};
/*
 * Read the configfile into memory and store it into a hashmap
 */
int readConfig();
/*
 * Some parameters can be retrieved by an enum Value
 * (EXECUTIONTYPE, PORTNR, VERBOSE, LOGFNAME)
 */
void* getConfigParameter(int option);
/*
 * All the parameters can be retrieved by their key
 */
void* getConfigParameterKey(const char* key);
/*
 * Add parameters to the parameterstore
 */
void setConfigParameterKey(char* key, char *data);
/*
 * cleanup on shutdown
 */
void removeConfiguration();

#endif /* CONFIG_H_ */
