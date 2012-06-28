#include <appcore-efl.h>
#include <Elementary.h>
#include <Ecore_X.h>
#include <unistd.h>

#include <dlog.h>
#include <ail.h>

#include <livebox.h>
#include <shortcut.h>

static struct info {
	Eina_List *boxes;
	Evas_Object *win;
	Evas_Object *bg;
	Evas_Coord w, h;
	Evas_Object *pkg_list;
	Evas_Object *c_list;
	Evas_Object *s_list;
	Evas_Object *pd_btn;
	Evas_Object *create_btn;
	char *pkgname;
	char *content;
	char *cluster;
	char *category;
} s_info = {
	.boxes = NULL,
	.win = NULL,
	.bg = NULL,
	.w = 0,
	.h = 0,
	.pkgname = NULL,
	.content = NULL,
	.cluster = NULL,
	.category = NULL,
	.pkg_list = NULL,
	.c_list = NULL,
	.s_list = NULL,
	.pd_btn = NULL,
};

struct box_info {
	struct livebox *handler;
	Evas_Object *box;
	Evas_Object *pd;
};

char *util_get_iconfile(const char *pkgname)
{
	ail_appinfo_h handle;
	ail_error_e ret;
	char *iconfile;
	char *ret_iconfile;

	ret = ail_package_get_appinfo(pkgname, &handle);
	if (ret != AIL_ERROR_OK) {
		fprintf(stderr, "ail get pkgname = %s\n", pkgname);
		return NULL;
	}

	ret = ail_appinfo_get_str(handle, AIL_PROP_ICON_STR, &iconfile);
	if (ret != AIL_ERROR_OK) {
		ret = ail_package_destroy_appinfo(handle);
		fprintf(stderr, "Get iconfile from pkgname = %s\n", pkgname);
		return NULL;
	}

	ret_iconfile = strdup(iconfile);
	if (!ret_iconfile)
		fprintf(stderr, "Error: %s\n", strerror(errno));

	ret = ail_package_destroy_appinfo(handle);
	if (ret != AIL_ERROR_OK) {
		if (ret_iconfile)
			free(ret_iconfile);
		fprintf(stderr, "Failed to destory appinfo\n");
		return NULL;
	}

	return ret_iconfile;
}

static void lb_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Down *down = event_info;
	Evas_Coord x, y, w, h;
	double rx, ry;
	struct box_info *info = data;

	evas_object_geometry_get(obj, &x, &y, &w, &h);

	rx = (double)(down->canvas.x - x) / (double)w;
	ry = (double)(down->canvas.y - y) / (double)h;

	livebox_content_event(info->handler, LB_MOUSE_DOWN, rx, ry);
}

static void lb_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Up *up = event_info;
	Evas_Coord x, y, w, h;
	double rx, ry;
	struct box_info *info = data;

	evas_object_geometry_get(obj, &x, &y, &w, &h);

	rx = (double)(up->canvas.x - x) / (double)w;
	ry = (double)(up->canvas.y - y) / (double)h;

	livebox_content_event(info->handler, LB_MOUSE_MOVE, rx, ry);
}

static void lb_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Up *up = event_info;
	Evas_Coord x, y, w, h;
	double rx, ry;
	struct box_info *info = data;

	evas_object_geometry_get(obj, &x, &y, &w, &h);

	rx = (double)(up->canvas.x - x) / (double)w;
	ry = (double)(up->canvas.y - y) / (double)h;

	livebox_content_event(info->handler, LB_MOUSE_UP, rx, ry);
}

static void pd_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Down *down = event_info;
	Evas_Coord x, y, w, h;
	double rx, ry;
	struct box_info *info = data;

	evas_object_geometry_get(obj, &x, &y, &w, &h);

	rx = (double)(down->canvas.x - x) / (double)w;
	ry = (double)(down->canvas.y - y) / (double)h;

	livebox_content_event(info->handler, PD_MOUSE_DOWN, rx, ry);
}

static void pd_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Up *up = event_info;
	Evas_Coord x, y, w, h;
	double rx, ry;
	struct box_info *info = data;

	evas_object_geometry_get(obj, &x, &y, &w, &h);

	rx = (double)(up->canvas.x - x) / (double)w;
	ry = (double)(up->canvas.y - y) / (double)h;

	livebox_content_event(info->handler, PD_MOUSE_UP, rx, ry);
}

