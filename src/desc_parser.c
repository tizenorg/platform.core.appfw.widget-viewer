/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
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

#include <dlog.h>
#include <livebox-errno.h>

#include "debug.h"
#include "livebox.h"
#include "livebox_internal.h"
#include "desc_parser.h"
#include "dlist.h"
#include "util.h"
#include "critical_log.h"

#define TYPE_TEXT "text"
#define TYPE_IMAGE "image"
#define TYPE_EDJE "edje"
#define TYPE_SIGNAL "signal"
#define TYPE_INFO "info"
#define TYPE_DRAG "drag"

#define INFO_SIZE "size"
#define INFO_CATEGORY "category"

struct block {
	char *type;
	int type_len;

	char *part;
	int part_len;

	char *data;
	int data_len;

	char *file;
	int file_len;

	char *option;
	int option_len;

	char *id;
	int id_len;
};

static int update_text(struct livebox *handle, struct block *block, int is_pd)
{
	struct livebox_script_operators *ops;

	if (!block || !block->part || !block->data) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	ops = is_pd ? &handle->pd.data.ops : &handle->lb.data.ops;
	if (ops->update_text)
		ops->update_text(handle, (const char *)block->id, (const char *)block->part, (const char *)block->data);

	return 0;
}

static int update_image(struct livebox *handle, struct block *block, int is_pd)
{
	struct livebox_script_operators *ops;
	if (!block || !block->part) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	ops = is_pd ? &handle->pd.data.ops : &handle->lb.data.ops;
	if (ops->update_image)
		ops->update_image(handle, block->id, block->part, block->data, block->option);

	return 0;
}

static int update_script(struct livebox *handle, struct block *block, int is_pd)
{
	struct livebox_script_operators *ops;
	if (!block || !block->part) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	ops = is_pd ? &handle->pd.data.ops : &handle->lb.data.ops;
	if (ops->update_script)
		ops->update_script(handle, block->id, block->part, block->data, block->option);

	return 0;
}

