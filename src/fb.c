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

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/Xutil.h>

#include <dlog.h>
#include <livebox-errno.h> /* For error code */

#include "debug.h"
#include "util.h"
#include "fb.h"
#include "critical_log.h"

int errno;

struct fb_info {
	char *id;
	int w;
	int h;
	int bufsz;
	void *buffer;

	int handle;
};

struct buffer { /*!< Must has to be sync with slave & provider */
	enum {
		CREATED = 0x00beef00,
		DESTROYED = 0x00dead00
	} state;
	enum buffer_type type;
	int refcnt;
	void *info;
	char data[];
};

static struct {
	Display *disp;
	int screen;
	int depth;
	Visual *visual;
	int disp_is_opened;
} s_info = {
	.disp = NULL,
	.disp_is_opened = 0,
	.screen = -1,
	.depth = 0,
	.visual = NULL,
};

int fb_init(void *disp)
{
	s_info.disp = disp;
	if (s_info.disp) {
		Screen *screen;

		screen = DefaultScreenOfDisplay(s_info.disp);

		s_info.screen = DefaultScreen(s_info.disp);
		s_info.visual = DefaultVisualOfScreen(screen);
	}

	s_info.depth = sizeof(int); //DefaultDepthOfScreen(screen);
	return 0;
}

int fb_fini(void)
{
	if (s_info.disp_is_opened && s_info.disp) {
		XCloseDisplay(s_info.disp);
	}

	s_info.disp = NULL;
	s_info.disp_is_opened = 0;
	s_info.visual = NULL;
	s_info.screen = -1;
	s_info.depth = 0;
	return 0;
}

static inline void update_fb_size(struct fb_info *info)
{
	info->bufsz = info->w * info->h * s_info.depth;
}