static void pd_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Move *move = event_info;
	Evas_Coord x, y, w, h;
	double rx, ry;
	struct box_info *info = data;

	evas_object_geometry_get(obj, &x, &y, &w, &h);

	rx = (double)(move->cur.canvas.x - x) / (double)w;
	ry = (double)(move->cur.canvas.y - y) / (double)h;

	livebox_content_event(info->handler, PD_MOUSE_MOVE, rx, ry);
}

static void box_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Up *up = event_info;
	Evas_Coord x, y, w, h;
	double rx, ry;
	struct box_info *info = data;

	evas_object_geometry_get(obj, &x, &y, &w, &h);

	rx = (double)(up->canvas.x - x) / (double)w;
	ry = (double)(up->canvas.y - y) / (double)h;

	livebox_click(info->handler, rx, ry);
}

static inline int create_new_box(struct livebox *handler)
{
	struct box_info *info;

	info = malloc(sizeof(*info));
	if (!info) {
		LOGE("Heap: %s\n", strerror(errno));
		return -ENOMEM;
	}

	info->handler = handler;

	info->box = evas_object_image_filled_add(evas_object_evas_get(s_info.win));
	if (!info->box) {
		fprintf(stderr, "Failed to add an image object\n");
	} else {
		Evas_Coord w, h;
		Evas_Coord x, y;
		void *fb = NULL;

		livebox_get_size(handler, &w, &h);
		fprintf(stderr, "created size: %dx%d\n", w, h);
		if (livebox_lb_type(handler) == LB_TYPE_BUFFER) {
			fb = livebox_acquire_fb(handler);
			if (fb) {
				evas_object_image_size_set(info->box, w, h);
				evas_object_image_colorspace_set(info->box, EVAS_COLORSPACE_ARGB8888);
				evas_object_image_alpha_set(info->box, EINA_TRUE);
				evas_object_image_fill_set(info->box, 0, 0, w, h);
				evas_object_image_data_copy_set(info->box, fb);
				evas_object_image_data_update_add(info->box, 0, 0, w, h);

				evas_object_event_callback_add(info->box, EVAS_CALLBACK_MOUSE_DOWN, lb_mouse_down_cb, info);
				evas_object_event_callback_add(info->box, EVAS_CALLBACK_MOUSE_MOVE, lb_mouse_move_cb, info);
				evas_object_event_callback_add(info->box, EVAS_CALLBACK_MOUSE_UP, lb_mouse_up_cb, info);
				fprintf(stderr, "Buffer type livebox is created\n");
			} else {
				fprintf(stderr, "Failed to get FB <<<<<<<<<<<<<<<<<<<<<<<<<<<<,\n");
			}
		} else {
			evas_object_image_file_set(info->box, livebox_filename(info->handler), NULL);
			evas_object_image_fill_set(info->box, 0, 0, w, h);
		}
		evas_object_resize(info->box, w, h);

		y = (rand() % ((s_info.h / 2) - h));
		x = (rand() % (s_info.w - w));

		if (y < 40)
			y = 40;

		if (y + h > s_info.h)
			y = s_info.h - h;

		if (x < 0)
			x = 0;

		if (x + w > s_info.w)
			x = s_info.w - x;

		evas_object_move(info->box, x, y);
		evas_object_event_callback_add(info->box, EVAS_CALLBACK_MOUSE_UP, box_mouse_up_cb, info);
		evas_object_layer_set(info->box, livebox_priority(info->handler) * 10);
		evas_object_show(info->box);
		if (fb) {
			fprintf(stderr, "Release buffer begin\n");
			livebox_release_fb(fb);
			fprintf(stderr, "Release buffer done\n");
		}
	}

	info->pd = NULL;

	s_info.boxes = eina_list_append(s_info.boxes, info);
	return 0;
}

static void activated_cb(struct livebox *handler, int ret, void *data)
{
	char *pkgname = data;

	fprintf(stderr, "Activate %s returns %d\n", pkgname, ret);

	free(pkgname);
}

static void lb_created_cb(struct livebox *handler, int ret, void *data)
{
	if (ret >= 0)
		create_new_box(handler);
	else
		fprintf(stderr, "Failed to create a new livebox\n");
}