static int update_signal(struct livebox *handle, struct block *block, int is_pd)
{
	struct livebox_script_operators *ops;

	if (!block) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	ops = is_pd ? &handle->pd.data.ops : &handle->lb.data.ops;
	if (ops->update_signal)
		ops->update_signal(handle, block->id, block->data, block->part);

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

	ops = is_pd ? &handle->pd.data.ops : &handle->lb.data.ops;

	if (sscanf(block->data, "%lfx%lf", &dx, &dy) != 2) {
		ErrPrint("Invalid format of data\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (ops->update_drag)
		ops->update_drag(handle, block->id, block->part, dx, dy);

	return 0;
}

static int update_info(struct livebox *handle, struct block *block, int is_pd)
{
	struct livebox_script_operators *ops;

	if (!block || !block->part || !block->data) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	ops = is_pd ? &handle->pd.data.ops : &handle->lb.data.ops;

	if (!strcasecmp(block->part, INFO_SIZE)) {
		int w, h;

		if (sscanf(block->data, "%dx%d", &w, &h) != 2) {
			ErrPrint("Invalid format (%s)\n", block->data);
			return LB_STATUS_ERROR_INVALID;
		}

		if (ops->update_info_size)
			ops->update_info_size(handle, block->id, w, h);

	} else if (!strcasecmp(block->part, INFO_CATEGORY)) {
		if (ops->update_info_category)
			ops->update_info_category(handle, block->id, block->data);
	}

	return 0;
}

static inline int update_begin(struct livebox *handle, int is_pd)
{
	struct livebox_script_operators *ops;

	ops = is_pd ? &handle->pd.data.ops : &handle->lb.data.ops;

	if (ops->update_begin)
		ops->update_begin(handle);

	return 0;
}

static inline int update_end(struct livebox *handle, int is_pd)
{
	struct livebox_script_operators *ops;

	ops = is_pd ? &handle->pd.data.ops : &handle->lb.data.ops;

	if (ops->update_end)
		ops->update_end(handle);

	return 0;
}

int parse_desc(struct livebox *handle, const char *descfile, int is_pd)
{
	FILE *fp;
	int ch;
	enum state {
		UNKNOWN = 0x10,
		BLOCK_OPEN = 0x11,
		FIELD = 0x12,
		VALUE = 0x13,
		BLOCK_CLOSE = 0x14,

		VALUE_TYPE = 0x00,
		VALUE_PART = 0x01,
		VALUE_DATA = 0x02,
		VALUE_FILE = 0x03,
		VALUE_OPTION = 0x04,
		VALUE_ID = 0x05,
	};
	const char *field_name[] = {
		"type",
		"part",
		"data",
		"file",
		"option",
		"id",
		NULL
	};
	enum state state;
	register int field_idx;
	register int idx = 0;
	register int i;
	struct block *block;
	struct {
		const char *type;
		int (*handler)(struct livebox *handle, struct block *block, int is_pd);
	} handlers[] = {
		{
			.type = TYPE_TEXT,
			.handler = update_text,
		},
		{
			.type = TYPE_IMAGE,
			.handler = update_image,
		},
		{
			.type = TYPE_EDJE,
			.handler = update_script,
		},
		{
			.type = TYPE_SIGNAL,
			.handler = update_signal,
		},
		{
			.type = TYPE_DRAG,
			.handler = update_drag,
		},
		{
			.type = TYPE_INFO,
			.handler = update_info,
		},
		{
			.type = NULL,
			.handler = NULL,
		},
	};

	fp = fopen(descfile, "rt");
	if (!fp) {
		ErrPrint("Error: %s\n", strerror(errno));
		return LB_STATUS_ERROR_IO;
	}

	update_begin(handle, is_pd);

	state = UNKNOWN;
	field_idx = 0;

	block = NULL;
	while (!feof(fp)) {
		ch = getc(fp);

		switch (state) {
		case UNKNOWN:
			if (ch == '{') {
				state = BLOCK_OPEN;
				break;
			}

			if (!isspace(ch)) {
				update_end(handle, is_pd);
				fclose(fp);
				return LB_STATUS_ERROR_INVALID;
			}
			break;

		case BLOCK_OPEN:
			if (isblank(ch))
				break;

			if (ch != '\n')
				goto errout;

			block = calloc(1, sizeof(*block));
			if (!block) {
				CRITICAL_LOG("Heap: %s\n", strerror(errno));
				update_end(handle, is_pd);
				fclose(fp);
				return LB_STATUS_ERROR_MEMORY;
			}

			state = FIELD;
			idx = 0;
			field_idx = 0;
			break;

		case FIELD:
			if (isspace(ch))
				break;

			if (ch == '}') {
				state = BLOCK_CLOSE;
				break;
			}

			if (ch == '=') {
				if (field_name[field_idx][idx] != '\0')
					goto errout;

				switch (field_idx) {
				case 0:
					state = VALUE_TYPE;
					if (block->type) {
						free(block->type);
						block->type = NULL;
						block->type_len = 0;
					}
					idx = 0;
					break;
				case 1:
					state = VALUE_PART;
					if (block->part) {
						free(block->part);
						block->part = NULL;
						block->part_len = 0;
					}
					idx = 0;
					break;
				case 2:
					state = VALUE_DATA;
					if (block->data) {
						free(block->data);
						block->data = NULL;
						block->data_len = 0;
					}
					idx = 0;
					break;
				case 3:
					state = VALUE_FILE;
					if (block->file) {
						free(block->file);
						block->file = NULL;
						block->file_len = 0;
					}
					idx = 0;
					break;
				case 4:
					state = VALUE_OPTION;
					if (block->option) {
						free(block->option);
						block->option = NULL;
						block->option_len = 0;
					}
					idx = 0;
					break;
				case 5:
					state = VALUE_ID;
					if (block->id) {
						free(block->id);
						block->id = NULL;
						block->id_len = 0;
					}
					idx = 0;
					break;
				default:
					goto errout;
				}

				break;
			}

			if (ch == '\n')
				goto errout;

			if (field_name[field_idx][idx] != ch) {
				ungetc(ch, fp);
				while (--idx >= 0)
					ungetc(field_name[field_idx][idx], fp);

				field_idx++;
				if (field_name[field_idx] == NULL)
					goto errout;

				idx = 0;
				break;
			}

			idx++;
			break;

		case VALUE_TYPE:
			if (idx == block->type_len) {
				block->type_len += 256;
				block->type = realloc(block->type, block->type_len);
				if (!block->type) {
					CRITICAL_LOG("Heap: %s\n", strerror(errno));
					goto errout;
				}
			}

			if (ch == '\n') {
				block->type[idx] = '\0';
				state = FIELD;
				idx = 0;
				field_idx = 0;
				break;
			}

			block->type[idx] = ch;
			idx++;
			break;

		case VALUE_PART:
			if (idx == block->part_len) {
				block->part_len += 256;
				block->part = realloc(block->part, block->part_len);
				if (!block->part) {
					CRITICAL_LOG("Heap: %s\n", strerror(errno));
					goto errout;
				}
			}

			if (ch == '\n') {
				block->part[idx] = '\0';
				state = FIELD;
				idx = 0;
				field_idx = 0;
				break;
			}

			block->part[idx] = ch;
			idx++;
			break;

		case VALUE_DATA:
			if (idx == block->data_len) {
				block->data_len += 256;
				block->data = realloc(block->data, block->data_len);
				if (!block->data) {
					CRITICAL_LOG("Heap: %s\n", strerror(errno));
					goto errout;
				}
			}

			if (ch == '\n') {
				block->data[idx] = '\0';
				state = FIELD;
				idx = 0;
				field_idx = 0;
				break;
			}

			block->data[idx] = ch;
			idx++;
			break;

		case VALUE_FILE:
			if (idx == block->file_len) {
				block->file_len += 256;
				block->file = realloc(block->file, block->file_len);
				if (!block->file) {
					CRITICAL_LOG("Heap: %s\n", strerror(errno));
					goto errout;
				}
			}

			if (ch == '\n') {
				block->file[idx] = '\0';
				state = FIELD;
				idx = 0;
				field_idx = 0;
				break;
			}

			block->file[idx] = ch;
			idx++;
			break;

		case VALUE_OPTION:
			if (idx == block->option_len) {
				block->option_len += 256;
				block->option = realloc(block->option, block->option_len);
				if (!block->option) {
					CRITICAL_LOG("Heap: %s\n", strerror(errno));
					goto errout;
				}
			}

			if (ch == '\n') {
				block->option[idx] = '\0';
				state = FIELD;
				idx = 0;
				field_idx = 0;
				break;
			}

			block->option[idx] = ch;
			idx++;
			break;
		case VALUE_ID:
			if (idx == block->id_len) {
				block->id_len += 256;
				block->id = realloc(block->id, block->id_len);
				if (!block->id) {
					CRITICAL_LOG("Heap: %s\n", strerror(errno));
					goto errout;
				}
			}

			if (ch == '\n') {
				block->id[idx] = '\0';
				state = FIELD;
				idx = 0;
				field_idx = 0;
				break;
			}

			block->id[idx] = ch;
			idx++;
			break;
		case BLOCK_CLOSE:
			if (!block->file) {
				block->file = strdup(util_uri_to_path(handle->id));
				if (!block->file)
					goto errout;
			}

			i = 0;
			while (handlers[i].type) {
				if (!strcasecmp(handlers[i].type, block->type)) {
					handlers[i].handler(handle, block, is_pd);
					break;
				}
				i++;
			}

			if (!handlers[i].type)
				ErrPrint("Unknown block type: %s\n", block->type);

			free(block->file);
			free(block->type);
			free(block->part);
			free(block->data);
			free(block->option);
			free(block->id);
			free(block);
			block = NULL;

			state = UNKNOWN;
			break;

		default:
			break;
		} /* switch */
	} /* while */

	if (state != UNKNOWN)
		goto errout;

	update_end(handle, is_pd);

	fclose(fp);
	return 0;

errout:
	ErrPrint("Parse error\n");
	if (block) {
		free(block->file);
		free(block->type);
		free(block->part);
		free(block->data);
		free(block->option);
		free(block->id);
		free(block);
	}

	update_end(handle, is_pd);

	fclose(fp);
	return LB_STATUS_ERROR_INVALID;
}

/* End of a file */
