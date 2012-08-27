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
		DESTROYED = 0x00dead00,
	} state;
	enum buffer_type type;
	int refcnt;
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
		s_info.depth = sizeof(int); //DefaultDepthOfScreen(screen);
	}
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

static inline int sync_for_file(struct fb_info *info)
{
	int fd;
	struct buffer *buffer;

	buffer = info->buffer;

	if (buffer->state != CREATED) {
		ErrPrint("Invalid state of a FB\n");
		return -EINVAL;
	}

	if (buffer->type != BUFFER_TYPE_FILE) {
		DbgPrint("Invalid buffer\n");
		return 0;
	}

	fd = open(util_uri_to_path(info->id), O_RDONLY);
	if (fd < 0) {
		ErrPrint("Failed to open a file (%s) because of (%s)\n", util_uri_to_path(info->id), strerror(errno));
		return -EIO;
	}

	if (read(fd, buffer->data, info->bufsz) != info->bufsz) {
		ErrPrint("read: %s\n", strerror(errno));
		close(fd);
		return -EIO;
	}

	close(fd);
	DbgPrint("Sync done (%s, %p)\n", info->id, info->buffer);
	return 0;
}

static inline int sync_for_pixmap(struct fb_info *info)
{
	struct buffer *buffer;
	XShmSegmentInfo si;
	XImage *xim;

	if (!s_info.disp) {
		s_info.disp = XOpenDisplay(NULL);
		if (s_info.disp) {
			Screen *screen;

			s_info.disp_is_opened = 1;

			screen = DefaultScreenOfDisplay(s_info.disp);

			s_info.screen = DefaultScreen(s_info.disp);
			s_info.visual = DefaultVisualOfScreen(screen);
			s_info.depth = sizeof(int); // DefaultDepthOfScreen(screen);
		} else {
			ErrPrint("Failed to open a display\n");
			return -EFAULT;
		}
	}

	buffer = info->buffer;
	if (buffer->state != CREATED) {
		ErrPrint("Invalid state of a FB\n");
		return -EINVAL;
	}

	if (buffer->type != BUFFER_TYPE_PIXMAP) {
		DbgPrint("Invalid buffer\n");
		return 0;
	}

	DbgPrint("PIXMAP: 0x%X\n", info->handle);
	if (info->handle == 0) {
		DbgPrint("Pixmap ID is not valid\n");
		return -EINVAL;
	}

	if (info->bufsz == 0) {
		DbgPrint("Nothing can be sync\n");
		return 0;
	}

	si.shmid = shmget(IPC_PRIVATE, info->bufsz, IPC_CREAT | 0666);
	if (si.shmid < 0) {
		ErrPrint("shmget: %s\n", strerror(errno));
		return -EFAULT;
	}

	si.readOnly = False;
	si.shmaddr = shmat(si.shmid, NULL, 0);
	if (si.shmaddr == (void *)-1) {
		if (shmctl(si.shmid, IPC_RMID, 0) < 0)
			ErrPrint("shmctl: %s\n", strerror(errno));
		return -EFAULT;
	}

	xim = XShmCreateImage(s_info.disp, s_info.visual, (s_info.depth << 3), ZPixmap, NULL, &si, info->w, info->h);
	if (xim == NULL) {
		if (shmdt(si.shmaddr) < 0)
			ErrPrint("shmdt: %s\n", strerror(errno));

		if (shmctl(si.shmid, IPC_RMID, 0) < 0)
			ErrPrint("shmctl: %s\n", strerror(errno));

		return -EFAULT;
	}

	DbgPrint("Depth: %d, %dx%d\n", s_info.depth, info->w, info->h);

	xim->data = si.shmaddr;
	XShmAttach(s_info.disp, &si);

	XShmGetImage(s_info.disp, info->handle, xim, 0, 0, 0xFFFFFFFF);
	XSync(s_info.disp, False);

	memcpy(buffer->data, xim->data, info->bufsz);
	DbgPrint("Buf size: %d\n", info->bufsz);

	XShmDetach(s_info.disp, &si);
	XDestroyImage(xim);

	if (shmdt(si.shmaddr) < 0)
		ErrPrint("shmdt: %s\n", strerror(errno));

	if (shmctl(si.shmid, IPC_RMID, 0) < 0)
		ErrPrint("shmctl: %s\n", strerror(errno));

	return 0;
}