static inline struct box_info *find_box_info(struct livebox *handler)
{
	Eina_List *l;
	Eina_List *n;
	struct box_info *info;

	EINA_LIST_FOREACH_SAFE(s_info.boxes, l, n, info) {
		if (info->handler == handler)
			return info;
	}

	return NULL;
}

static inline int delete_pd(struct livebox *handler)
{
	struct box_info *info;

	info = find_box_info(handler);
	if (info && info->pd) {
		elm_object_part_text_set(info->pd, "default", "CreatePD");
		evas_object_del(info->pd);
		info->pd = NULL;
	}

	return 0;
}

static void pd_destroyed_cb(struct livebox *handler, int ret, void *data)
{
	struct box_info *box = data;

	if (ret == 0) {
		evas_object_del(box->pd);
		box->pd = NULL;

		elm_object_part_text_set(s_info.pd_btn, "default", "CreatePD");
	}
}

static inline int create_new_pd(struct livebox *handler)
{
	struct box_info *info;

	info = find_box_info(handler);
	if (!info)
		return -ENOENT;

	if (info->pd)
		return -EEXIST;

	info->pd = evas_object_image_add(evas_object_evas_get(s_info.win));
	if (!info->pd) {
		fprintf(stderr, "Failed to add an image object for pd\n");
	} else {
		Evas_Coord w, h;
		Evas_Coord x, y;
		void *fb;

		livebox_get_pdsize(handler, &w, &h);
		fprintf(stderr, "PDSize: %dx%d\n", w, h);
		fb = livebox_acquire_pdfb(handler);
		if (fb) {
			evas_object_image_size_set(info->pd, w, h); 
			evas_object_image_colorspace_set(info->pd, EVAS_COLORSPACE_ARGB8888);
			evas_object_image_alpha_set(info->pd, EINA_TRUE);

			evas_object_image_fill_set(info->pd, 0, 0, w, h);
			fprintf(stderr, "PD ptr: %p\n", fb);
			evas_object_image_data_copy_set(info->pd, fb);
			evas_object_image_data_update_add(info->pd, 0, 0, w, h);
			evas_object_resize(info->pd, w, h);
			if (s_info.w != w)
				x = (rand() % (s_info.w - w));
			else
				x = 0;

			y = (rand() % ((s_info.h / 2) - h));
			if (y < 40)
				y = 40;
			evas_object_move(info->pd, x, y);
			evas_object_show(info->pd);
			evas_object_layer_set(info->pd, EVAS_LAYER_MAX);

			evas_object_event_callback_add(info->pd, EVAS_CALLBACK_MOUSE_DOWN, pd_mouse_down_cb, info);
			evas_object_event_callback_add(info->pd, EVAS_CALLBACK_MOUSE_MOVE, pd_mouse_move_cb, info);
			evas_object_event_callback_add(info->pd, EVAS_CALLBACK_MOUSE_UP, pd_mouse_up_cb, info);
			fprintf(stderr, "PD created: %p\n", fb);
			livebox_release_pdfb(fb);
		}
	}

	return 0;
}

static void pd_created_cb(struct livebox *handler, int ret, void *data)
{
	if (ret == 0) {
		create_new_pd(handler);
		elm_object_part_text_set(data, "default", "DestroyPD");
	} else {
		fprintf(stderr, "Failed to create a new pd\n");
	}
}

static inline int delete_box(struct livebox *handler)
{
	Eina_List *l;
	Eina_List *n;
	struct box_info *info;

	EINA_LIST_FOREACH_SAFE(s_info.boxes, l, n, info) {
		if (info->handler == handler) {
			s_info.boxes = eina_list_remove(s_info.boxes, info);

			if (info->box)
				evas_object_del(info->box);

			if (info->pd) {
				elm_object_part_text_set(info->pd, "default", "CreatePD");
				evas_object_del(info->pd);
			}

			fprintf(stderr, "%s is deleted\n", livebox_filename(info->handler));
			/* Handler will be freed by the system */
			free(info);
			return 0;
		}
	}

	return -ENOENT;
}

