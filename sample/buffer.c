#include <stdio.h>
#include <appcore-efl.h>
#include <unistd.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <Elementary.h>
#include <Ecore_X.h>
#include <Ecore_Evas.h>

int errno;

static struct info {
	Evas_Object *win;
	struct fb_info *info;
	int w;
	int h;

	Evas_Object *bg;
	Evas_Object *pd;
} s_info = {
	.info = NULL,
	.win = NULL,
	.w = 0,
	.h = 0,

	.bg = NULL,
	.pd = NULL,
};

struct fb_info {
	Ecore_Evas *ee;
	int w;
	int h;
	char *filename;
	int fd;

	void *buffer;
	int bufsz;
};

static void *alloc_fb(void *data, int size)
{
	struct fb_info *info;

	info = data;

	info->fd = open(info->filename, O_RDWR | O_CREAT, 0644);
	if (info->fd < 0) {
		fprintf(stderr, "%s open failed: %s\n", info->filename, strerror(errno));
		if (unlink(info->filename) < 0)
			fprintf(stderr, "unlink: %s - %s\n", info->filename, strerror(errno));
		return NULL;
	}

	info->buffer = calloc(1, size);
	if (!info->buffer) {
		close(info->fd);
		info->fd = -EINVAL;
		if (unlink(info->filename) < 0)
			fprintf(stderr, "unlink: %s - %s\n", info->filename, strerror(errno));

		return NULL;
	}

	info->bufsz = size;
	return info->buffer;
}

static void free_fb(void *data, void *ptr)
{
	struct fb_info *info;

	info = data;

//	munmap(info->buffer, info->bufsz);
	if (info->buffer) {
		free(info->buffer);
		info->buffer = NULL;
	}

	if (info->fd >= 0) {
		close(info->fd);
		info->fd = -EINVAL;
	}

	if (unlink(info->filename) < 0)
		fprintf(stderr, "Unlink: %s - %s\n", info->filename, strerror(errno));
}

struct fb_info *fb_create(const char *filename, int w, int h)
{
	struct fb_info *info;

	info = calloc(1, sizeof(*info));
	if (!info) {
		fprintf(stderr, "Heap: %s\n", strerror(errno));
		return NULL;
	}

	info->w = w;
	info->h = h;

	info->filename = strdup(filename);
	if (!info->filename) {
		fprintf(stderr, "Heap: %s\n", strerror(errno));
		free(info);
		return NULL;
	}

	info->fd = -EINVAL;
	info->ee = NULL;

	return info;
}

int fb_create_buffer(struct fb_info *info)
{
	if (info->ee) {
		int w = 0;
		int h = 0;

		ecore_evas_geometry_get(info->ee, NULL, NULL, &w, &h);
		if (w != info->w || h != info->h) {
			fprintf(stderr, "EE exists, size mismatched requested (%dx%d) but (%dx%d)\n", info->w, info->h, w, h);
			ecore_evas_resize(info->ee, info->w, info->h);
		}

		return 0;
	}

	info->ee = ecore_evas_buffer_allocfunc_new(info->w, info->h, alloc_fb, free_fb, info);
	if (!info->ee) {
		fprintf(stderr, "Failed to create a buffer\n");
		return -EFAULT;
	}

	ecore_evas_alpha_set(info->ee, EINA_TRUE);
	ecore_evas_manual_render_set(info->ee, EINA_FALSE);
	ecore_evas_resize(info->ee, info->w, info->h);
	return 0;
}

int fb_destroy_buffer(struct fb_info *info)
{
	if (!info->ee) {
		fprintf(stderr, "EE is not exists\n");
		return -EINVAL;
	}

	ecore_evas_free(info->ee);
	info->ee = NULL;
	return 0;
}

int fb_destroy(struct fb_info *info)
{
	if (info->ee) {
		fprintf(stderr, "EE is not destroyed\n");
		return -EINVAL;
	}

	free(info->filename);
	free(info);
	return 0;
}

Ecore_Evas * const fb_canvas(struct fb_info *info)
{
	return info->ee;
}

const char *fb_filename(struct fb_info *fb)
{
	return (fb && fb->filename) ? fb->filename : "";
}

int fb_resize(struct fb_info *info, int w, int h)
{
	info->w = w;
	info->h = h;

	if (info->ee)
		ecore_evas_resize(info->ee, info->w, info->h);

	return 0;
}