int fb_sync(struct fb_info *info)
{
	if (!info) {
		ErrPrint("FB Handle is not valid\n");
		return -EINVAL;
	}

	if (!info->buffer) {
		DbgPrint("Buffer is not prepared\n");
		return 0;
	}

	if (!info->id || info->id[0] == '\0') {
		DbgPrint("Ingore sync\n");
		return 0;
	}

	if (!strncasecmp(info->id, SCHEMA_FILE, strlen(SCHEMA_FILE))) {
		return sync_for_file(info);
	} else if (!strncasecmp(info->id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP))) {
		return sync_for_pixmap(info);
	}

	return -EINVAL;
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
		info->handle = -EINVAL;
	}

	info->bufsz = 0;
	info->buffer = NULL;
	info->w = w;
	info->h = h;

	return info;
}

int fb_create_buffer(struct fb_info *info)
{
	struct buffer *buffer;

	if (!info || !!info->buffer) {
		ErrPrint("FB Info handle is not valid\n");
		return -EINVAL;
	}

	info->bufsz = info->w * info->h * s_info.depth;
	if (info->bufsz == 0) {
		DbgPrint("Buffer size is ZERO(%d)\n", info->bufsz);
		return 0;
	}

	if (!strncasecmp(info->id, SCHEMA_FILE, strlen(SCHEMA_FILE))) {
		buffer = calloc(1, sizeof(*buffer) + info->bufsz);
		if (!buffer) {
			CRITICAL_LOG("Heap: %s\n", strerror(errno));
			info->bufsz = 0;
			return -ENOMEM;
		}

		buffer->type = BUFFER_TYPE_FILE;
		buffer->refcnt = 1;
		buffer->state = CREATED;
	} else if (!strncasecmp(info->id, SCHEMA_SHM, strlen(SCHEMA_SHM))) {
		DbgPrint("SHMID: %d\n", info->handle);
		if (info->handle < 0) {
			DbgPrint("SHM is not prepared yet\n");
			info->bufsz = 0;
			return 0;
		}

		buffer = shmat(info->handle, NULL, 0);
		if (buffer == (void *)-1) {
			ErrPrint("shmat: %s\n", strerror(errno));
			info->bufsz = 0;
			return -EFAULT;
		}
	} else if (!strncasecmp(info->id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP))) {
		if (info->handle < 0) {
			DbgPrint("PIXMAP is not prepared yet\n");
			info->bufsz = 0;
			return 0;
		}

		DbgPrint("PIXMAP-ID: 0x%X\n", info->handle);

		buffer = calloc(1, sizeof(*buffer) + info->bufsz);
		if (!buffer) {
			CRITICAL_LOG("Heap: %s\n", strerror(errno));
			info->bufsz = 0;
			return -ENOMEM;
		}

		buffer->type = BUFFER_TYPE_PIXMAP;
		buffer->refcnt = 1;
		buffer->state = CREATED;
	} else {
		ErrPrint("Invalid FB type\n");
		return -EINVAL;
	}

	info->buffer = buffer;
	DbgPrint("FB(%s) allocated (%p)\n", info->id, info->buffer);
	return 0;
}

int fb_destroy_buffer(struct fb_info *info)
{
	struct buffer *buffer;

	if (!info) {
		ErrPrint("Handle is not valid\n");
		return -EINVAL;
	}

	if (!info->buffer) /* PIXMAP */
		return 0;

	buffer = info->buffer;
	/*!
	 * \note
	 * Reset the buffer pointer.
	 */
	info->buffer = NULL;
	info->bufsz = 0;
	fb_release_buffer(buffer->data);

	return 0;
}

