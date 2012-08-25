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
	char *id;
	int w;
	int h;
	int bufsz;
	struct buffer *buffer;

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

	if (!info) {
		ErrPrint("FB Handle is not valid\n");
		return -EINVAL;
	}

	if (!info->buffer) {
		DbgPrint("Buffer is not prepared\n");
		return 0;
	}

	if (info->buffer->state != CREATED) {
		ErrPrint("Invalid state of a FB\n");
		return -EINVAL;
	}

	if (!info->id || info->id[0] == '\0') {
		DbgPrint("Ingore sync\n");
		return 0;
	}

	if (info->buffer->type != BUFFER_TYPE_FILE) {
		DbgPrint("Ingore sync\n");
		return 0;
	}

	if (strncmp(info->id, SCHEMA_FILE, strlen(SCHEMA_FILE))) {
		DbgPrint("Invalid URI: [%s]\n", info->id);
		return -EINVAL;
	}

	fd = open(util_uri_to_path(info->id), O_RDONLY);
	if (fd < 0) {
		ErrPrint("Failed to open a file (%s) because of (%s)\n", util_uri_to_path(info->id), strerror(errno));
		return -EIO;
	}

	if (read(fd, info->buffer->data, info->bufsz) != info->bufsz) {
		ErrPrint("read: %s\n", strerror(errno));
		close(fd);
		return -EIO;
	}

	close(fd);
	DbgPrint("Sync done (%s, %p)\n", info->id, info->buffer);
	return 0;
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
	} else if (sscanf(info->id, SCHEMA_PIXMAP "%d", &info->handle) == 1) {
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

	info->bufsz = info->w * info->h * sizeof(int);
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
		buffer = NULL;
		ErrPrint("pixmap is not supported yet\n");
	} else {
		ErrPrint("Invalid FB type\n");
		return -EINVAL;
	}

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

	if (!info->buffer)
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
		ErrPrint("Buffer is not created\n");
		return NULL;
	}

	buffer = info->buffer;

	if (buffer->type == BUFFER_TYPE_SHM) {
		buffer = shmat(info->handle, NULL, 0);
		if (buffer == (void *)-1) {
			ErrPrint("shmat: %s\n", strerror(errno));
			return NULL;
		}
	} else if (buffer->type == BUFFER_TYPE_PIXMAP) {
		ErrPrint("Pixmap is not supported yet\n");
		return NULL;
	} else {
		buffer->refcnt++;
		DbgPrint("FB acquired (%p) %d\n", buffer, buffer->refcnt);
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
		ErrPrint("Pixmap is not supported yet\n");
		return -ENOTSUP;
	} else {
		buffer->refcnt--;
		if (buffer->refcnt == 0) {
			DbgPrint("FB released (%p)\n", buffer);
			buffer->state = DESTROYED;
			free(buffer);
		} else {
			DbgPrint("FB decrease[%p] refcnt: %d\n", buffer, buffer->refcnt);
		}
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
		ErrPrint("Pixmap is not supported yet\n");
		return -ENOTSUP;
	} else {
		ret = buffer->refcnt;
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
