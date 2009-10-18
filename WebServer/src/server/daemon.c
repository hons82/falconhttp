/* This method can convert a normal process into a Daemon process
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
#include "daemon.h"

void daemonizeIt(const char *cmd) {
	int i, fd0, fd1, fd2;
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;

	// clear creation mask
	umask(0);
	// get maximum filedescriptors
	if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
		//TODO: Error, can't get file limit
	}
	//become session leader & loose the controlling terminal
	if ((pid = fork()) < 0) {
		//TODO: Error, can't fork
	} else if (pid != 0) {
		//Parent
		exit(0);
	}
	// Create a new Session
	setsid();
	// Ensure future opens won't allocate controlling TTYs
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) < 0) {
		// TODO: can't ignore SIGHUP
	}
	if ((pid = fork()) < 0) {
		// TODO: can't fork
	} else if (pid != 0) {
		//Parent
		exit(0);
	}
	// change current working directory to "/"
	// to not prevent filesystems from beeing unmounted
	if (chdir("/") < 0) {
		// TODO: Error, can't change directory to "/"
	}
	//Close all open Filedescriptors
	if (rl.rlim_max == RLIM_INFINITY) {
		rl.rlim_max = 1024;
	}
	for (i = 0; i < rl.rlim_max; i++) {
		close(i);
	}
	//Attach Filedescriptors 0, 1, 2 to /dev/null
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);
}