static inline int sync_for_file(struct fb_info *info)
{
	int fd;
	struct buffer *buffer;

	buffer = info->buffer;

	if (!buffer) { /* Ignore this sync request */
		return LB_STATUS_SUCCESS;
	}

	if (buffer->state != CREATED) {
		ErrPrint("Invalid state of a FB\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (buffer->type != BUFFER_TYPE_FILE) {
		ErrPrint("Invalid buffer\n");
		return LB_STATUS_SUCCESS;
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
		return LB_STATUS_SUCCESS;
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
		return LB_STATUS_SUCCESS;
	}

	if (close(fd) < 0) {
		ErrPrint("close: %s\n", strerror(errno));
	}
	return LB_STATUS_SUCCESS;
}

static inline __attribute__((always_inline)) int sync_for_pixmap(struct fb_info *info)
{
	struct buffer *buffer;
	XShmSegmentInfo si;
	XImage *xim;

	buffer = info->buffer;
	if (!buffer) { /*!< Ignore this sync request */
		return LB_STATUS_SUCCESS;
	}

	if (buffer->state != CREATED) {
		ErrPrint("Invalid state of a FB\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (buffer->type != BUFFER_TYPE_PIXMAP) {
		ErrPrint("Invalid buffer\n");
		return LB_STATUS_SUCCESS;
	}

	if (!s_info.disp) {
		s_info.disp = XOpenDisplay(NULL);
		if (s_info.disp) {
			Screen *screen;

			s_info.disp_is_opened = 1;

			screen = DefaultScreenOfDisplay(s_info.disp);

			s_info.screen = DefaultScreen(s_info.disp);
			s_info.visual = DefaultVisualOfScreen(screen);
		} else {
			ErrPrint("Failed to open a display\n");
			return LB_STATUS_ERROR_FAULT;
		}
	}

	if (info->handle == 0) {
		ErrPrint("Pixmap ID is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (info->bufsz == 0) {
		/*!
		 * If the client does not acquire the buffer,
		 * This function will do nothing.
		 * It will work only if the buffer is acquired.
		 * To sync its contents.
		 */
		DbgPrint("Nothing can be sync\n");
		return LB_STATUS_SUCCESS;
	}

	si.shmid = shmget(IPC_PRIVATE, info->bufsz, IPC_CREAT | 0666);
	if (si.shmid < 0) {
		ErrPrint("shmget: %s\n", strerror(errno));
		return LB_STATUS_ERROR_FAULT;
	}

	si.readOnly = False;
	si.shmaddr = shmat(si.shmid, NULL, 0);
	if (si.shmaddr == (void *)-1) {
		if (shmctl(si.shmid, IPC_RMID, 0) < 0) {
			ErrPrint("shmctl: %s\n", strerror(errno));
		}

		return LB_STATUS_ERROR_FAULT;
	}

	/*!
	 * \NOTE
	 * Use the 24 bits Pixmap for Video player
	 */
	xim = XShmCreateImage(s_info.disp, s_info.visual,
				(s_info.depth << 3), ZPixmap, NULL,
				&si,
				info->w, info->h);
	if (xim == NULL) {
		if (shmdt(si.shmaddr) < 0) {
			ErrPrint("shmdt: %s\n", strerror(errno));
		}

		if (shmctl(si.shmid, IPC_RMID, 0) < 0) {
			ErrPrint("shmctl: %s\n", strerror(errno));
		}

		return LB_STATUS_ERROR_FAULT;
	}

	xim->data = si.shmaddr;
	XShmAttach(s_info.disp, &si);

	XShmGetImage(s_info.disp, info->handle, xim, 0, 0, 0xFFFFFFFF);
	XSync(s_info.disp, False);

	memcpy(buffer->data, xim->data, info->bufsz);

	XShmDetach(s_info.disp, &si);
	XDestroyImage(xim);

	if (shmdt(si.shmaddr) < 0) {
		ErrPrint("shmdt: %s\n", strerror(errno));
	}

	if (shmctl(si.shmid, IPC_RMID, 0) < 0) {
		ErrPrint("shmctl: %s\n", strerror(errno));
	}

	return LB_STATUS_SUCCESS;
}

int fb_sync(struct fb_info *info)
{
	if (!info) {
		ErrPrint("FB Handle is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!info->id || info->id[0] == '\0') {
		DbgPrint("Ingore sync\n");
		return LB_STATUS_SUCCESS;
	}

	if (!strncasecmp(info->id, SCHEMA_FILE, strlen(SCHEMA_FILE))) {
		return sync_for_file(info);
	} else if (!strncasecmp(info->id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP))) {
		return sync_for_pixmap(info);
	} else if (!strncasecmp(info->id, SCHEMA_SHM, strlen(SCHEMA_SHM))) {
		/* No need to do sync */ 
		return LB_STATUS_SUCCESS;
	}

	return LB_STATUS_ERROR_INVALID;
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
		CRITICAL_LOG("Heap: %s\n", strerror(errno));
		return NULL;
	}

	info->id = strdup(id);
	if (!info->id) {
		CRITICAL_LOG("Heap: %s\n", strerror(errno));
		free(info);
		return NULL;
	}

	if (sscanf(info->id, SCHEMA_SHM "%d", &info->handle) == 1) {
		DbgPrint("SHMID: %d is gotten\n", info->handle);
	} else if (sscanf(info->id, SCHEMA_PIXMAP "%d", &info->handle) == 1) {
		DbgPrint("PIXMAP-SHMID: %d is gotten\n", info->handle);
	} else {
		info->handle = LB_STATUS_ERROR_INVALID;
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
		return LB_STATUS_ERROR_INVALID;
	}

	if (info->buffer) {
		struct buffer *buffer;
		buffer = info->buffer;

		buffer->info = NULL;
	}

	free(info->id);
	free(info);
	return LB_STATUS_SUCCESS;
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
		return NULL;
	}

	if (!info->buffer) {
		if (!strncasecmp(info->id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP))) {
			update_fb_size(info);

			buffer = calloc(1, sizeof(*buffer) + info->bufsz);
			if (!buffer) {
				CRITICAL_LOG("Heap: %s\n", strerror(errno));
				info->bufsz = 0;
				return NULL;
			}

			buffer->type = BUFFER_TYPE_PIXMAP;
			buffer->refcnt = 0;
			buffer->state = CREATED;
			buffer->info = info;
			info->buffer = buffer;

			/*!
			 * \note
			 * Just update from here.
			 */
			sync_for_pixmap(info);
		} else if (!strncasecmp(info->id, SCHEMA_FILE, strlen(SCHEMA_FILE))) {
			update_fb_size(info);

			buffer = calloc(1, sizeof(*buffer) + info->bufsz);
			if (!buffer) {
				CRITICAL_LOG("Heap: %s\n", strerror(errno));
				info->bufsz = 0;
				return NULL;
			}

			buffer->type = BUFFER_TYPE_FILE;
			buffer->refcnt = 0;
			buffer->state = CREATED;
			buffer->info = info;
			info->buffer = buffer;

			sync_for_file(info);
		} else if (!strncasecmp(info->id, SCHEMA_SHM, strlen(SCHEMA_SHM))) {
			buffer = shmat(info->handle, NULL, 0);
			if (buffer == (void *)-1) {
				ErrPrint("shmat: %s\n", strerror(errno));
				return NULL;
			}

			return buffer->data;
		} else {
			ErrPrint("Buffer is not created (%s)\n", info->id);
			return NULL;
		}
	}

	buffer = info->buffer;

	switch (buffer->type) {
	case BUFFER_TYPE_PIXMAP:
		buffer->refcnt++;
		break;
	case BUFFER_TYPE_FILE:
		buffer->refcnt++;
		break;
	default:
		DbgPrint("Unknwon FP: %d\n", buffer->type);
		break;
	}

	return buffer->data;
}

