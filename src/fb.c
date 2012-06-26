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

#include "debug.h"
#include "util.h"
#include "fb.h"

int errno;

struct fb_info {
	enum {
		FB_TYPE_UNKNOWN,
		FB_TYPE_FILE,
		FB_TYPE_SHM,
	} type;
	char *filename;
	int w;
	int h;
	int bufsz;
	void *buffer;

	int id;
};

struct buffer {
	enum {
		CREATED = 0x00beef00,
		DESTROYED = 0x00dead00,
	} state;
	enum {
		BUFFER_FILE = 0x0,
		BUFFER_SHM = 0x1,
	} type;
	int refcnt;
	char data[];
};

int fb_init(void)
{
	return 0;
}

int fb_fini(void)
{
	return 0;
}

/*
static inline struct flock *file_lock(short type, short whence)
{
	static struct flock ret;

	ret.l_type = type;
	ret.l_start = 0;
	ret.l_whence = whence;
	ret.l_len = 0;
	ret.l_pid = getpid();
	return &ret;
}
*/

int fb_sync(struct fb_info *info)
{
	int fd;
	struct buffer *buffer;

	if (!info || !info->buffer) {
		ErrPrint("FB Handle is not valid\n");
		return -EINVAL;
	}

	buffer = info->buffer;

	if (buffer->state != CREATED) {
		ErrPrint("Invalid state of a FB\n");
		return -EINVAL;
	}

	if (!info->filename || info->filename[0] == '\0') {
		DbgPrint("Ingore sync\n");
		return 0;
	}

	if (info->type == FB_TYPE_UNKNOWN) {
		DbgPrint("Ingore sync\n");
		return 0;
	}

	fd = open(info->filename, O_RDONLY);
	if (fd < 0) {
		ErrPrint("Failed to open a file %s because of %s\n", info->filename, strerror(errno));
		return -EIO;
	}

	if (read(fd, buffer->data, info->bufsz) != info->bufsz) {
		ErrPrint("read: %s\n", strerror(errno));
		close(fd);
		return -EIO;
	}

	close(fd);
	DbgPrint("Sync done (%s, %p)\n", info->filename, buffer);
	return 0;
}

struct fb_info *fb_create(const char *filename, int w, int h)
{
	struct fb_info *info;

	if (!filename || filename[0] == '\0') {
		ErrPrint("Invalid filename\n");
		return NULL;
	}

	info = calloc(1, sizeof(*info));
	if (!info) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return NULL;
	}

	info->filename = strdup(filename);
	if (!info->filename) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(info);
		return NULL;
	}

	if (!strcmp(info->filename, "undefined")) {
		info->type = FB_TYPE_UNKNOWN;
		info->id = -EINVAL;
	} else if (sscanf(info->filename, "shm://%d", &info->id) == 1) {
		info->type = FB_TYPE_SHM;
	} else {
		info->type = FB_TYPE_FILE;
		info->id = -EINVAL;
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

	info->bufsz = info->w * info->h * sizeof(int);
	if (info->bufsz == 0) {
		DbgPrint("Buffer size is ZERO(%d)\n", info->bufsz);
		return 0;
	}

	if (info->type == FB_TYPE_FILE || info->type == FB_TYPE_UNKNOWN) {
		buffer = calloc(1, sizeof(*buffer) + info->bufsz);
		if (!buffer) {
			ErrPrint("Heap: %s\n", strerror(errno));
			info->bufsz = 0;
			return -ENOMEM;
		}

		buffer->type = BUFFER_FILE;
	} else {
		DbgPrint("SHMID: %d\n", info->id);

		buffer = shmat(info->id, NULL, 0);
		if (!buffer) {
			ErrPrint("shmat: %s\n", strerror(errno));
			info->bufsz = 0;
			return -EFAULT;
		}

		buffer->type = BUFFER_SHM;
	}
	buffer->refcnt = 1;
	buffer->state = CREATED;
	info->buffer = buffer;

	DbgPrint("FB allocated (%p)\n", info->buffer);

	return 0;
}

int fb_destroy_buffer(struct fb_info *info)
{
	struct buffer *buffer;

	if (!info) {
		ErrPrint("Handle is not valid\n");
		return -EINVAL;
	}

	if (info->buffer) {
		buffer = info->buffer;
		/*!
		 * \note
		 * Reset the buffer pointer.
		 */
		info->buffer = NULL;
		info->bufsz = 0;
		fb_release_buffer(buffer->data);
	}
	return 0;
}

int fb_destroy(struct fb_info *info)
{
	if (!info) {
		ErrPrint("Handle is not valid\n");
		return -EINVAL;
	}

	if (info->buffer) {
		struct buffer *buffer;
		buffer = info->buffer;
		fb_release_buffer(buffer->data);
	}

	free(info->filename);
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

	buffer = info->buffer;
	if (!buffer)
		return NULL;

	buffer->refcnt++;
	return buffer->data;
}

int fb_release_buffer(void *data)
{
	struct buffer *buffer;

	if (!data)
		return 0;

	buffer = container_of(data, struct buffer, data);

	if (buffer->state != CREATED) {
		ErrPrint("Invalid handle\n");
		return -EINVAL;
	}

	buffer->refcnt--;
	if (buffer->refcnt == 0) {
		DbgPrint("FB released (%p)\n", buffer);
		buffer->state = DESTROYED;

		if (buffer->type == BUFFER_SHM) {
			if (shmdt(buffer) < 0)
				ErrPrint("shmdt: %s\n", strerror(errno));
		} else if (buffer->type == BUFFER_FILE) {
			free(buffer);
		}
	}

	return 0;
}

int fb_refcnt(void *data)
{
	struct buffer *buffer;

	buffer = container_of(data, struct buffer, data);

	if (buffer->state != CREATED) {
		ErrPrint("Invalid handle\n");
		return -EINVAL;
	}

	return buffer->refcnt;
}

const char *fb_filename(struct fb_info *info)
{
	return info ? info->filename : NULL;
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
