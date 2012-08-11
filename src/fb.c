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
#include "critical_log.h"

int errno;

struct fb_info {
	enum {
		FB_TYPE_UNKNOWN,
		FB_TYPE_FILE,
		FB_TYPE_SHM,
	} type;
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

	if (!info) {
		ErrPrint("FB Handle is not valid\n");
		return -EINVAL;
	}

	if (!info->buffer) {
		DbgPrint("Buffer is not prepared\n");
		return 0;
	}

	buffer = info->buffer;

	if (buffer->state != CREATED) {
		ErrPrint("Invalid state of a FB\n");
		return -EINVAL;
	}

	if (!info->id || info->id[0] == '\0') {
		DbgPrint("Ingore sync\n");
		return 0;
	}

	if (info->type == FB_TYPE_UNKNOWN) {
		DbgPrint("Ingore sync\n");
		return 0;
	}

	if (strncmp(info->id, "file://", 7)) {
		DbgPrint("Invalid URI: [%s]\n", info->id);
		return -EINVAL;
	}

	fd = open(URI_TO_PATH(info->id), O_RDONLY);
	if (fd < 0) {
		ErrPrint("Failed to open a file (%s) because of (%s)\n", URI_TO_PATH(info->id), strerror(errno));
		return -EIO;
	}

	if (read(fd, buffer->data, info->bufsz) != info->bufsz) {
		ErrPrint("read: %s\n", strerror(errno));
		close(fd);
		return -EIO;
	}

	close(fd);
	DbgPrint("Sync done (%s, %p)\n", info->id, buffer);
	return 0;
}

struct fb_info *fb_create(const char *id, int w, int h)
{
	struct fb_info *info;

	if (!id || id[0] == '\0') {
		ErrPrint("Invalid id\n");
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

	if (!strlen(info->id)) {
		info->type = FB_TYPE_UNKNOWN;
		info->handle = -EINVAL;
	} else if (sscanf(info->id, "shm://%d", &info->handle) == 1) {
		info->type = FB_TYPE_SHM;
	} else {
		info->type = FB_TYPE_FILE;
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

	info->bufsz = info->w * info->h * sizeof(int);
	if (info->bufsz == 0) {
		DbgPrint("Buffer size is ZERO(%d)\n", info->bufsz);
		return 0;
	}

	if (info->type == FB_TYPE_FILE || info->type == FB_TYPE_UNKNOWN) {
		buffer = calloc(1, sizeof(*buffer) + info->bufsz);
		if (!buffer) {
			CRITICAL_LOG("Heap: %s\n", strerror(errno));
			info->bufsz = 0;
			return -ENOMEM;
		}

		buffer->type = BUFFER_TYPE_FILE;
	} else {
		DbgPrint("SHMID: %d\n", info->handle);

		buffer = shmat(info->handle, NULL, 0);
		if (!buffer) {
			ErrPrint("shmat: %s\n", strerror(errno));
			info->bufsz = 0;
			return -EFAULT;
		}

		buffer->type = BUFFER_TYPE_SHM;
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

	if (!info)
		return NULL;

	buffer = info->buffer;
	if (!buffer)
		return NULL;

	buffer->refcnt++;
	return buffer->data;
}

int fb_release_buffer(void *data)
{
	struct buffer *buffer;

	if (!data) {
		DbgPrint("Here?\n");
		return 0;
	}

	buffer = container_of(data, struct buffer, data);

	if (buffer->state != CREATED) {
		ErrPrint("Invalid handle\n");
		return -EINVAL;
	}

	buffer->refcnt--;
	if (buffer->refcnt == 0) {
		DbgPrint("FB released (%p)\n", buffer);
		buffer->state = DESTROYED;

		if (buffer->type == BUFFER_TYPE_SHM) {
			if (shmdt(buffer) < 0)
				ErrPrint("shmdt: %s\n", strerror(errno));
		} else if (buffer->type == BUFFER_TYPE_FILE) {
			free(buffer);
		}
	}

	return 0;
}

int fb_refcnt(void *data)
{
	struct buffer *buffer;

	if (!data)
		return -EINVAL;

	buffer = container_of(data, struct buffer, data);

	if (buffer->state != CREATED) {
		ErrPrint("Invalid handle\n");
		return -EINVAL;
	}

	return buffer->refcnt;
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