static inline void reload_buffer(Evas_Object *box, void *buffer, double priority, Evas_Coord w, Evas_Coord h)
{
	Evas_Coord ow, oh, x, y;

	evas_object_geometry_get(box, &x, &y, &ow, &oh);

	x += ((ow - w) / 2);
	y += ((oh - h) / 2);

	evas_object_move(box, x, y);
	if (ow != w || oh != h) {
		evas_object_image_size_set(box, w, h);
		evas_object_image_fill_set(box, 0, 0, w, h);
		evas_object_image_data_copy_set(box, buffer);
		evas_object_resize(box, w, h);
	}

	evas_object_image_data_update_add(box, 0, 0, w, h);
	evas_object_show(box);
}

static inline void reload_file(Evas_Object *box, const char *filename, double priority, Evas_Coord w, Evas_Coord h)
{
	const char *fname;
	Evas_Coord ow, oh, x, y;

	evas_object_image_file_get(box, &fname, NULL);
	if (!strcmp(fname, filename)) {
		evas_object_image_reload(box);
		fprintf(stderr, "Reload file: %s, %s\n", fname, filename);
	} else {
		evas_object_image_file_set(box, fname, NULL);
		fprintf(stderr, "First load: %s\n", fname);
	}
	evas_object_geometry_get(box, &x, &y, &ow, &oh);

	x += ((ow - w) / 2);
	y += ((oh - h) / 2);

	evas_object_image_fill_set(box, 0, 0, w, h);
	evas_object_resize(box, w, h);

	if (y < 40)
		y = 40;

	if (y + h > s_info.h)
		y = s_info.h - h;

	if (x < 0)
		x = 0;

	if (x + w > s_info.w)
		x = s_info.w - x;

	evas_object_move(box, x, y);
	evas_object_layer_set(box, priority * 10);
	evas_object_show(box);
}

static inline int update_box(struct livebox *handler)
{
	Eina_List *l;
	struct box_info *info;
	Evas_Coord w, h;

	livebox_get_size(handler, &w, &h);
	fprintf(stderr, "lb: reload size: %dx%d\n", w, h);

	EINA_LIST_FOREACH(s_info.boxes, l, info) {
		if (info->handler == handler) {
			if (!info->box)
				return -EINVAL;

			if (livebox_lb_type(handler) == LB_TYPE_IMAGE)
				reload_file(info->box, livebox_filename(handler), livebox_priority(handler), w, h);
			else {
				void *fb;
				fb = livebox_acquire_fb(handler);
				if (fb) {
					fprintf(stderr, "pd: updated %p\n", fb);
					reload_buffer(info->box, fb, 1.0, w, h);
					livebox_release_fb(fb);
				} else {
					fprintf(stderr, ">>>>>>>>>>> Buffer is not exists\n");
				}
			}

			return 0;
		}
	}

	return create_new_box(handler);
}

static inline int update_pd(struct livebox *handler)
{
	Eina_List *l;
	struct box_info *info;
	Evas_Coord w, h;

	livebox_get_pdsize(handler, &w, &h);
	fprintf(stderr, "pd: reload size: %dx%d\n", w, h);

	EINA_LIST_FOREACH(s_info.boxes, l, info) {
		if (info->handler == handler) {
			if (!info->pd) {
				create_new_pd(handler);
			} else {
				void *fb;
				fb = livebox_acquire_pdfb(handler);
				if (fb) {
					fprintf(stderr, "pd: updated %p\n", fb);
					reload_buffer(info->pd, fb, 1.0, w, h);
					livebox_release_pdfb(fb);
				} else {
					fprintf(stderr, "!!!!!!!!!!!!!!! pd buffer is not valid\n");
				}
			}

			return 0;
		}
	}

	return -ENOENT;
}

static int fault_cb(const char *event, const char *pkgname, const char *filename, const char *funcname, void *data)
{
	fprintf(stderr, "event: %s ==========================\n", event);
	fprintf(stderr, "pkgname: %s\n", pkgname);
	fprintf(stderr, "filename: %s\n", filename);
	fprintf(stderr, "funcname: %s\n", funcname);

	livebox_activate(pkgname, activated_cb, strdup(pkgname));
	return EXIT_SUCCESS;
}

