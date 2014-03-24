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
#include <errno.h>
#include <stdlib.h> /* malloc */
#include <string.h> /* strdup */
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <gio/gio.h>
#include <dlog.h>
#include <livebox-errno.h>
#include <livebox-service.h>

#include "debug.h"
#include "livebox.h"
#include "livebox_internal.h"
#include "desc_parser.h"
#include "dlist.h"
#include "util.h"

#define INFO_SIZE "size"
#define INFO_CATEGORY "category"

static const char *type_list[] = {
	"access",
	"access,operation",
	"color",
	"drag",
	"image",
	"info",
	"script",
	"signal",
	"text",
	NULL
};

static const char *field_list[] = {
	"type",
	"part",
	"data",
	"option",
	"id",
	"target",
	"file",
	NULL
};

enum block_type {
	TYPE_ACCESS,
	TYPE_ACCESS_OP,
	TYPE_COLOR,
	TYPE_DRAG,
	TYPE_IMAGE,
	TYPE_INFO,
	TYPE_SCRIPT,
	TYPE_SIGNAL,
	TYPE_TEXT,
	TYPE_MAX
};

enum field_type {
	FIELD_TYPE,
	FIELD_PART,
	FIELD_DATA,
	FIELD_OPTION,
	FIELD_ID,
	FIELD_TARGET,
	FIELD_FILE
};

struct block {
	enum block_type type;
	char *part;
	char *data;
	char *option;
	char *id;
	char *target;
	char *file;

	/* Should be released */
	char *filebuf;
	const char *filename;
};

static int update_text(struct livebox *handle, struct block *block, int is_pd)
{
	struct livebox_script_operators *ops;

	if (!block || !block->part || !block->data) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	ops = is_pd ? &handle->cbs.pd_ops : &handle->cbs.lb_ops;
	if (ops->update_text) {
		ops->update_text(handle, (const char *)block->id, (const char *)block->part, (const char *)block->data);
	}

	return 0;
}

static int update_image(struct livebox *handle, struct block *block, int is_pd)
{
	struct livebox_script_operators *ops;

	if (!block || !block->part) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	ops = is_pd ? &handle->cbs.pd_ops : &handle->cbs.lb_ops;
	if (ops->update_image) {
		ops->update_image(handle, block->id, block->part, block->data, block->option);
	}

	return 0;
}

static int update_script(struct livebox *handle, struct block *block, int is_pd)
{
	struct livebox_script_operators *ops;

	if (!block || !block->part) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	ops = is_pd ? &handle->cbs.pd_ops : &handle->cbs.lb_ops;
	if (ops->update_script) {
		ops->update_script(handle, block->id, block->target, block->part, block->data, block->option);
	}

	return 0;
}

static int update_signal(struct livebox *handle, struct block *block, int is_pd)
{
	struct livebox_script_operators *ops;

	if (!block) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	ops = is_pd ? &handle->cbs.pd_ops : &handle->cbs.lb_ops;
	if (ops->update_signal) {
		ops->update_signal(handle, block->id, block->data, block->part);
	}

	return 0;
}

static int update_drag(struct livebox *handle, struct block *block, int is_pd)
{
	double dx, dy;
	struct livebox_script_operators *ops;

	if (!block || !block->data || !block->part) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (sscanf(block->data, "%lfx%lf", &dx, &dy) != 2) {
		ErrPrint("Invalid format of data\n");
		return LB_STATUS_ERROR_INVALID;
	}

	ops = is_pd ? &handle->cbs.pd_ops : &handle->cbs.lb_ops;
	if (ops->update_drag) {
		ops->update_drag(handle, block->id, block->part, dx, dy);
	}

	return 0;
}

static int update_info(struct livebox *handle, struct block *block, int is_pd)
{
	struct livebox_script_operators *ops;

	if (!block || !block->part || !block->data) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	ops = is_pd ? &handle->cbs.pd_ops : &handle->cbs.lb_ops;
	if (!strcasecmp(block->part, INFO_SIZE)) {
		int w, h;

		if (sscanf(block->data, "%dx%d", &w, &h) != 2) {
			ErrPrint("Invalid format (%s)\n", block->data);
			return LB_STATUS_ERROR_INVALID;
		}

		if (ops->update_info_size) {
			ops->update_info_size(handle, block->id, w, h);
		}
	} else if (!strcasecmp(block->part, INFO_CATEGORY)) {
		if (ops->update_info_category) {
			ops->update_info_category(handle, block->id, block->data);
		}
	}

	return 0;
}

static int update_access(struct livebox *handle, struct block *block, int is_pd)
{
	struct livebox_script_operators *ops;

	if (!block) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	ops = is_pd ? &handle->cbs.pd_ops : &handle->cbs.lb_ops;
	if (ops->update_access) {
		ops->update_access(handle, block->id, block->part, block->data, block->option);
	}

	return 0;
}

static int operate_access(struct livebox *handle, struct block *block, int is_pd)
{
	struct livebox_script_operators *ops;

	if (!block) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	ops = is_pd ? &handle->cbs.pd_ops : &handle->cbs.lb_ops;
	if (ops->operate_access) {
		ops->operate_access(handle, block->id, block->part, block->data, block->option);
	}

	return 0;
}

