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
	int fd;
	void *buffer;
	int created;
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
	if (info->created != 1)
		return -EINVAL;

//	fcntl(info->fd, F_SETLKW, file_lock(F_RDLCK, SEEK_SET));
	if (lseek(info->fd, 0l, SEEK_SET) != 0) {
		ErrPrint("seek: %s\n", strerror(errno));
//		fcntl(info->fd, F_SETLKW, file_lock(F_UNLCK, SEEK_SET));
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

	if (!filename || filename[0] == '\0')
		return NULL;

	info = calloc(1, sizeof(*info));
	if (!info) {
		ErrPrint("Memory: %s\n", strerror(errno));
		return NULL;
	}

	info->filename = strdup(filename);
	if (!info->filename) {
		ErrPrint("Memory: %s\n", strerror(errno));
		free(info);
		return NULL;
	}

	info->fd = -EINVAL;
	info->bufsz = -EINVAL;
	info->buffer = NULL;
	info->w = w;
	info->h = h;
	info->created = 0;

	return info;
}

int fb_create_buffer(struct fb_info *info)
{
	if (!info)
		return -EINVAL;

	if (info->created == 1)
		return -EALREADY;

	info->fd = open(info->filename, O_RDWR);
	if (info->fd < 0) {
		ErrPrint("Open: %s\n", strerror(errno));
		return -EIO;
	}

	info->bufsz = info->w * info->h * sizeof(int);

	/*
	info->bufsz = lseek(info->fd, 0l, SEEK_END);
	if (info->bufsz < 0) {
		ErrPrint("lseek: %s\n", strerror(errno));
		close(info->fd);
		info->fd = -EINVAL;
		return -EIO;
	}

	lseek(info->fd, 0l, SEEK_SET);
	DbgPrint("Buffer size: %ld\n", info->bufsz);
	*/

	info->buffer = calloc(1, info->bufsz);
	if (!info->buffer) {
		ErrPrint("calloc: %s\n", strerror(errno));
		close(info->fd);
		info->fd = -EINVAL;
		return -ENOMEM;
	}

	info->created = 1;
	return 0;
}

int fb_destroy_buffer(struct fb_info *info)
{
	if (!info)
		return -EINVAL;

	if (info->created != 1)
		return -EINVAL;

	if (info->fd > 0) {
		close(info->fd);
		info->fd = -EINVAL;
	}

	if (info->buffer) {
		free(info->buffer);
		info->buffer = NULL;
	}

	info->created = 0;
	return 0;
}

int fb_destroy(struct fb_info *info)
{
	if (!info || info->created)
		return -EINVAL;

	free(info->filename);
	free(info);
	return 0;
}

int fb_is_created(struct fb_info *info)
{
	if (!info)
		return -EINVAL;

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
	if (!info)
		return -EINVAL;

	*w = info->w;
	*h = info->h;
	return 0;
}

int fb_size(struct fb_info *info)
{
	return info->bufsz;
}

/* End of a file */
