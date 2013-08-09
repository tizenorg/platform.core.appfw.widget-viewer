/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include <dlog.h>

#include "conf.h"
#include "debug.h"
#include "util.h"
#include "critical_log.h"
#include "livebox-errno.h" /* For error code */

static struct {
	FILE *fp;
	int file_id;
	int nr_of_lines;
	char *filename;
} s_info = {
	.fp = NULL,
	.file_id = 0,
	.nr_of_lines = 0,
	.filename = NULL,
};



int critical_log(const char *func, int line, const char *fmt, ...)
{
	va_list ap;
	int ret;
	struct timeval tv;

	if (!s_info.fp) {
		return LB_STATUS_ERROR_IO;
	}

	gettimeofday(&tv, NULL);
	fprintf(s_info.fp, "%d %lu.%lu [%s:%d] ", getpid(), tv.tv_sec, tv.tv_usec, util_basename((char *)func), line);

	va_start(ap, fmt);
	ret = vfprintf(s_info.fp, fmt, ap);
	va_end(ap);

	s_info.nr_of_lines++;
	if (s_info.nr_of_lines == MAX_LOG_LINE) {
		char *filename;
		int namelen;

		s_info.file_id = (s_info.file_id + 1) % MAX_LOG_FILE;

		namelen = strlen(s_info.filename) + strlen(SLAVE_LOG_PATH) + 20;
		filename = malloc(namelen);
		if (filename) {
			snprintf(filename, namelen, "%s/%d_%s", SLAVE_LOG_PATH, s_info.file_id, s_info.filename);

			if (s_info.fp) {
				if (fclose(s_info.fp) != 0) {
					ErrPrint("fclose: %s\n", strerror(errno));
				}
			}

			s_info.fp = fopen(filename, "w+");
			if (!s_info.fp) {
				ErrPrint("Failed to open a file: %s\n", filename);
			}

			free(filename);
		}

		s_info.nr_of_lines = 0;
	}
	return ret;
}



int critical_log_init(const char *name)
{
	int namelen;
	char *filename;

	if (s_info.fp) {
		return 0;
	}

	s_info.filename = strdup(name);
	if (!s_info.filename) {
		ErrPrint("Failed to create a log file\n");
		return LB_STATUS_ERROR_MEMORY;
	}

	namelen = strlen(name) + strlen(SLAVE_LOG_PATH) + 20;

	filename = malloc(namelen);
	if (!filename) {
		ErrPrint("Failed to create a log file\n");
		free(s_info.filename);
		s_info.filename = NULL;
		return LB_STATUS_ERROR_MEMORY;
	}

	snprintf(filename, namelen, "%s/%d_%s", SLAVE_LOG_PATH, s_info.file_id, name);

	s_info.fp = fopen(filename, "w+");
	if (!s_info.fp) {
		ErrPrint("Failed to open log: %s\n", strerror(errno));
		free(s_info.filename);
		s_info.filename = NULL;
		free(filename);
		return LB_STATUS_ERROR_IO;
	}

	free(filename);
	return 0;
}



int critical_log_fini(void)
{
	if (s_info.filename) {
		free(s_info.filename);
		s_info.filename = NULL;
	}

	if (s_info.fp) {
		fclose(s_info.fp);
		s_info.fp = NULL;
	}

	return 0;
}



/* End of a file */