int fb_release_buffer(void *data)
{
	struct buffer *buffer;

	if (!data) {
		ErrPrint("buffer data == NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	buffer = container_of(data, struct buffer, data);

	if (buffer->state != CREATED) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	switch (buffer->type) {
	case BUFFER_TYPE_SHM:
		if (shmdt(buffer) < 0) {
			ErrPrint("shmdt: %s\n", strerror(errno));
		}
		break;
	case BUFFER_TYPE_PIXMAP:
		buffer->refcnt--;
		if (buffer->refcnt == 0) {
			struct fb_info *info;
			info = buffer->info;

			buffer->state = DESTROYED;
			free(buffer);
		
			if (info && info->buffer == buffer) {
				info->buffer = NULL;
			}
		}
		break;
	case BUFFER_TYPE_FILE:
		buffer->refcnt--;
		if (buffer->refcnt == 0) {
			struct fb_info *info;
			info = buffer->info;

			buffer->state = DESTROYED;
			free(buffer);

			if (info && info->buffer == buffer) {
				info->buffer = NULL;
			}
		}
		break;
	default:
		ErrPrint("Unknwon buffer type\n");
		break;
	}

	return LB_STATUS_SUCCESS;
}

int fb_refcnt(void *data)
{
	struct buffer *buffer;
	struct shmid_ds buf;
	int ret;

	if (!data) {
		return LB_STATUS_ERROR_INVALID;
	}

	buffer = container_of(data, struct buffer, data);

	if (buffer->state != CREATED) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	switch (buffer->type) {
	case BUFFER_TYPE_SHM:
		if (shmctl(buffer->refcnt, IPC_STAT, &buf) < 0) {
			ErrPrint("Error: %s\n", strerror(errno));
			return LB_STATUS_ERROR_FAULT;
		}

		ret = buf.shm_nattch;
		break;
	case BUFFER_TYPE_PIXMAP:
		ret = buffer->refcnt;
		break;
	case BUFFER_TYPE_FILE:
		ret = buffer->refcnt;
		break;
	default:
		ret = LB_STATUS_ERROR_INVALID;
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
		return LB_STATUS_ERROR_INVALID;
	}

	*w = info->w;
	*h = info->h;
	return LB_STATUS_SUCCESS;
}

int fb_size(struct fb_info *info)
{
	if (!info) {
		return 0;
	}

	update_fb_size(info);

	return info->bufsz;
}

/* End of a file */