void fb_get_size(struct fb_info *info, int *w, int *h)
{
	*w = info->w;
	*h = info->h;
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

void fb_sync(struct fb_info *info)
{
	if (info->fd < 0 || !info->buffer)
		return;

//	fcntl(info->fd, F_SETLKW, file_lock(F_WRLCK, SEEK_SET));

	if (lseek(info->fd, 0l, SEEK_SET) != 0) {
		fprintf(stderr, "Failed to do seek : %s\n", strerror(errno));
//		fcntl(info->fd, F_SETLKW, file_lock(F_UNLCK, SEEK_SET));
		return;
	}

	if (write(info->fd, info->buffer, info->bufsz) != info->bufsz)
		fprintf(stderr, "Write is not completed: %s\n", strerror(errno));

//	fcntl(info->fd, F_SETLKW, file_lock(F_UNLCK, SEEK_SET));
//	if (msync(info->buffer, info->bufsz, MS_SYNC) < 0)
//		ErrPrint("Sync: %s\n", strerror(errno));
}

/**************************************************************************/

static void win_del(void *data, Evas_Object *obj, void *event)
{
	elm_exit();
}

static int app_create(void *data)
{
	Evas_Coord w, h;
	s_info.win = elm_win_add(NULL, "test", ELM_WIN_BASIC);
	if (!s_info.win)
		return -1;

	elm_win_title_set(s_info.win, "test");
	elm_win_borderless_set(s_info.win, EINA_TRUE);
	evas_object_smart_callback_add(s_info.win, "delete,request", win_del, NULL);
	ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
	evas_object_resize(s_info.win, w, h);
	evas_object_show(s_info.win);
	elm_win_indicator_mode_set(s_info.win, EINA_FALSE);

	s_info.info = fb_create("/tmp/buffer.png", 720, 320);
	if (!s_info.info) {
		fprintf(stderr, "Failed to create a buffer\n");
		return -1;
	}

	s_info.bg = evas_object_image_filled_add(evas_object_evas_get(s_info.win));
	evas_object_image_colorspace_set(s_info.bg, EVAS_COLORSPACE_ARGB8888);
	evas_object_image_alpha_set(s_info.bg, EINA_TRUE);
	evas_object_image_size_set(s_info.bg, 720, 320);
	evas_object_image_smooth_scale_set(s_info.bg, EINA_TRUE);
	evas_object_image_fill_set(s_info.bg, 0, 0, 720, 320);
	evas_object_resize(s_info.bg, 720,  320);
	evas_object_move(s_info.bg, 0, 200);
	evas_object_show(s_info.bg);

	return 0;
}

static int app_terminate(void *data)
{
	fb_destroy_buffer(s_info.info);
	fb_destroy(s_info.info);
	if (s_info.win)
		evas_object_del(s_info.win);

	return 0;
}

static int app_pause(void *data)
{
	return 0;
}

static int app_resume(void *data)
{
	return 0;
}

static void render_post_cb(void *data, Evas *e, void *event_info)
{
	evas_image_cache_flush(e);
	evas_font_cache_flush(e);
	evas_render_dump(e);

	fprintf(stderr, "Render post invoked\n");
	fb_sync(s_info.info);

	data = (void *)ecore_evas_buffer_pixels_get(s_info.info->ee);
	if (!data) {
		fprintf(stderr, "Failed to get pixel get\n");
		evas_object_del(s_info.bg);
		s_info.bg = NULL;
		return;
	}
	evas_object_image_data_set(s_info.bg, data);
	evas_object_image_data_update_add(s_info.bg, 0, 0, 720, 320);
	return;
}

static int app_reset(bundle *b,void *data)
{
	const char *edje;
	const char *group;
	const char *source;
	const char *signal;

	fprintf(stderr, "RESET\n");

	edje = (char *)bundle_get_val(b, "edje");
	group = (char *)bundle_get_val(b, "group");
	fprintf(stderr, "EDJE: %s, GROUP: %s\n", edje, group);

	if (!edje || !group)
		return 0;

	if (s_info.pd) {
		evas_event_callback_del_full(
			ecore_evas_get(fb_canvas(s_info.info)),
			EVAS_CALLBACK_RENDER_FLUSH_POST,
			render_post_cb, NULL);
		evas_object_del(s_info.pd);
		s_info.pd = NULL;
		fb_destroy_buffer(s_info.info);
	}
	fb_create_buffer(s_info.info);
	evas_event_callback_add(ecore_evas_get(fb_canvas(s_info.info)),
				EVAS_CALLBACK_RENDER_FLUSH_POST,
				render_post_cb, NULL);

	s_info.pd = edje_object_add(ecore_evas_get(fb_canvas(s_info.info)));
	fprintf(stderr, "Load: %s (%s)\n", edje, group);
	edje_object_file_set(s_info.pd, edje, group);
	evas_object_resize(s_info.pd, 720, 320);
	evas_object_move(s_info.pd, 0, 0);
	evas_object_show(s_info.pd);

	source = (char *)bundle_get_val(b, "source");
	signal = (char *)bundle_get_val(b, "signal");
	fprintf(stderr, "Source: %s, signal: %s\n", source, signal);
	if (source && signal)
		edje_object_signal_emit(s_info.pd, signal, source);
	return 0;
}

int main(int argc, char *argv[])
{
	struct appcore_ops ops = {
		.create = app_create,
		.terminate = app_terminate,
		.pause = app_pause,
		.resume = app_resume,
		.reset = app_reset,
	};

	ops.data = NULL;
	return appcore_efl_main(argv[0], &argc, &argv, &ops);
}

/* End of a file */
