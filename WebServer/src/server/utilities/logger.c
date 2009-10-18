/* Implementation of the logging facility. This file creates its own Thread
 * handling all the incoming logging requests issued by the server itself.
 * This decouples the logging from the real server functionalities in order
 * to be non blocking whenever logging could take more time. Sll this is
 * secured by Semaphores and Mutexes.
 * It offers error and access logging.
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
#include "logger.h"

enum TYPE {
	LOG, ERR
};
char* levels[8] = { "emerg", "alert", "crit", "error", "warn", "notice",
		"info", "debug" };

#define TOLOG_SIZE 512
struct log_task {
	char to_log[TOLOG_SIZE];
	char *filename;
	enum TYPE type;
	enum SEVERITY severity;
};
pthread_t *logger;

/* id of semaphore*/
#define NUM_SLOTS 128
struct sharedValues {
	sem_t slots_free;
	sem_t slots_used;
	int slot_read;
	int slot_write;
	struct log_task tasks[NUM_SLOTS];
	pthread_mutex_t queue_mutex;
};
struct sharedValues *sv;
int term_logger = 0;

/* Stop the logger */
void stop_logger(int sig_no) {
	if (atoi(getConfigParameter(VERBOSE))) {
		printf("Signal %i received\nStopping the LoggerThread!\n", sig_no);
	}
	term_logger = 1;
}

/*
 * returns 1 on success, 0 otherwise.
 */
//void log_msg(char* to_log, char* filename, enum TYPE type, enum SEVERITY severity) {
void log_msg(struct log_task log) {
	// log to syslog if no filename is provided
	if (log.filename == 0 || log.filename[0] == '\0') {
		char* program = "bth_webserver";
		// option => write to console if problem; facility => generic user level
		if (atoi(getConfigParameterKey(DAEMON))) {
			openlog(program, LOG_CONS, LOG_DAEMON);
		} else {
			openlog(program, LOG_CONS, LOG_USER);
		}
		syslog(log.severity, "%s", log.to_log);
		closelog();
	} else {
		// write to <filename>.err if type denotes an error msg, else log to <filename>.log
		char* file = (char *) safeCalloc(strlen(log.filename) + 4 + 1,
				sizeof(char));
		strcat(file, log.filename);
		(log.type == ERR ? strcat(file, ".err") : strcat(file, ".log"));
		// open and append 1 line with msg to file
		FILE *fp = fopen(file, "a");
		if (fp != 0) {
			fprintf(fp, "%s\n", log.to_log);
			fclose(fp);
		}
		safeFree(file);
		return;
	}
}

/*
 * check the content of string. If empty replace it by "-"
 * returns the length of the string
 */
int prepare_part(char* str) {
	if (str == NULL || str[0] == '\0') {
		return 1;
	} else {
		return strlen(str);
	}
}

/*
 * for each part of the entry, prepare it for logging
 * returns the total length of the entry incl. spaces
 */
int prepare_entry(struct access_log_entry entry) {
	return prepare_part(entry.ip) + prepare_part(entry.ident) + prepare_part(
			entry.auth) + prepare_part(entry.action) + prepare_part(
			entry.request) + prepare_part(entry.code) + prepare_part(
			entry.bytes) + 38; // 28 = date, 2 = " of request, 8 = blanks & string termination
}

/*
 * concatenate the log entrie's parts into dest
 * dest must be large enough and non-NULL
 */
void concatLogEntry(struct access_log_entry entry, char* dest) {
	if (dest == NULL) {
		return;
	}
	memset(dest, 0, sizeof(dest));
	(entry.ip == NULL ? strcat(dest, "-") : strcat(dest, entry.ip));
	strcat(dest, " ");
	(entry.ident == NULL ? strcat(dest, "-") : strcat(dest, entry.ident));
	strcat(dest, " ");
	(entry.auth == NULL ? strcat(dest, "-") : strcat(dest, entry.auth));
	strcat(dest, " ");
	// get current date in the common log file format
	char* current_date = (char*) safeCalloc(29, 1);
	formatLogTime(current_date);
	strcat(dest, current_date);
	safeFree(current_date);
	strcat(dest, " \"");

	(entry.action == NULL ? strcat(dest, "-") : strcat(dest, entry.action));
	strcat(dest, " ");
	(entry.request == NULL ? strcat(dest, "-") : strcat(dest, entry.request));
	strcat(dest, "\" ");
	(entry.code == NULL ? strcat(dest, "-") : strcat(dest, entry.code));
	strcat(dest, " ");
	(entry.bytes == NULL ? strcat(dest, "-") : strcat(dest, entry.bytes));
}