static int event_cb(struct livebox *handler, const char *event, void *data)
{
	fprintf(stderr, "event: %s ==========================\n", event);
	fprintf(stderr, "pkgname: %s\n", livebox_pkgname(handler));
	fprintf(stderr, "priority: %lf\n", livebox_priority(handler));
	fprintf(stderr, "created by %s\n", livebox_is_user(handler) ? "user" : "system");
	fprintf(stderr, "handler: %p\n", handler);

	if (!strcmp(event, "lb,created"))
		create_new_box(handler);
	else if (!strcmp(event, "lb,deleted"))
		delete_box(handler);
	else if (!strcmp(event, "lb,updated"))
		update_box(handler);
	else if (!strcmp(event, "pd,updated"))
		update_pd(handler);

	return EXIT_SUCCESS;
}

static void win_del(void *data, Evas_Object *obj, void *event)
{
	elm_exit();
}

static void item_cb(void *data, Evas_Object *obj, void *event_info)
{
	fprintf(stderr, "Pressed: %p\n", obj);
}

static void category_cb(void *data, Evas_Object *obj, void *event_info)
{
	fprintf(stderr, "category: %s\n", (char *)data);
}

static void cluster_cb(void *data, Evas_Object *obj, void *event_info)
{
	fprintf(stderr, "cluster: %s\n", (char *)data);
}

static inline void pkg_list(void)
{
	struct dirent *ent;
	DIR *dir;
	const char *name;

	s_info.pkg_list = elm_list_add(s_info.bg);
	if (!s_info.pkg_list)
		return;

	evas_object_resize(s_info.pkg_list, s_info.w / 3, s_info.h / 3 - 130);
	evas_object_move(s_info.pkg_list, 0, s_info.h - (s_info.h / 3 + 30));
	evas_object_show(s_info.pkg_list);

	dir = opendir("/opt/live");
	if (dir) {
		Elm_Object_Item *item;
		Evas_Object *icon;
		//Evas_Object *button;
		char *iconpath;
		char *tmppath;
		int len;

		while ((ent = readdir(dir))) {
			if (ent->d_name[0] == '.')
				continue;

			len = strlen(ent->d_name) * 2;
			len += strlen("/opt/live/%s/libexec/liblive-%s.so");

			tmppath = malloc(len + 1);
			if (!tmppath) {
				perror("malloc");
				continue;
			}

			snprintf(tmppath, len, "/opt/live/%s/libexec/liblive-%s.so",
								ent->d_name, ent->d_name);
			if (access(tmppath, F_OK|R_OK) != 0) {
				fprintf(stderr, "It seems that the %s is not a package\n",
										ent->d_name);
				free(tmppath);
				continue;
			}

			iconpath = util_get_iconfile(tmppath);
			if (!iconpath)
				iconpath = strdup("nofile");

			icon = NULL;
			if (access(iconpath, R_OK|F_OK) == 0) {
				icon = elm_icon_add(s_info.bg);
				if (!icon || elm_icon_file_set(icon, iconpath, NULL) == EINA_FALSE) {
					fprintf(stderr, "Failed to set icon file: %s\n", iconpath);
					evas_object_del(icon);
					icon = NULL;
				}
			}
			free(iconpath);

			len = strlen(ent->d_name);
			while (len > 0 && ent->d_name[len] != '.')
				len--;

			if (len > 0)
				name = ent->d_name + len + 1;
			else
				name = ent->d_name;

			item = elm_list_item_append(s_info.pkg_list, name, icon, NULL, item_cb, NULL);
			if (!item) {
				if (icon)
					evas_object_del(icon);
				fprintf(stderr, "Failed to append a new list item\n");
			}
		}

		closedir(dir);
	}

	elm_list_go(s_info.pkg_list);
}

