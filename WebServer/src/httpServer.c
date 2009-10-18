/* Main entry point into the server
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
#include <string.h>
#include <regex.h>
#include "server/socket_server.h"
#include "server/jail.h"
#include "server/daemon.h"
#include "server/utilities/configHandler.h"
#include "server/utilities/dateUtils.h"
#include "server/utilities/headerHandler.h"
#include "server/utilities/logger.h"

// VARIABLES:
char
		* usage =
				"USAGE: bth_webserver [ -p P ] [ -d ] [-l <filename>]\n\t where\n\t\t-p ...   use P as new port\n\t\t-d ...   run as daemon\n\t\t-l ...   use <filename> as log file";

/*
 * Parsing the commandline options
 */
void parseCommandLine(int count, char* args[]) {
	int i;
	for (i = 1; i < count; i++) {

		// Is there a port specified?
		if (strcmp(args[i], "-p") == 0) {
			if (args[i + 1] != NULL) {
				char* end;
				int port = strtol(args[i + 1], &end, 10);
				if (*end) {
					printf("%s\n  %s\n",
							"Error: -p wants an integer as argument.", usage);
					exit(1);
				}
				sprintf(end, "%i", port);
				setConfigParameterKey(SERVER_PORT, end);
			} else {
				printf("%s\n  %s\n", "Error: -p wants an integer as argument.",
						usage);
				exit(1);
			}
		}
		// should the Server run as a Daemon-process?
		if (strcmp(args[i], "-d") == 0) {
			setConfigParameterKey(DAEMON, "1");
			setConfigParameterKey(OUT_VERBOSE, "0");
		}
		// logfile-prefix (if specified the logging will be to a file istead of the syslog)
		if (strcmp(args[i], "-l") == 0) {
			if (args[i + 1] != NULL) {
				char* file = args[i + 1];
				setConfigParameterKey(LOGFILE, file);
			} else {
				printf("%s\n  %s\n", "Error: -l wants a filename as argument.",
						usage);
				exit(1);
			}
		}
	}
}
/*
 * This is the main entry point to the Server.
 * According to the parameters specified in config file, the server will be instanciated
 */
int main(int argc, char * argv[]) {
	// parse the configFile
	readConfig();
	// parse command line arguments
	parseCommandLine(argc, argv);
	// Daemon?
	if (atoi(getConfigParameterKey(DAEMON))) {
		daemonizeIt(NULL);
	}
	// start the logging facility
	setup_logger();
	// should it run in the Jail?
	// set up known mime types for headers
	setup_header_handler();
	if (atoi(getConfigParameterKey(JAIL))) {
		// so put the process into a jail
		if (!enJailIt()) {
			if (atoi(getConfigParameter(VERBOSE))) {
				printf("Failed to put in jail the process.\nServer Stopped\n");
			}
			exit(-1);
		}
	}

	if (atoi(getConfigParameter(VERBOSE))) {
		printf("Configuration Values\n====================\n");
		printf("type: %s\n", (char *) getConfigParameter(EXECUTIONTYPE));
		printf("port: %i\n", atoi(getConfigParameter(PORTNR)));
		printf("verbose: %s\n", atoi(getConfigParameter(VERBOSE)) ? "true"
				: "false");
		printf("====================\n");
	}

	if (strcasecmp((char*) getConfigParameter(EXECUTIONTYPE), "multi") == 0) {
		server_multi();
	} else if (strcasecmp((char*) getConfigParameter(EXECUTIONTYPE), "thread")
			== 0) {
		server_thread();
	} else if (strcasecmp((char*) getConfigParameter(EXECUTIONTYPE), "fork")
			== 0) {
		server_fork();
	} else {
		server_simple();
	}
	removeMimeTypes();
	removeConfiguration();
	exit(0);

}