/*
 * truncate the part to_trunc of the log entry if it is longet than max_length
 * max_length is considered to be w/o string termination '\0'
 */
void trunc_part(char* to_trunc, int max_length) {
	if (to_trunc != NULL && strlen(to_trunc) > (max_length)) {
		char* tmp = (char*) safeCalloc(max_length + 1, sizeof(char));
		strncpy(tmp, to_trunc, max_length);
		strcpy(to_trunc, tmp);
		safeFree(tmp);
	}
}

// transforms the entry into an access log message (common log format)
// lets the logging thread log the msg to the file or to syslog if filename is NULL
// severity is used when logging to syslog
void bth_log_access(struct access_log_entry entry, enum SEVERITY severity) {
	char* to_log = NULL;
	int tot_size = prepare_entry(entry);

	//log max. half KB incl. null termination
	if (tot_size > 512) {
		// truncate the entry's parts with variable length to their maximum
		trunc_part(entry.ident, 50);
		trunc_part(entry.auth, 50);
		trunc_part(entry.action, 10);
		trunc_part(entry.request, 335);
		to_log = (char*) safeCalloc(512, sizeof(char));
	} else {
		to_log = (char*) safeCalloc(tot_size, sizeof(char));
	}
	// concatenate the parts into to_log
	concatLogEntry(entry, to_log);
	// log result to file/syslog
	if (atoi(getConfigParameter(VERBOSE))) {
		printf("Adder: Waiting to enter the critical reg\n");
	}
	sem_wait(&sv->slots_free);
	pthread_mutex_lock(&sv->queue_mutex);
	//do logging
	if (atoi(getConfigParameter(VERBOSE))) {
		printf("AdderAcc: logging %i (%s)...\n", sv->slot_write, to_log);
	}
	//sv->tasks[sv->slot_write].to_log = strdup(to_log);
	memset(sv->tasks[sv->slot_write].to_log, 0, TOLOG_SIZE);
	strncpy(sv->tasks[sv->slot_write].to_log, to_log, TOLOG_SIZE - 1);
	sv->tasks[sv->slot_write].filename = getConfigParameter(LOGFNAME);
	sv->tasks[sv->slot_write].type = LOG; // enum
	sv->tasks[sv->slot_write].severity = severity; // enum
	sv->slot_write = (sv->slot_write + 1 < NUM_SLOTS ? sv->slot_write + 1 : 0);
	safeFree(to_log);
	pthread_mutex_unlock(&sv->queue_mutex);
	sem_post(&sv->slots_used);

	if (atoi(getConfigParameter(VERBOSE))) {
		printf("AdderAcc: Done\n");
	}
	return;
}
// lets the logging thread log an error message to the file or to syslog if filename is NULL
// severity is used when logging to syslog
// ip, msg must be non-NULL
void bth_log_error(char* ip, char* msg, enum SEVERITY severity) {
	if (msg == NULL || ip == NULL) {
		return;
	}
	char* to_log = NULL;

	//log max 1/4 KB incl. null termination
	if (strlen(msg) > 199) {
		// truncate the msg to its maximum length
		trunc_part(msg, 199);
		to_log = (char*) safeCalloc(256, sizeof(char));
	} else {
		to_log = (char*) safeCalloc(57 + strlen(msg), sizeof(char));
	}
	// clear and concatenate parts to a common error log msg format
	char* current_date = (char*) safeCalloc(29, 1);
	formatLogTime(current_date);
	strcat(to_log, current_date);
	safeFree(current_date);
	strcat(to_log, " [");
	strcat(to_log, levels[severity]);
	strcat(to_log, "] [");
	strcat(to_log, ip);
	strcat(to_log, "] ");
	strcat(to_log, msg);
	// log result to file/syslog
	sem_wait(&sv->slots_free);
	pthread_mutex_lock(&sv->queue_mutex);
	//do logging
	if (atoi(getConfigParameter(VERBOSE))) {
		printf("AdderErr: logging %i (%s)...\n", sv->slot_write, to_log);
	}
	//sv->tasks[sv->slot_write].to_log = strdup(to_log);
	memset(sv->tasks[sv->slot_write].to_log, 0, TOLOG_SIZE);
	strncpy(sv->tasks[sv->slot_write].to_log, to_log, TOLOG_SIZE - 1);
	sv->tasks[sv->slot_write].filename = getConfigParameter(LOGFNAME);
	sv->tasks[sv->slot_write].type = ERR; // enum
	sv->tasks[sv->slot_write].severity = severity; // enum
	sv->slot_write = (sv->slot_write + 1 < NUM_SLOTS ? sv->slot_write + 1 : 0);
	safeFree(to_log);
	pthread_mutex_unlock(&sv->queue_mutex);
	sem_post(&sv->slots_used);

	if (atoi(getConfigParameter(VERBOSE))) {
		printf("AdderErr: Done\n");
	}
	return;
}