static inline void group_list(void)
{
	register int i;
	Elm_Object_Item *item;
	static const char *cluster[] = {
		"people",
		"agenda",
		"information",
		"location",
		"news_feeds",
		"music",
		"photos_viedos",
		"apps",
		"condition",
		"user,created",
		NULL,
	};

	static const char *category[] = {
		"people_frequently",
		"people_rarely",
		"people_during",
		"people_at",
		"location_now",
		"location_apps",
		"location_appointment",
		"music_top_track",
		"music_recently",
		"music_top_album",
		"music_new_album",
		"media_location",
		"facebook_media",
		"media_latest",
		"apps_frequently",
		"apps_location",
		"default",
		NULL,
	};

	s_info.c_list = elm_list_add(s_info.bg);
	if (!s_info.c_list)
		return;

	evas_object_resize(s_info.c_list, s_info.w / 3, s_info.h / 3 - 130);
	evas_object_move(s_info.c_list, s_info.w / 3, s_info.h - (s_info.h / 3 + 30));
	evas_object_show(s_info.c_list);

	for (i = 0; cluster[i]; i++) {
		item = elm_list_item_append(s_info.c_list, cluster[i], NULL, NULL, cluster_cb, cluster[i]);
		if (!item)
			fprintf(stderr, "Failed to append a new list item\n");
	}

	elm_list_go(s_info.c_list);

	s_info.s_list = elm_list_add(s_info.bg);
	if (!s_info.s_list)
		return;

	evas_object_resize(s_info.s_list, s_info.w / 3, s_info.h / 3 - 130);
	evas_object_move(s_info.s_list, s_info.w * 2 / 3, s_info.h - (s_info.h / 3 + 30));
	evas_object_show(s_info.s_list);

	for (i = 0; category[i]; i++) {
		item = elm_list_item_append(s_info.s_list, category[i], NULL, NULL, category_cb, category[i]);
		if (!item)
			fprintf(stderr, "Failed to append a new list item\n");
	}

	elm_list_go(s_info.s_list);
}

static void btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *pkg;
	Elm_Object_Item *c_item;
	Elm_Object_Item *s_item;
	const char *p_name;
	const char *c_name;
	const char *s_name;
	char pkgname[PATH_MAX];

	pkg = elm_list_selected_item_get(s_info.pkg_list);
	c_item = elm_list_selected_item_get(s_info.c_list);
	s_item = elm_list_selected_item_get(s_info.s_list);

	if (pkg)
		p_name = elm_object_item_part_text_get(pkg, "default");
	else
		p_name = "nicesj";

	snprintf(pkgname, sizeof(pkgname), "com.samsung.%s", p_name);

	if (c_item)
		c_name = elm_object_item_part_text_get(c_item, "default");
	else
		c_name = "user,created";

	if (s_item)
		s_name = elm_object_item_part_text_get(s_item, "default");
	else
		s_name = "default";

	fprintf(stderr, "pkgname: %s, c_name: %s, s_name: %s\n", pkgname, c_name, s_name);
	fprintf(stderr, "content_info: \"default\" [FIXED]]\n");

	struct livebox *handler;
	handler = livebox_add(pkgname, "default", c_name, s_name, DEFAULT_PERIOD, lb_created_cb, NULL);
	if (!handler)
		fprintf(stderr, "Failed to create a livebox\n");

	fprintf(stderr, "Handler added: %p\n", handler);
}

static void pd_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	const char *text;
	Eina_List *l;
	struct box_info *box;

	l = eina_list_last(s_info.boxes);
	if (!l)
		return;

	box = eina_list_data_get(l);
	if (!box)
		return;

	text = elm_object_part_text_get(obj, "default");
	if (!strcmp(text, "DestroyPD")) {
		livebox_destroy_pd(box->handler, pd_destroyed_cb, box);
	} else if (!strcmp(text, "CreatePD")) {
		if (livebox_has_pd(box->handler))
			livebox_create_pd(box->handler, pd_created_cb, obj);
	}
}

static inline void btn(void)
{
	s_info.create_btn = elm_button_add(s_info.bg);
	elm_object_part_text_set(s_info.create_btn, "default", "Create");
	evas_object_resize(s_info.create_btn, s_info.w / 3, 100);
	evas_object_move(s_info.create_btn, s_info.w / 3, s_info.h - 100);
	evas_object_smart_callback_add(s_info.create_btn, "clicked", btn_clicked_cb, NULL);
	evas_object_show(s_info.create_btn);

	s_info.pd_btn = elm_button_add(s_info.bg);
	elm_object_part_text_set(s_info.pd_btn, "default", "CreatePD");
	evas_object_move(s_info.pd_btn, 0, s_info.h - 100);
	evas_object_resize(s_info.pd_btn, s_info.w / 3, 100);
	evas_object_smart_callback_add(s_info.pd_btn, "clicked", pd_clicked_cb, NULL);
	evas_object_show(s_info.pd_btn);
}

