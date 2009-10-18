/* This method can put a process into a jail. Meaning, that its root will
 * be put into the directory specified as "HOME". In this way the process
 * will not be allowed to read files outside the tree below "HOME".
 *
 * This will only work if the process is started as root, or with rights
 * to call chroot.
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
#include "jail.h"

int enJailIt() {
	int dir_fd; /* File descriptor to directory */
	struct stat sbuf; /* The stat() buffer */
	char *home_dir = getConfigParameterKey(HOME_DIR); // retrieve the home_dir

	// retrieve stats from the directory
	if (stat(home_dir, &sbuf) < 0) {
		// check the returnstatus of the stat command
		if (errno == ENOENT) {
			// create the directory
			if (mkdir(home_dir, 0755) < 0) {
				if (atoi(getConfigParameter(VERBOSE))) {
					printf("Failed to create %s - %s\n", home_dir, strerror(
							errno));
				}
				return 0;
			}
		} else {
			if (atoi(getConfigParameter(VERBOSE))) {
				printf("Failed to stat %s - %s\n", home_dir, strerror(errno));
			}
			return 0;
		}
	} else if (!S_ISDIR(sbuf.st_mode)) {
		// file is present, but not a directory
		if (atoi(getConfigParameter(VERBOSE))) {
			fprintf(stderr, "Error - %s is not a directory!\n", home_dir);
		}
		return 0;
	}

	// open the directory for reading
	if ((dir_fd = open(".", O_RDONLY)) < 0) {
		if (atoi(getConfigParameter(VERBOSE))) {
			printf("Failed to open '.' for reading - %s\n", strerror(errno));
		}
		return 0;
	}

	// chroot to the directory
	if (chroot(home_dir) < 0) {
		if (atoi(getConfigParameter(VERBOSE))) {
			printf("Failed to chroot to %s - %s\n", home_dir, strerror(errno));
		}
		return 0;
	}
	// change the userid according to the value in the configfile
	setuid(atoi(getConfigParameterKey(JAIL)));
	// Process is in Jail
	if (atoi(getConfigParameter(VERBOSE))) {
		printf("Process in Jail (Owned by: %i)\n", getuid());
	}
	return 1;
}