static int update_color(struct livebox *handle, struct block *block, int is_pd)
{
	struct livebox_script_operators *ops;

	if (!block) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	ops = is_pd ? &handle->cbs.pd_ops : &handle->cbs.lb_ops;
	if (ops->update_color) {
		ops->update_color(handle, block->id, block->part, block->data);
	}

	return 0;
}

static inline int update_begin(struct livebox *handle, int is_pd)
{
	struct livebox_script_operators *ops;

	ops = is_pd ? &handle->cbs.pd_ops : &handle->cbs.lb_ops;
	if (ops->update_begin) {
		ops->update_begin(handle);
	}

	return 0;
}

static inline int update_end(struct livebox *handle, int is_pd)
{
	struct livebox_script_operators *ops;

	ops = is_pd ? &handle->cbs.pd_ops : &handle->cbs.lb_ops;
	if (ops->update_end) {
		ops->update_end(handle);
	}

	return 0;
}

static inline void delete_block(struct block *block)
{
	free(block->filebuf);
	free(block);
}

static inline void consuming_parsed_block(struct livebox *handle, int is_pd, struct block *block)
{
	typedef int (*update_function_t)(struct livebox *handle, struct block *block, int is_pd);
	static update_function_t updators[] = {
		update_access,
		operate_access,
		update_color,
		update_drag,
		update_image,
		update_info,
		update_script,
		update_signal,
		update_text,
		NULL
	};

	if (block->type >= 0 || block->type < TYPE_MAX) {
		(void)updators[block->type](handle, block, is_pd);
	} else {
		ErrPrint("Block type[%d] is not valid\n", block->type);
	}
}

static inline char *load_file(const char *filename)
{
	char *filebuf = NULL;
	int fd;
	off_t filesize;
	int ret;
	size_t readsize = 0;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		ErrPrint("open: %s\n", strerror(errno));
		return NULL;
	}

	filesize = lseek(fd, 0L, SEEK_END);
	if (filesize == (off_t)-1) {
		ErrPrint("lseek: %s\n", strerror(errno));
		goto errout;
	}

	if (lseek(fd, 0L, SEEK_SET) < 0) {
		ErrPrint("lseek: %s\n", strerror(errno));
		goto errout;
	}

	filebuf = malloc(filesize + 1);
	if (!filebuf) {
		ErrPrint("malloc: %s\n", strerror(errno));
		goto errout;
	}

	while (readsize < filesize) {
		ret = read(fd, filebuf + readsize, (size_t)filesize - readsize);
		if (ret < 0) {
			if (errno == EINTR) {
				DbgPrint("Read is interrupted\n");
				continue;
			}

			ErrPrint("read: %s\n", strerror(errno));
			free(filebuf);
			filebuf = NULL;
			break;
		}

		readsize += ret;
	}

	if (filebuf) {
		filebuf[readsize] = '\0';
	}

	/*!
	 * \note
	 * Now, we are ready to parse the filebuf.
	 */

errout:
	if (close(fd) < 0) {
		ErrPrint("close: %s\n", strerror(errno));
	}

	return filebuf;
}