static inline void package_list(void)
{
	Evas_Object *text;

	text = evas_object_text_add(evas_object_evas_get(s_info.bg));
	evas_object_text_font_set(text, "SLP:Medium", 30);
	evas_object_text_text_set(text, "Select a package to load it");
	evas_object_resize(text, s_info.w / 2, 30);
	evas_object_move(text, 0, s_info.h - (s_info.h / 3));
	evas_object_color_set(text, 255, 255, 255, 255);
	evas_object_show(text);

	pkg_list();
	group_list();
	btn();
}


static int shortcut_request_cb(const char *pkgname,
					const char *name, int type,
					const char *content, const char *icon,
					int pid, double period, void *data)
{
	struct livebox *handler;

	if (livebox_is_exists(pkgname) != 1) {
		fprintf(stderr, "%s has no livebox package\n", pkgname);
		return -EINVAL;
	}
	
	handler = livebox_add(pkgname, content, "user,created", "default", period, lb_created_cb, NULL);
	if (!handler) {
		fprintf(stderr, "Failed to add a new livebox\n");
		return -EFAULT;
	}

	fprintf(stderr, "%s - [%s] is successfully created [%p], period: %lf\n", pkgname, content, handler, period);
	return 0;
}


static int app_create(void *data)
{
	int w = 0;
	int h = 0;

	livebox_init();

	s_info.win = elm_win_add(NULL, "test", ELM_WIN_BASIC);
	if (!s_info.win)
		return -1;

	elm_win_title_set(s_info.win, "test");
	elm_win_borderless_set(s_info.win, EINA_TRUE);
	evas_object_smart_callback_add(s_info.win, "delete,request", win_del, NULL);
	ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
	evas_object_resize(s_info.win, w, h);
	s_info.w = w;
	s_info.h = h;
	evas_object_show(s_info.win);
	elm_win_indicator_mode_set(s_info.win, EINA_FALSE);

	s_info.bg = evas_object_rectangle_add(evas_object_evas_get(s_info.win));
	if (s_info.bg) {
		evas_object_color_set(s_info.bg, 0, 0, 0, 255);
		evas_object_resize(s_info.bg, w, h);
		evas_object_move(s_info.bg, 0, 0);
		evas_object_show(s_info.bg);
	}

	livebox_event_handler_set(event_cb, NULL);
	livebox_fault_handler_set(fault_cb, NULL);
	shortcut_set_request_cb(shortcut_request_cb, NULL);

	package_list();
	return 0;
}

static int app_terminate(void *data)
{
	if (s_info.pd_btn)
		evas_object_del(s_info.pd_btn);

	if (s_info.create_btn)
		evas_object_del(s_info.create_btn);

	if (s_info.bg)
		evas_object_del(s_info.bg);

	if (s_info.win)
		evas_object_del(s_info.win);

	livebox_fini();
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

static int app_reset(bundle *b,void *data)
{
	struct livebox *handler;
	const char *pkgname;
	const char *content;
	const char *cluster;
	const char *category;

	if (s_info.pkgname)
		pkgname = s_info.pkgname;
	else
		pkgname = "com.samsung.nicesj";

	if (s_info.content)
		content = s_info.content;
	else
		content = "nicesj";
	
	if (s_info.cluster)
		cluster = s_info.cluster;
	else
		cluster = "user,created";

	if (s_info.category)
		category = s_info.category;
	else
		category = "default";

	fprintf(stderr, "pkgname: %s\n", pkgname);
	fprintf(stderr, "content: %s\n", content);
	fprintf(stderr, "cluster: %s\n", cluster);
	fprintf(stderr, "category: %s\n", category);

	handler = livebox_add(pkgname, content, cluster, category, DEFAULT_PERIOD, lb_created_cb, NULL);
	if (!handler)
		fprintf(stderr, "Failed to create a livebox\n");

	fprintf(stderr, "Add a new livebox!!! %p\n", handler);

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

	switch (argc) {
	case 5:
		s_info.category = strdup(argv[4]);
	case 4:
		s_info.cluster = strdup(argv[3]);
	case 3:
		s_info.content = strdup(argv[2]);
	case 2:
		s_info.pkgname = strdup(argv[1]);
	case 1:
	default:
		break;
	}

	ops.data = NULL;
	return appcore_efl_main("test", &argc, &argv, &ops);
}

/* End of a file */
