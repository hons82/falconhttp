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
#include "socket_server.h"

#define MAX_PORT_OFFSET 100

int stop_server = 0;

struct multiargs {
	int fdclient;
	int fdlocal;
	struct access_log_entry entry;
};

struct threadargs {
	int pos;
	int fdclient;
	struct sockaddr_in from;
};

hashmap_t threads;
sem_t threadsem;
/*
 * Signal Handler, to stop the running ServerInstances
 */
void sig_usr1(int sig_no) {
	char msg[128];
	sprintf(msg, "Signal %i received - Stopping the Server!", sig_no);
	if (atoi(getConfigParameter(VERBOSE))) {
		printf("%s\n", msg);
	}
	// stop the loggerThread
	stop_logger(sig_no);
	bth_log_error("localhost", msg, INFO);
	stop_server = 1;
}

/*
 * Setup the connection for all the different ServerTypes
 */
int setup() {
	int fd;
	struct sockaddr_in sin;

	//register the signalhandler
	signal(SIGUSR1, sig_usr1);

	/* Allocate a TCP/IP socket. */
	fd = socket(AF_INET, SOCK_STREAM, 0);

	int port = atoi(getConfigParameter(PORTNR));
	if (atoi(getConfigParameter(VERBOSE))) {
		printf("Bind to Port: %i\n", port);
	}

	int ret = -1;
	while (ret < 0) {
		/* Listen for connections*/
		bzero(&sin, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr.s_addr = INADDR_ANY;
		memset(&sin.sin_zero, 0, sizeof(sin.sin_zero));
		if ((ret = bind(fd, (struct sockaddr*) &sin, sizeof(sin))) < 0) {
			char msg[128];
			sprintf(msg, "Bind error (Port: %i)", port);
			bth_log_error("localhost", msg, CRITICAL);
			if (port > MAX_PORT_OFFSET + atoi(getConfigParameter(PORTNR))) {
				exit(-1);
			}
			port++;
		}
	}
	// create a queue for incomming connections on fd of length xx
	listen(fd, 64);

	return (fd);
}

void cleanUp(struct access_log_entry *entry) {
	safeFree(entry->ip);
	safeFree(entry->action);
	safeFree(entry->auth);
	safeFree(entry->bytes);
	safeFree(entry->code);
	safeFree(entry->ident);
	safeFree(entry->request);
}

void* controlThreadedHandle(void *args) {
	int fdclient, pos;
	char posc[16];
	int fdlocal; /* file descriptor for the local file */
	struct sockaddr_in from;
	struct access_log_entry entry;

	fdclient = ((struct threadargs *) args)->fdclient;
	pos = ((struct threadargs *) args)->pos;
	from = ((struct threadargs *) args)->from;
	if (atoi(getConfigParameter(VERBOSE))) {
		printf("Thread %i starts handling! \n", pos);
	}
	/* Perform the client’s request */
	if (handle(fdclient, from, &entry, &fdlocal) > 0) {
		if (strcmp(entry.action, "GET") == 0) {
			char *buf = NULL;
			while (getPage(fdlocal, &buf) >= 0) {
				if (sendPage(fdclient, buf, &entry) <= 0) {
					if (atoi(getConfigParameter(VERBOSE))) {
						printf("Error While sending Page \n");
					}
					break;
				}
				safeFree(buf);
			}
		}
		bth_log_access(entry, WARNING);
		close(fdlocal);
	}
	cleanUp(&entry);
	/* Clean up the open filehandlers */
	shutdown(fdclient, SHUT_RDWR);
	close(fdclient);
	safeFree(args);
	/*
	 * remove the Threaddata from the Hashmap
	 * Note: This does already the "free" of it
	 */
	sprintf(posc, "%15d", pos);
	if (!hashmap_remove(threads, posc) > 0) {
		// No elements deleted (This should never happen)
		char msg[128];
		sprintf(msg, "Thread %i not present in the Hashmap", pos);
		bth_log_error(inet_ntoa(from.sin_addr), msg, WARNING);
	}
	if (atoi(getConfigParameter(VERBOSE))) {
		printf("Thread %i Finished! \n", pos);
	}
	/* Increase the Semaphore */
	sem_post(&threadsem);
	/* ---------------------- */
	pthread_exit(NULL);
}

void server_thread() {
	int fd; /* file descriptor */
	int fdclient; /* file descriptor for client connection */
	socklen_t addrlen;
	struct sockaddr_in from;
	pthread_t *threaddata;
	int threadpos = 0;
	char posc[16];
	struct threadargs *args;

	sem_init(&threadsem, 0, MAX_THREADS);

	if (atoi(getConfigParameter(VERBOSE))) {
		printf("Starting Server...\n");
	}

	fd = setup();
	threads = hashmap_create(THREAD_BUCKETS);

	while (!stop_server) {
		if (atoi(getConfigParameter(VERBOSE))) {
			printf("Listening...\n");
		}
		/* Wait for a new connection from a client. */
		addrlen = sizeof(from);
		if ((fdclient = accept(fd, (struct sockaddr*) &from, &addrlen)) < 0) {
			bth_log_error("localhost", "Could not accept Connection", CRITICAL);
			continue;
		}
		if (atoi(getConfigParameter(VERBOSE))) {
			printf("Threading...\n");
		}
		/* Is there a free slot in the Hashmap? Decrease the Semaphore */
		sem_wait(&threadsem);
		/* Look for it */
		sprintf(posc, "%15d", threadpos);
		while (hashmap_search(threads, posc) > 0) {
			threadpos = (threadpos + 1 < MAX_THREADS ? threadpos + 1 : 0);
			sprintf(posc, "%15d", threadpos);
		}
		/* Prepare the arguments for the ThreadedHandling */
		args = safeMalloc(sizeof(struct threadargs));
		args->pos = threadpos;
		args->fdclient = fdclient;
		args->from = from;
		/* store the threaddata in the Hashmap */
		threaddata = safeMalloc(sizeof(pthread_t));
		hashmap_insert(threads, posc, threaddata, sizeof(threaddata));
		/* start the Thread */
		if (pthread_create(threaddata, NULL, controlThreadedHandle,
				(void*) args)) {
			bth_log_error(inet_ntoa(from.sin_addr),
					"Creation of Thread failed", ERROR);
			continue;
		}
		if (atoi(getConfigParameter(VERBOSE))) {
			printf("Detaching Thread...\n");
		}
		// detach the Thread, so it does not need to join with other Threads
		pthread_detach(*threaddata);
		// hashmap makes its own copy, so not needed anymore
		safeFree(threaddata);
		// reset the threadpos
		threadpos = (threadpos + 1 < MAX_THREADS ? threadpos + 1 : 0);
	}
	hashmap_delete(threads);
	close(fd);
	return;
}

int pagesend_multi(struct multiargs *args) {
	char *buf = NULL;
	// set the filedescriptor NONBLOCKING
	int flags = fcntl(args->fdlocal, F_GETFL, 0);
	fcntl(args->fdlocal, F_SETFL, flags | O_NONBLOCK);
	//nonblocking read from file
	getPage(args->fdlocal, &buf);
	//still blocking send to socket
	if (sendPage(args->fdclient, buf, &args->entry) <= 0) {
		if (atoi(getConfigParameter(VERBOSE))) {
			printf("Error While sending Page \n");
		}
	}
	safeFree(buf);
	return errno;
}

void server_multi() {
	int i, maxi, fd, fdclient;
	int nready;
	int fdlocal; /* file descriptor for the local file */
	socklen_t addrlen;
	struct sockaddr_in from;
	struct access_log_entry entry;
	struct pollfd client[MAX_OPEN];
	hashmap_t combine;

	if (atoi(getConfigParameter(VERBOSE))) {
		printf("%s \n", "Starting Server...");
	}

	fd = setup();

	combine = hashmap_create(10);

	// prepare client-structure
	client[0].fd = fd;
	client[0].events = POLLIN;
	for (i = 1; i < MAX_OPEN; i++) {
		client[i].fd = -1; /* -1 indicates available entry */
	}
	maxi = 0; /* max index into client[] array */
	while (!stop_server) {
		if (atoi(getConfigParameter(VERBOSE))) {
			printf("Initiate Polling \n");
		}
		nready = poll(client, maxi + 1, INFTIM);
		if (client[0].revents & POLLIN) { /* new client connection */
			addrlen = sizeof(from);
			if ((fdclient = accept(fd, (struct sockaddr*) &from, &addrlen)) < 0) {
				bth_log_error("localhost", "Could not accept Connection",
						CRITICAL);
				continue;
			}
			if (atoi(getConfigParameter(VERBOSE))) {
				printf("new client...\n");
			}
			for (i = 1; i < MAX_OPEN; i++) {
				if (client[i].fd < 0) {
					client[i].fd = fdclient; /* save descriptor */
					client[i].events = POLLIN;
					/*Update the maxindex*/
					maxi = (i > maxi ? i : maxi);
					break;
				}
			}
			if (i == MAX_OPEN) {
				//err_quit("too many clients");
				bth_log_error(inet_ntoa(from.sin_addr),
						"IO-Multiplexing - Too Many Clients", ERROR);
				shutdown(fdclient, SHUT_RDWR);
				close(fdclient);
			}
			if (--nready <= 0) {
				continue; /* no more readable descriptors */
			}
		}
		for (i = 1; i <= maxi; i++) { /* check all clients for data */
			if ((fdclient = client[i].fd) < 0) {
				continue;
			}
			if (client[i].revents & (POLLIN | POLLERR)) {
				struct multiargs *args = NULL;
				char key[16];
				snprintf(key, 15, "%i", fdclient);
				if (hashmap_entry_by_key(combine, key, (void **) &args) <= 0) {
					// not yet in the hashmap
					if (handle(fdclient, from, &entry, &fdlocal) > 0) {
						args = safeMalloc(sizeof(struct multiargs));
						//-- to show, that file read is non blocking, we read from console
						//fdlocal = 0; /* test*/
						//----------------------------------------------------------------
						args->fdlocal = fdlocal;
						args->fdclient = fdclient;
						args->entry = entry;
						snprintf(key, 15, "%i", fdlocal);
						hashmap_insert(combine, key, args,
								sizeof(struct multiargs));
						if (strcmp(entry.action, "GET") == 0) {
							if (pagesend_multi(args) == EWOULDBLOCK) {
								client[i].fd = fdlocal; /* save descriptor */
								client[i].events = POLLIN;
							}
						}
						// hasmap makes its own copy --> so free it and...
						safeFree(args);
						// ...use the reference from the hashmap
						hashmap_entry_by_key(combine, key, (void **) &args);
					}
				} else {
					if (atoi(getConfigParameter(VERBOSE))) {
						printf("Resending after EWOULDBLOCK\n");
					}
					pagesend_multi(args);
				}
				// if EOF
				if (errno != EWOULDBLOCK) {
					if (args != NULL) {
						bth_log_access(args->entry, WARNING);
						close(args->fdlocal);
						shutdown(args->fdclient, SHUT_RDWR);
						close(args->fdclient);
						cleanUp(&args->entry);
						hashmap_remove(combine, key);
					} else {
						cleanUp(&entry);
						shutdown(fdclient, SHUT_RDWR);
						close(fdclient);
					}
					client[i].fd = -1;
					if (--nready <= 0) {
						break; /* no more readable descriptors */
					}
				}
			}
		}
	}
	// cleanup
	hashmap_delete(combine);
	// close remaining file descriptors
	for (i = 1; i < MAX_OPEN; i++) {
		if (client[i].fd >= 0) {
			shutdown(client[i].fd, SHUT_RDWR);
			close(client[i].fd);
		}
	}
	return;
}

void server_fork() {
	int fd; /* file descriptor */
	int fdclient; /* file descriptor for client connection */
	int status;
	int fdlocal; /* file descriptor for the local file */
	socklen_t addrlen;
	struct sockaddr_in from;
	struct access_log_entry entry;

	if (atoi(getConfigParameter(VERBOSE))) {
		printf("%s \n", "Starting Server...");
	}

	fd = setup();

	while (!stop_server) {
		if (atoi(getConfigParameter(VERBOSE))) {
			printf("Listening...\n");
		}
		/* Wait for a new connection from a client. */
		addrlen = sizeof(from);
		if ((fdclient = accept(fd, (struct sockaddr*) &from, &addrlen)) < 0) {
			bth_log_error("localhost", "Could not accept Connection", CRITICAL);
			continue;
		}

		if (atoi(getConfigParameter(VERBOSE))) {
			printf("Forking...\n");
		}
		/* Create a new child process. */
		if (fork() == 0) {
			/* Perform the client’s request in the child process. */
			if (handle(fdclient, from, &entry, &fdlocal) > 0) {
				if (strcmp(entry.action, "GET") == 0) {
					char *buf = NULL;
					while (getPage(fdlocal, &buf) >= 0) {
						if (sendPage(fdclient, buf, &entry) <= 0) {
							if (atoi(getConfigParameter(VERBOSE))) {
								printf("Error While sending Page \n");
							}
							break;
						}
						safeFree(buf);
					}
				}
				bth_log_access(entry, WARNING);
				close(fdlocal);
			}
			cleanUp(&entry);
			shutdown(fdclient, SHUT_RDWR);
			close(fdclient);
			exit(0);
		}
		// parent doesn't need this
		close(fdclient);

		if (atoi(getConfigParameter(VERBOSE))) {
			printf("Filedescriptor closed...\n");
		}

		/* Collect dead children, but don’t wait for them. */
		waitpid(-1, &status, WNOHANG);
	}
	close(fd);
	return;
}

void server_simple() {

	int fd; /* file descriptor */
	int fdclient; /* file descriptor for client connection */
	int fdlocal; /* file descriptor for the local file */
	socklen_t addrlen;
	struct sockaddr_in from;
	struct access_log_entry entry;

	if (atoi(getConfigParameter(VERBOSE))) {
		printf("%s \n", "Starting Server...");
	}

	fd = setup();

	while (!stop_server) {
		if (atoi(getConfigParameter(VERBOSE))) {
			printf("Listening...\n");
		}
		addrlen = sizeof(from);
		if ((fdclient = accept(fd, (struct sockaddr*) &from, &addrlen)) < 0) {
			bth_log_error("localhost", "Could not accept Connection", CRITICAL);
			continue;
		}

		if (atoi(getConfigParameter(VERBOSE))) {
			printf("%s \n", "Accepted...");
		}

		if (handle(fdclient, from, &entry, &fdlocal) > 0) {
			if (strcmp(entry.action, "GET") == 0) {
				char* buf = NULL;
				while (getPage(fdlocal, &buf) >= 0) {
					if (sendPage(fdclient, buf, &entry) <= 0) {
						if (atoi(getConfigParameter(VERBOSE))) {
							printf("Error While sending Page \n");
						}
						break;
					}
					safeFree(buf);
				}
			}
			bth_log_access(entry, WARNING);
			close(fdlocal);
		}
		cleanUp(&entry);
		shutdown(fdclient, SHUT_RDWR);
		close(fdclient);
	}
	close(fd);
	return;
}
