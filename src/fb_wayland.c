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
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#include <dlog.h>
#include <dynamicbox_errno.h> /* For error code */
#include <dynamicbox_buffer.h>

#include "debug.h"
#include "util.h"
#include "fb.h"

int errno;

struct fb_info {
    char *id;
    int w;
    int h;
    int bufsz;
    void *buffer;

    int pixels;
    int handle;
};

static struct {
} s_info = {
};

int fb_init(void *disp)
{
    return 0;
}

int fb_fini(void)
{
    return 0;
}

static inline void update_fb_size(struct fb_info *info)
{
    info->bufsz = info->w * info->h * info->pixels;
}

static inline int sync_for_file(struct fb_info *info)
{
    int fd;
    struct buffer *buffer;

    buffer = info->buffer;

    if (!buffer) { /* Ignore this sync request */
	return DBOX_STATUS_ERROR_NONE;
    }

    if (buffer->state != CREATED) {
	ErrPrint("Invalid state of a FB\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (buffer->type != DBOX_BUFFER_TYPE_FILE) {
	ErrPrint("Invalid buffer\n");
	return DBOX_STATUS_ERROR_NONE;
    }

    fd = open(util_uri_to_path(info->id), O_RDONLY);
    if (fd < 0) {
	ErrPrint("Failed to open a file (%s) because of (%s)\n",
		util_uri_to_path(info->id), strerror(errno));

	/*!
	 * \note
	 * But return ZERO, even if we couldn't get a buffer file,
	 * the viewer can draw empty screen.
	 *
	 * and then update it after it gots update events
	 */
	return DBOX_STATUS_ERROR_NONE;
    }

    if (read(fd, buffer->data, info->bufsz) != info->bufsz) {
	ErrPrint("read: %s\n", strerror(errno));
	if (close(fd) < 0) {
	    ErrPrint("close: %s\n", strerror(errno));
	}

	/*!
	 * \note
	 * But return ZERO, even if we couldn't get a buffer file,
	 * the viewer can draw empty screen.
	 *
	 * and then update it after it gots update events
	 */
	return DBOX_STATUS_ERROR_NONE;
    }

    if (close(fd) < 0) {
	ErrPrint("close: %s\n", strerror(errno));
    }
    return DBOX_STATUS_ERROR_NONE;
}

int fb_sync(struct fb_info *info, int x, int y, int w, int h)
{
    if (!info) {
	ErrPrint("FB Handle is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!info->id || info->id[0] == '\0') {
	DbgPrint("Ingore sync\n");
	return DBOX_STATUS_ERROR_NONE;
    }

    if (!strncasecmp(info->id, SCHEMA_FILE, strlen(SCHEMA_FILE))) {
	return sync_for_file(info);
    } else if (!strncasecmp(info->id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP))) {
    } else if (!strncasecmp(info->id, SCHEMA_SHM, strlen(SCHEMA_SHM))) {
	/* No need to do sync */ 
	return DBOX_STATUS_ERROR_NONE;
    }

    return DBOX_STATUS_ERROR_INVALID_PARAMETER;
}

struct fb_info *fb_create(const char *id, int w, int h)
{
    struct fb_info *info;

    if (!id || id[0] == '\0') {
	ErrPrint("Invalid ID\n");
	return NULL;
    }

    info = calloc(1, sizeof(*info));
    if (!info) {
	ErrPrint("Heap: %s\n", strerror(errno));
	return NULL;
    }

    info->id = strdup(id);
    if (!info->id) {
	ErrPrint("Heap: %s\n", strerror(errno));
	free(info);
	return NULL;
    }

    info->pixels = sizeof(int);    /* Use the default pixels(depth) */

    if (sscanf(info->id, SCHEMA_SHM "%d", &info->handle) == 1) {
	DbgPrint("SHMID: %d is gotten\n", info->handle);
    } else if (sscanf(info->id, SCHEMA_PIXMAP "%d:%d", &info->handle, &info->pixels) == 2) {
	DbgPrint("PIXMAP-SHMID: %d is gotten (%d)\n", info->handle, info->pixels);
	ErrPrint("Unsupported\n");
	free(info);
	return NULL;
    } else {
	info->handle = DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    info->bufsz = 0;
    info->buffer = NULL;
    info->w = w;
    info->h = h;

    return info;
}

int fb_destroy(struct fb_info *info)
{
    if (!info) {
	ErrPrint("Handle is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (info->buffer) {
	struct buffer *buffer;
	buffer = info->buffer;

	buffer->info = NULL;
    }

    free(info->id);
    free(info);
    return DBOX_STATUS_ERROR_NONE;
}

int fb_is_created(struct fb_info *info)
{
    if (!info) {
	ErrPrint("Handle is not valid\n");
	return 0;
    }

    if (!strncasecmp(info->id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP)) && info->handle != 0) {
	return 1;
    } else if (!strncasecmp(info->id, SCHEMA_SHM, strlen(SCHEMA_SHM)) && info->handle > 0) {
	return 1;
    } else {
	const char *path;
	path = util_uri_to_path(info->id);
	if (path && access(path, F_OK | R_OK) == 0) {
	    return 1;
	} else {
	    ErrPrint("access: %s (%s)\n", strerror(errno), path);
	}
    }

    return 0;
}

void *fb_acquire_buffer(struct fb_info *info)
{
    struct buffer *buffer;

    if (!info) {
	ErrPrint("info == NIL\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return NULL;
    }

    if (!info->buffer) {
	if (!strncasecmp(info->id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP))) {
	    ErrPrint("Unsupported Type\n");
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	    return NULL;
	} else if (!strncasecmp(info->id, SCHEMA_FILE, strlen(SCHEMA_FILE))) {
	    update_fb_size(info);

	    buffer = calloc(1, sizeof(*buffer) + info->bufsz);
	    if (!buffer) {
		ErrPrint("Heap: %s\n", strerror(errno));
		info->bufsz = 0;
		dynamicbox_set_last_status(DBOX_STATUS_ERROR_OUT_OF_MEMORY);
		return NULL;
	    }

	    buffer->type = DBOX_BUFFER_TYPE_FILE;
	    buffer->refcnt = 0;
	    buffer->state = CREATED;
	    buffer->info = info;
	    info->buffer = buffer;

	    sync_for_file(info);
	} else if (!strncasecmp(info->id, SCHEMA_SHM, strlen(SCHEMA_SHM))) {
	    buffer = shmat(info->handle, NULL, 0);
	    if (buffer == (void *)-1) {
		ErrPrint("shmat: %s (%d)\n", strerror(errno), info->handle);
		dynamicbox_set_last_status(DBOX_STATUS_ERROR_FAULT);
		return NULL;
	    }

	    return buffer->data;
	} else {
	    ErrPrint("Buffer is not created (%s)\n", info->id);
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	    return NULL;
	}
    }

    buffer = info->buffer;

    switch (buffer->type) {
	case DBOX_BUFFER_TYPE_FILE:
	    buffer->refcnt++;
	    break;
	case DBOX_BUFFER_TYPE_PIXMAP:
	default:
	    DbgPrint("Unknwon FP: %d\n", buffer->type);
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	    break;
    }

    return buffer->data;
}

int fb_release_buffer(void *data)
{
    struct buffer *buffer;

    if (!data) {
	ErrPrint("buffer data == NIL\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    buffer = container_of(data, struct buffer, data);

    if (buffer->state != CREATED) {
	ErrPrint("Invalid handle\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    switch (buffer->type) {
	case DBOX_BUFFER_TYPE_SHM:
	    if (shmdt(buffer) < 0) {
		ErrPrint("shmdt: %s\n", strerror(errno));
	    }
	    break;
	case DBOX_BUFFER_TYPE_FILE:
	    buffer->refcnt--;
	    if (buffer->refcnt == 0) {
		struct fb_info *info;
		info = buffer->info;

		buffer->state = DBOX_FB_STATE_DESTROYED;
		free(buffer);

		if (info && info->buffer == buffer) {
		    info->buffer = NULL;
		}
	    }
	    break;
	case DBOX_BUFFER_TYPE_PIXMAP:
	default:
	    ErrPrint("Unknwon buffer type\n");
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	    break;
    }

    return DBOX_STATUS_ERROR_NONE;
}

int fb_refcnt(void *data)
{
    struct buffer *buffer;
    struct shmid_ds buf;
    int ret;

    if (!data) {
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    buffer = container_of(data, struct buffer, data);

    if (buffer->state != CREATED) {
	ErrPrint("Invalid handle\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    switch (buffer->type) {
	case DBOX_BUFFER_TYPE_SHM:
	    if (shmctl(buffer->refcnt, IPC_STAT, &buf) < 0) {
		ErrPrint("Error: %s\n", strerror(errno));
		dynamicbox_set_last_status(DBOX_STATUS_ERROR_FAULT);
		return DBOX_STATUS_ERROR_FAULT;
	    }

	    ret = buf.shm_nattch;
	    break;
	case DBOX_BUFFER_TYPE_FILE:
	    ret = buffer->refcnt;
	    break;
	case DBOX_BUFFER_TYPE_PIXMAP:
	default:
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	    ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
	    break;
    }

    return ret;
}

const char *fb_id(struct fb_info *info)
{
    return info ? info->id : NULL;
}

int fb_get_size(struct fb_info *info, int *w, int *h)
{
    if (!info) {
	ErrPrint("Handle is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    *w = info->w;
    *h = info->h;
    return DBOX_STATUS_ERROR_NONE;
}

int fb_size(struct fb_info *info)
{
    if (!info) {
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return 0;
    }

    update_fb_size(info);

    return info->bufsz;
}

int fb_type(struct fb_info *info)
{
    struct buffer *buffer;

    if (!info) {
	return DBOX_BUFFER_TYPE_ERROR;
    }

    buffer = info->buffer;
    if (!buffer) {
	int type = DBOX_BUFFER_TYPE_ERROR;
	/*!
	 * \note
	 * Try to get this from SCHEMA
	 */
	if (info->id) {
	    if (!strncasecmp(info->id, SCHEMA_FILE, strlen(SCHEMA_FILE))) {
		type = DBOX_BUFFER_TYPE_FILE;
	    } else if (!strncasecmp(info->id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP))) {
		/* Unsupported type */
	    } else if (!strncasecmp(info->id, SCHEMA_SHM, strlen(SCHEMA_SHM))) {
		type = DBOX_BUFFER_TYPE_SHM;
	    }
	}

	return type;
    }

    return buffer->type;
}
/* End of a file */