int parse_desc(struct livebox_common *common, const char *filename, int is_pd)
{
	int type_idx = 0;
	int type_len = 0;
	int field_idx = 0;
	int field_len = 0;
	char *filebuf;
	char *fileptr;
	char *ptr = NULL;
	struct block *block = NULL;
	struct dlist *block_list = NULL;
	struct dlist *l;
	struct dlist *n;
	struct dlist *handle_iterator;
	struct livebox *handler;
	enum state {
		BEGIN,
		FIELD,
		DATA,
		END,
		DONE,
		ERROR,
	} state;

	filebuf = load_file(filename);
	if (!filebuf) {
		return LB_STATUS_ERROR_IO;
	}

	fileptr = filebuf;

	state = BEGIN;
	while (*fileptr && state != ERROR) {
		switch (state) {
		case BEGIN:
			if (*fileptr == '{') {
				block = calloc(1, sizeof(*block));
				if (!block) {
					ErrPrint("calloc: %s\n", strerror(errno));
					state = ERROR;
					continue;
				}
				state = FIELD;
				ptr = NULL;
			}
			break;
		case FIELD:
			if (isspace(*fileptr)) {
				if (ptr != NULL) {
					*fileptr = '\0';
				}
			} else if (*fileptr == '=') {
				*fileptr = '\0';
				ptr = NULL;
				state = DATA;
			} else if (ptr == NULL) {
				ptr = fileptr;
				field_idx = 0;
				field_len = 0;

				while (field_list[field_idx]) {
					if (field_list[field_idx][field_len] == *fileptr) {
						break;
					}
					field_idx++;
				}

				if (!field_list[field_idx]) {
					ErrPrint("Invalid field\n");
					state = ERROR;
					continue;
				}

				field_len++;
			} else {
				if (field_list[field_idx][field_len] != *fileptr) {
					field_idx++;
					while (field_list[field_idx]) {
						if (!strncmp(field_list[field_idx], fileptr - field_len, field_len)) {
							break;
						} else {
							field_idx++;
						}
					}

					if (!field_list[field_idx]) {
						state = ERROR;
						ErrPrint("field is not valid\n");
						continue;
					}
				}

				field_len++;
			}
			break;
		case DATA:
			switch (field_idx) {
			case FIELD_TYPE:
				if (ptr == NULL) {
					if (isspace(*fileptr)) {
						break;
					}

					if (*fileptr == '\0') {
						state = ERROR;
						ErrPrint("Type is not valid\n");
						continue;
					}

					ptr = fileptr;
					type_idx = 0;
					type_len = 0;
				}

				if (*fileptr && (*fileptr == '\n' || *fileptr == '\r' || *fileptr == '\f')) {
					*fileptr = '\0';
				}

				if (type_list[type_idx][type_len] != *fileptr) {
					type_idx++;
					while (type_list[type_idx]) {
						if (!strncmp(type_list[type_idx], fileptr - type_len, type_len)) {
							break;
						} else {
							type_idx++;
						}
					}

					if (!type_list[type_idx]) {
						state = ERROR;
						ErrPrint("type is not valid (%s)\n", fileptr - type_len);
						continue;
					}
				}

				if (!*fileptr) {
					block->type = type_idx;
					state = DONE;
					ptr = NULL;
				}

				type_len++;
				break;
			case FIELD_PART:
				if (ptr == NULL) {
					ptr = fileptr;
				}

				if (*fileptr && (*fileptr == '\n' || *fileptr == '\r' || *fileptr == '\f')) {
					*fileptr = '\0';
				}

				if (!*fileptr) {
					block->part = ptr;
					state = DONE;
					ptr = NULL;
				}
				break;
			case FIELD_DATA:
				if (ptr == NULL) {
					ptr = fileptr;
				}

				if (*fileptr && (*fileptr == '\n' || *fileptr == '\r' || *fileptr == '\f')) {
					*fileptr = '\0';
				}

				if (!*fileptr) {
					block->data = ptr;
					state = DONE;
					ptr = NULL;
				}
				break;
			case FIELD_OPTION:
				if (ptr == NULL) {
					ptr = fileptr;
				}

				if (*fileptr && (*fileptr == '\n' || *fileptr == '\r' || *fileptr == '\f')) {
					*fileptr = '\0';
				}

				if (!*fileptr) {
					block->option = ptr;
					state = DONE;
					ptr = NULL;
				}
				break;
			case FIELD_ID:
				if (ptr == NULL) {
					ptr = fileptr;
				}

				if (*fileptr && (*fileptr == '\n' || *fileptr == '\r' || *fileptr == '\f')) {
					*fileptr = '\0';
				}

				if (!*fileptr) {
					block->id = ptr;
					state = DONE;
					ptr = NULL;
				}
				break;
			case FIELD_TARGET:
				if (ptr == NULL) {
					ptr = fileptr;
				}

				if (*fileptr && (*fileptr == '\n' || *fileptr == '\r' || *fileptr == '\f')) {
					*fileptr = '\0';
				}

				if (!*fileptr) {
					block->target = ptr;
					state = DONE;
					ptr = NULL;
				}
				break;
			case FIELD_FILE:
				if (ptr == NULL) {
					ptr = fileptr;
				}

				if (*fileptr && (*fileptr == '\n' || *fileptr == '\r' || *fileptr == '\f')) {
					*fileptr = '\0';
				}

				if (!*fileptr) {
					block->target = ptr;
					state = DONE;
					ptr = NULL;
				}
			default:
				break;
			}

			break;
		case DONE:
			if (isspace(*fileptr)) {
			} else if (*fileptr == '}') {
					state = BEGIN;
					block->filename = filename;
					block_list = dlist_append(block_list, block);
					block = NULL;
			} else {
				state = FIELD;
				continue;
			}
			break;
		case END:
		default:
			break;
		}

		fileptr++;
	}

	if (state != BEGIN) {
		struct dlist *l;
		struct dlist *n;
		ErrPrint("State %d\n", state);

		free(filebuf);
		free(block);

		dlist_foreach_safe(block_list, l, n, block) {
			free(block);
			block_list = dlist_remove(block_list, l);
		}

		return LB_STATUS_ERROR_FAULT;
	}

		
	block = dlist_data(dlist_prev(block_list));
	if (block) {
		block->filebuf = filebuf;
	} else {
		ErrPrint("Last block is not exists (There is no parsed block)\n");
		free(filebuf);
	}

	ErrPrint("Begin: Set content for object\n");
	dlist_foreach(common->livebox_list, l, handler) {
		update_begin(handler, is_pd);
	}

	dlist_foreach_safe(block_list, l, n, block) {
		dlist_foreach(common->livebox_list, handle_iterator, handler) {
			consuming_parsed_block(handler, is_pd, block);
		}

		block_list = dlist_remove(block_list, l);
		delete_block(block);
	}

	dlist_foreach(common->livebox_list, l, handler) {
		update_end(handler, is_pd);
	}
	ErrPrint("End: Set content for object\n");

	return LB_STATUS_SUCCESS;
}

/* End of a file */