/*
 * wait on logging queue to perform task
 */
void* handleLogging(void *args) {

	while (!term_logger) {
		if (atoi(getConfigParameter(VERBOSE))) {
			printf("Logger: Waiting to enter the critical reg\n");
		}
		sem_wait(&sv->slots_used);
		//do logging
		if (atoi(getConfigParameter(VERBOSE))) {
			printf("Logger: logging %i...\n", sv->slot_read);
		}
		log_msg(sv->tasks[sv->slot_read]);
		memset(&sv->tasks[sv->slot_read], 0, sizeof(sv->tasks[sv->slot_read]));
		sv->slot_read = (sv->slot_read + 1 < NUM_SLOTS ? sv->slot_read + 1 : 0);
		sem_post(&sv->slots_free);
		if (atoi(getConfigParameter(VERBOSE))) {
			printf("Logger: Done\n");
		}
	}
	sem_destroy(&sv->slots_free);
	sem_destroy(&sv->slots_used);
	pthread_mutex_destroy(&sv->queue_mutex);
	shm_unlink("/sharedValues");
	safeFree(logger);
	pthread_exit(NULL);
}

/*
 * creates the logging thread and its semaphore
 * returns 1 on success, 0 otherwise
 */
int setup_logger() {
	int fd;

	// put the synchronization mechanisms into shared memory region
	fd = shm_open("/sharedValues", O_RDWR | O_CREAT, S_IRWXU | S_IRWXG
			| S_IRWXO);
	sv = (struct sharedValues *) mmap(NULL, sizeof(struct sharedValues),
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	// truncate the size of shared memory to the size of shmstruct
	if (ftruncate(fd, sizeof(struct sharedValues)) < 0) {
		printf("Truncate Shared Memory failed");
	}
	close(fd);
	//Initialize the Semaphores
	sem_init(&sv->slots_free, 1, NUM_SLOTS - 1);
	sem_init(&sv->slots_used, 1, 0);
	// initialize mutex for queue
	if (pthread_mutex_init(&sv->queue_mutex, NULL) != 0) {
		if (atoi(getConfigParameter(VERBOSE))) {
			printf("Failure while initializing logging mutex. Exiting...\n");
		}
		sem_destroy(&sv->slots_free);
		sem_destroy(&sv->slots_used);
		exit(-1);
	}
	sv->slot_read = 0;
	sv->slot_write = 0;
	// create the thread and let it work
	logger = safeMalloc(sizeof(pthread_t));
	int rc = pthread_create(logger, NULL, handleLogging, NULL);
	if (rc) {
		if (atoi(getConfigParameter(VERBOSE))) {
			printf("Logger thread faild to start up. Exiting...\n");
		}
		sem_destroy(&sv->slots_free);
		sem_destroy(&sv->slots_used);
		pthread_mutex_destroy(&sv->queue_mutex);
		exit(-1);
	}
	pthread_detach(*logger);
	return 1;
}
