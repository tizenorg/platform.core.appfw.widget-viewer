#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>

#include <dlog.h>

#include "debug.h"

int errno;

struct fb_info {
	char *filename;
	int w;
	int h;
	int bufsz;
	void *buffer;
	int created;
	int fd;
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
	if (!info || info->created != 1) {
		ErrPrint("FB Handle is not valid\n");
		return -EINVAL;
	}

	if (info->fd <= 0) {
		ErrPrint("Buffer handler is not valid, Let's try to fix it\n");
		info->fd = open(info->filename, O_RDONLY);
		if (info->fd < 0) {
			ErrPrint("Failed to open a file %s because of %s\n",
							info->filename, strerror(errno));
			return -EIO;
		}
	}

//	fcntl(info->fd, F_SETLKW, file_lock(F_RDLCK, SEEK_SET));
	if (lseek(info->fd, 0l, SEEK_SET) != 0) {
		ErrPrint("seek: %s\n", strerror(errno));
//		fcntl(info->fd, F_SETLKW, file_lock(F_UNLCK, SEEK_SET));
//		close(fd);
		return -EIO;
	}

	if (read(info->fd, info->buffer, info->bufsz) != info->bufsz) {
		ErrPrint("read: %s\n", strerror(errno));
//		fcntl(info->fd, F_SETLKW, file_lock(F_UNLCK, SEEK_SET));
		return -EIO;
	}
//	fcntl(info->fd, F_SETLKW, file_lock(F_UNLCK, SEEK_SET));

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

	info->bufsz = 0;
	info->buffer = NULL;
	info->w = w;
	info->h = h;
	info->created = 0;

	return info;
}

int fb_create_buffer(struct fb_info *info)
{
	if (!info) {
		ErrPrint("FB Info handle is not valid\n");
		return -EINVAL;
	}

	if (info->created == 1) {
		DbgPrint("Buffer is already created\n");
		return -EALREADY;
	}

	info->bufsz = info->w * info->h * sizeof(int);
	if (info->bufsz > 0) {
		info->buffer = calloc(1, info->bufsz);
		if (!info->buffer) {
			ErrPrint("Heap: %s\n", strerror(errno));
			return -ENOMEM;
		}
	} else {
		DbgPrint("Buffer size is ZERO(%d)\n", info->bufsz);
	}

	info->created = 1;

	info->fd = open(info->filename, O_RDONLY);
	if (info->fd < 0)
		ErrPrint("Open file %s: %s\n", info->filename, strerror(errno));

	return 0;
}

int fb_destroy_buffer(struct fb_info *info)
{
	if (!info) {
		ErrPrint("Handle is not valid\n");
		return -EINVAL;
	}

	if (info->created != 1) {
		DbgPrint("Buffer is not created\n");
		return -EINVAL;
	}

	if (info->fd > 0) {
		close(info->fd);
		info->fd = 0;
	}

	if (info->buffer) {
		free(info->buffer);
		info->buffer = NULL;
		info->bufsz = 0;
	}

	info->created = 0;
	return 0;
}

int fb_destroy(struct fb_info *info)
{
	if (!info || info->created) {
		ErrPrint("Handle is not valid\n");
		return -EINVAL;
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

	return info->created;
}

void *fb_buffer(struct fb_info *info)
{
	return info ? info->buffer : NULL;
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
	return info->bufsz;
}

/* End of a file */