int fb_destroy(struct fb_info *info)
{
	if (!info) {
		ErrPrint("Handle is not valid\n");
		return -EINVAL;
	}

	fb_destroy_buffer(info);
	free(info->id);
	free(info);
	return 0;
}

int fb_is_created(struct fb_info *info)
{
	if (!info) {
		ErrPrint("Handle is not valid\n");
		return 0;
	}

	return !!info->buffer;
}

void *fb_acquire_buffer(struct fb_info *info)
{
	struct buffer *buffer;

	if (!info) {
		ErrPrint("info == NIL\n");
		return NULL;
	}

	if (!info->buffer) {
		ErrPrint("Buffer is not created (%s)\n", info->id);
		return NULL;
	}

	buffer = info->buffer;

	if (buffer->type == BUFFER_TYPE_SHM) {
		buffer = shmat(info->handle, NULL, 0);
		if (buffer == (void *)-1) {
			ErrPrint("shmat: %s\n", strerror(errno));
			return NULL;
		}
		DbgPrint("SHM buffer acquired\n");
	} else if (buffer->type == BUFFER_TYPE_PIXMAP) {
		DbgPrint("Pixmap buffer acquired!!\n");
		buffer->refcnt++;
	} else if (buffer->type == BUFFER_TYPE_FILE) {
		buffer->refcnt++;
		DbgPrint("FB acquired (%p) %d\n", buffer, buffer->refcnt);
	} else {
		DbgPrint("Unknwon FP: %d\n", buffer->type);
	}

	return buffer->data;
}

int fb_release_buffer(void *data)
{
	struct buffer *buffer;

	if (!data) {
		DbgPrint("buffer data == NIL\n");
		return 0;
	}

	buffer = container_of(data, struct buffer, data);

	if (buffer->state != CREATED) {
		ErrPrint("Invalid handle\n");
		return -EINVAL;
	}

	if (buffer->type == BUFFER_TYPE_SHM) {
		if (shmdt(buffer) < 0)
			ErrPrint("shmdt: %s\n", strerror(errno));
	} else if (buffer->type == BUFFER_TYPE_PIXMAP) {
		buffer->refcnt--;
		if (buffer->refcnt == 0) {
			DbgPrint("FB released (%p)\n", buffer);
			buffer->state = DESTROYED;
			free(buffer);
		} else {
			DbgPrint("FB decrease[%p] refcnt: %d\n", buffer, buffer->refcnt);
		}
	} else if (buffer->type == BUFFER_TYPE_FILE) {
		buffer->refcnt--;
		if (buffer->refcnt == 0) {
			DbgPrint("FB released (%p)\n", buffer);
			buffer->state = DESTROYED;
			free(buffer);
		} else {
			DbgPrint("FB decrease[%p] refcnt: %d\n", buffer, buffer->refcnt);
		}
	} else {
		ErrPrint("Unknwon buffer type\n");
	}

	return 0;
}

int fb_refcnt(void *data)
{
	struct buffer *buffer;
	int ret;

	if (!data)
		return -EINVAL;

	buffer = container_of(data, struct buffer, data);

	if (buffer->state != CREATED) {
		ErrPrint("Invalid handle\n");
		return -EINVAL;
	}

	if (buffer->type == BUFFER_TYPE_SHM) {
		struct shmid_ds buf;

		if (shmctl(buffer->refcnt, IPC_STAT, &buf) < 0) {
			ErrPrint("Error: %s\n", strerror(errno));
			return -EFAULT;
		}

		ret = buf.shm_nattch;
	} else if (buffer->type == BUFFER_TYPE_PIXMAP) {
		ret = buffer->refcnt;
	} else if (buffer->type == BUFFER_TYPE_FILE) {
		ret = buffer->refcnt;
	} else {
		ret = -EINVAL;
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
		return -EINVAL;
	}

	*w = info->w;
	*h = info->h;
	return 0;
}

int fb_size(struct fb_info *info)
{
	return info ? info->bufsz : 0;
}

/* End of a file */
