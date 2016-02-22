/*
 * Samsung API
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.1 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <libintl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <glib.h>

#include <Elementary.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Edje.h>

#include <pkgmgr-info.h>
#include <system_info.h>

#include <widget_errno.h>

#include <widget_instance.h>
#include <Pepper_Efl.h>

#if defined(LOG_TAG)
#undef LOG_TAG
#endif
#define LOG_TAG "WIDGET_EVAS"
#include <dlog.h>

#include "widget_viewer_evas.h"

#if !defined(WIDGET_COUNT_OF_SIZE_TYPE)
	#define WIDGET_COUNT_OF_SIZE_TYPE 13
#endif

#if !defined(SECURE_LOGD)
#define SECURE_LOGD LOGD
#endif

#if !defined(SECURE_LOGW)
#define SECURE_LOGW LOGW
#endif

#if !defined(SECURE_LOGE)
#define SECURE_LOGE LOGE
#endif

#if !defined(S_)
#define S_(str) dgettext("sys_string", str)
#endif

#if !defined(T_)
#define T_(str) dgettext(PKGNAME, str)
#endif

#if !defined(N_)
#define N_(str) (str)
#endif

#if !defined(_)
#define _(str) gettext(str)
#endif

#if !defined(DbgPrint)
#define DbgPrint(format, arg...)	SECURE_LOGD(format, ##arg)
#endif

#if !defined(ErrPrint)
#define ErrPrint(format, arg...)	SECURE_LOGE(format, ##arg)
#endif

#if !defined(WarnPrint)
#define WarnPrint(format, arg...)	SECURE_LOGW(format, ##arg)
#endif

#if !defined(WIDGET_VIEWER_EVAS_RESOURCE_EDJ)
#define WIDGET_VIEWER_EVAS_RESOURCE_EDJ "/usr/share/widget_viewer_evas/res/edje/widget_viewer_evas.edj"
#endif

#if !defined(WIDGET_VIEWER_EVAS_UNKNOWN)
#define WIDGET_VIEWER_EVAS_UNKNOWN "/usr/share/widget_viewer_evas/res/image/unknown.png"
#endif

#if !defined(WIDGET_VIEWER_EVAS_RESOURCE_LB)
#define WIDGET_VIEWER_EVAS_RESOURCE_LB "widget"
#endif

#if !defined(WIDGET_VIEWER_EVAS_RESOURCE_IMG)
#define WIDGET_VIEWER_EVAS_RESOURCE_IMG "widget,image"
#endif

#if !defined(WIDGET_VIEWER_EVAS_RESOURCE_OVERLAY_LOADING)
#define WIDGET_VIEWER_EVAS_RESOURCE_OVERLAY_LOADING "overlay"
#endif

#define DEFAULT_OVERLAY_COUNTER 2
#define DEFAULT_OVERLAY_WAIT_TIME 1.0f
#define RESEVED_AREA_FOR_GBAR_EFFECT 100

#define WIDGET_CLASS_NAME "widget"

#define WIDGET_KEEP_BUFFER    -2

#define DEFAULT_CLUSTER "user,created"
#define DEFAULT_CATEGORY "default"

#define WIDGET_INFO_TAG "__WIDGET_INFO__"

/*!
 * \note
 * Enable this to apply shadow effect to image object (for text widget)
 * #define SUPPORT_IMAGE_EFFECT
 */

int errno;

static struct info {
	int w;
	int h;
	Evas_Object *win;
	const char *compositor_name;
	GHashTable *widget_table;
} s_info = {
	.w = 0,
	.h = 0,
	.win = NULL,
	.compositor_name = NULL,
	.widget_table = NULL,
};

struct widget_info {
	char *widget_id;
	char *instance_id;
	char *content_info;
	char *title;
	bundle *b;
	int pid;

	int permanent_delete;

	Evas_Object *layout;
};

static inline bool is_widget_feature_enabled(void)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

static void set_runtime_dir(void)
{
	char buf[256];

	snprintf(buf, sizeof(buf) - 1, "/run/user/%d", getuid());
	if (setenv("XDG_RUNTIME_DIR", buf, 0) < 0) {
		ErrPrint("Unable to set XDB_RUNTIME_DIR: %s (%s)\n", buf, strerror(errno));
	}
}

static void object_deleted_cb(void *data, Evas_Object *obj, void *event_info)
{
	ErrPrint("Object is deleted.\n");
	/**
	 * @todo
	 * ...
	 */
}

static void object_added_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct widget_info *info;
	Evas_Object *surface;
	const char *instance_id;

	surface = event_info;
	if (!surface) {
		ErrPrint("Invalid parameters\n");
		return;
	}

	/**
	 * @note
	 * Current version of Pepper only supports to pass the title of surface.
	 * So we should find the infomation data using this title.
	 * This title will be used as an instance id of the widget.
	 */
	instance_id = pepper_efl_object_title_get(surface);
	if (!instance_id) {
		ErrPrint("Instance Id is not valid\n");
		evas_object_del(surface);
		return;
	}

	info = g_hash_table_lookup(s_info.widget_table, instance_id);
	if (!info) {
		ErrPrint("Unable to find a proper object\n");
		evas_object_del(surface);
		return;
	}

	/**
	 * @note
	 * After swallow this widget object to the layout,
	 * It will be automatically resized by EDJE.
	 */
	elm_object_part_content_set(info->layout, "pepper,widget", surface);
}

EAPI int widget_viewer_evas_init(Evas_Object *win)
{
	if (!is_widget_feature_enabled()) {
		return WIDGET_ERROR_NOT_SUPPORTED;
	}

	s_info.win = win;

	/**
	 * @todo
	 * What should I set to initialize widget subsystem??
	 */
	widget_instance_init("WHAT I HAVE TO SET FOR THIS?");

	set_runtime_dir();

	s_info.compositor_name = pepper_efl_compositor_create(win, NULL);
	if (!s_info.compositor_name) {
		return WIDGET_ERROR_FAULT;
	}

	if (setenv("WAYLAND_DISPLAY", s_info.compositor_name, 1) < 0) {
		ErrPrint("Failed to set WAYLAND_DISPLAY: %s\n", strerror(errno));
	}

	evas_object_smart_callback_add(win, PEPPER_EFL_OBJ_ADD, object_added_cb, NULL);
	evas_object_smart_callback_add(win, PEPPER_EFL_OBJ_DEL, object_deleted_cb, NULL);

	s_info.widget_table = g_hash_table_new(g_str_hash, g_str_equal);

	return WIDGET_ERROR_NONE;
}

EAPI int widget_viewer_evas_fini(void)
{
	if (!is_widget_feature_enabled()) {
		return WIDGET_ERROR_NOT_SUPPORTED;
	}

	if (s_info.win && s_info.compositor_name) {
		pepper_efl_compositor_destroy(s_info.compositor_name);
		s_info.compositor_name = NULL;
	}

	widget_instance_fini();

	return WIDGET_ERROR_NONE;
}

EAPI int widget_viewer_evas_notify_resumed_status_of_viewer(void)
{
	if (!is_widget_feature_enabled()) {
		return WIDGET_ERROR_NOT_SUPPORTED;
	}

	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_notify_paused_status_of_viewer(void)
{
	if (!is_widget_feature_enabled()) {
		return WIDGET_ERROR_NOT_SUPPORTED;
	}

	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_notify_orientation_of_viewer(int orientation)
{
	if (!is_widget_feature_enabled()) {
		return WIDGET_ERROR_NOT_SUPPORTED;
	}

	return WIDGET_ERROR_NOT_SUPPORTED;
}

static void del_cb(void *data, Evas *e, Evas_Object *layout, void *event_info)
{
	struct widget_info *info = data;

	if (info->widget_id && info->instance_id) {
		widget_instance_terminate(info->widget_id, info->instance_id);
		info->pid = 0;
	}

	if (info->permanent_delete) {
		widget_instance_destroy(info->widget_id, info->instance_id);
	}

	evas_object_data_del(layout, WIDGET_INFO_TAG);

	info->layout = NULL;
	free(info->widget_id);
	free(info->instance_id);
	free(info->content_info);
	bundle_free(info->b);
	free(info);
}

static void resize_cb(void *data, Evas *e, Evas_Object *layout, void *event_info)
{
	struct widget_info *info = data;
	int x, y, w, h;

	evas_object_geometry_get(layout, &x, &y, &w, &h);

	if (info->pid == 0) {
		/**
		 * @note
		 * Create a new instance in this case.
		 */
		info->pid = widget_instance_launch(info->widget_id, info->instance_id, info->b, w, h);
		if (info->pid < 0) {
			ErrPrint("Failed to launch an widget\n");
			return;
		}
	} else {
		/**
		 * @note
		 * Layout will be resized, consequently, the pepper object also will be resized.
		 */
	}
}

static inline struct widget_info *create_info(Evas_Object *parent, const char *widget_id, const char *instance_id, const char *content_info)
{
	struct widget_info *info;

	info = (struct widget_info *)malloc(sizeof(*info));
	if (!info) {
		ErrPrint("malloc: %s\n", strerror(errno));
		return NULL;
	}

	info->widget_id = strdup(widget_id);
	if (!info->widget_id) {
		free(info);
		return NULL;
	}

	info->instance_id = strdup(instance_id);
	if (!info->instance_id) {
		free(info->widget_id);
		free(info);
		return NULL;
	}

	info->content_info = strdup(content_info);
	if (!info->content_info) {
		free(info->instance_id);
		free(info->widget_id);
		free(info);
		return NULL;
	}

	info->layout = elm_layout_add(parent);
	if (!info->layout) {
		free(info->content_info);
		free(info->instance_id);
		free(info->widget_id);
		free(info);
		return NULL;
	}

	if (elm_layout_file_set(info->layout, WIDGET_VIEWER_EVAS_RESOURCE_EDJ, "layout") == EINA_FALSE) {
		evas_object_del(info->layout);
		free(info->content_info);
		free(info->instance_id);
		free(info->widget_id);
		free(info);
		return NULL;
	}

	evas_object_data_set(info->layout, WIDGET_INFO_TAG, info);

	evas_object_event_callback_add(info->layout, EVAS_CALLBACK_RESIZE, resize_cb, info);
	evas_object_event_callback_add(info->layout, EVAS_CALLBACK_DEL, del_cb, info);

	info->permanent_delete = 0;

	return info;
}

EAPI Evas_Object *widget_viewer_evas_add_widget(Evas_Object *parent, const char *widget_id, const char *content_info, double period)
{
	char *instance_id = NULL;
	bundle *b = NULL;
	struct widget_info *info = NULL;
	const bundle_raw *bundle_info = NULL;

	if (!is_widget_feature_enabled()) {
		ErrPrint("Widget Feature is disabled\n");
		return NULL;
	}

	if (content_info) {
		bundle_info = (bundle_raw *) content_info;
		b = bundle_decode(bundle_info, strlen(content_info));
		if (b == NULL) {
			ErrPrint("Invalid content format: [%s]\n", content_info);
		}
	}

	if (b) {
		bundle_get_str(b, WIDGET_K_INSTANCE, &instance_id);
	}

	if (!instance_id) {
		if (widget_instance_create(widget_id, &instance_id) < 0) {
			if (b) {
				bundle_free(b);
			}
			return NULL;
		}

		if (!instance_id) {
			ErrPrint("Failed to get instance_id: %s\n", widget_id);
			widget_instance_destroy(widget_id, instance_id);
			if (b) {
				bundle_free(b);
			}
			return NULL;
		}

		info = create_info(parent, widget_id, instance_id, content_info);
		if (!info) {
			ErrPrint("Unable to create an information object\n");
			widget_instance_destroy(widget_id, instance_id);
			if (b) {
				bundle_free(b);
			}
			return NULL;
		}

		info->b = b;
		info->pid = 0;

		g_hash_table_insert(s_info.widget_table, instance_id, info);
	} else {
		info = g_hash_table_lookup(s_info.widget_table, instance_id);
		if (!info) {
			info = create_info(parent, widget_id, instance_id, content_info);
			if (!info) {
				if (b) {
					bundle_free(b);
				}
				return NULL;
			}

			info->b = b;
			info->pid = 0;

			g_hash_table_insert(s_info.widget_table, instance_id, info);
		}
	}

	/**
	 * @note
	 * In this case, user(developer) can get a same object using this API.
	 * If he knows the widget_id and instance_id, he can get the object of it.
	 * Same Evas_Object.
	 */
	return info->layout;
}

EAPI int widget_viewer_evas_set_option(widget_evas_conf_e type, int value)
{
	if (!is_widget_feature_enabled()) {
		return WIDGET_ERROR_NOT_SUPPORTED;
	}
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_pause_widget(Evas_Object *widget)
{
	if (!is_widget_feature_enabled()) {
		return WIDGET_ERROR_NOT_SUPPORTED;
	}
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_resume_widget(Evas_Object *widget)
{
	if (!is_widget_feature_enabled()) {
		return WIDGET_ERROR_NOT_SUPPORTED;
	}
	return WIDGET_ERROR_NOT_SUPPORTED;
}

static int foreach_cb(widget_instance_h handle, void *data)
{
	struct widget_info *info = data;
	bundle_raw *content_info = NULL;
	int content_len = 0;
	bundle *b = NULL;

	if (!handle) {
		return 0;
	}

	if (widget_instance_get_content(handle, &b) < 0 || b == NULL) {
		return 0;
	}

	if (bundle_encode(b, &content_info, &content_len) < 0) {
		return 0;
	}

	free(info->content_info);
	info->content_info = (char *)content_info;
	return 0;
}

EAPI const char *widget_viewer_evas_get_content_info(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		return NULL;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		return NULL;
	}

	widget_instance_foreach(info->widget_id, foreach_cb, info);

	return info->content_info;
}

EAPI const char *widget_viewer_evas_get_title_string(Evas_Object *widget)
{
	if (!is_widget_feature_enabled()) {
		return NULL;
	}

	// WIDGET_ERROR_NOT_SUPPORTED;
	return NULL;
}

EAPI const char *widget_viewer_evas_get_widget_id(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		return NULL;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		return NULL;
	}

	return info->widget_id;
}

EAPI double widget_viewer_evas_get_period(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		return -1.0f;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		return -1.0f;
	}

	return 0.0f;
}

EAPI void widget_viewer_evas_cancel_click_event(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		return;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		return;
	}

	return;
}

EAPI int widget_viewer_evas_feed_mouse_up_event(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		return WIDGET_ERROR_NOT_SUPPORTED;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		return WIDGET_ERROR_INVALID_PARAMETER;
	}
	return WIDGET_ERROR_NONE;
}

/**
 * @note
 * While displaying "loading" overlay, there is a preview image.
 * But if this function is called, the preview image will not be displayed.
 * Only the help text will be there. if it is not disabled of course.
 */
EAPI void widget_viewer_evas_disable_preview(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		return;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		return;
	}

	elm_object_signal_emit(info->layout, "disable", "preview");

	return;
}

/**
 * @note
 * While displaying "loading" overlay, there is a help text.
 * But if this function is called, the text help message will not be displayed.
 * Only the preview image will be there. if it is not disabled of course.
 */
EAPI void widget_viewer_evas_disable_overlay_text(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		return;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		return;
	}

	elm_object_signal_emit(info->layout, "disable", "text");
	return;
}

/**
 * @note
 * This function should be called before resize the widget object after create it.
 * If this function is called, the widget object will not display "preview" image.
 * Instead of it, the widget content object will be displayed immediately.
 */
EAPI void widget_viewer_evas_disable_loading(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		return;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		return;
	}

	elm_object_signal_emit(info->layout, "disable", "loading");
	return;
}

EAPI void widget_viewer_evas_activate_faulted_widget(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		return;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		return;
	}

	if (info->pid < 0) {
		int w;
		int h;

		evas_object_geometry_get(info->layout, NULL, NULL, &w, &h);

		info->pid = widget_instance_launch(info->widget_id, info->instance_id, info->b, w, h);
		if (info->pid < 0) {
			ErrPrint("Failed to launch an widget\n");
			return;
		}
	} else {
		/**
		 * @note
		 * Widget process is running well...
		 */
	}
	return;
}

EAPI bool widget_viewer_evas_is_faulted(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		return false;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		return false;
	}

	return info->pid < 0 ? true : false;
}

EAPI int widget_viewer_evas_freeze_visibility(Evas_Object *widget, widget_visibility_status_e status)
{
	if (!is_widget_feature_enabled()) {
		return WIDGET_ERROR_NOT_SUPPORTED;
	}

	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_thaw_visibility(Evas_Object *widget)
{
	if (!is_widget_feature_enabled()) {
		return WIDGET_ERROR_NOT_SUPPORTED;
	}

	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI bool widget_viewer_evas_is_visibility_frozen(Evas_Object *widget)
{
	if (!is_widget_feature_enabled()) {
		return false;
	}

	return false;
}

EAPI bool widget_viewer_evas_is_widget(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		return false;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		return false;
	}
	return true;
}

EAPI void widget_viewer_evas_set_permanent_delete(Evas_Object *widget, int flag)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		return;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		return;
	}

	info->permanent_delete = 1;
	return;
}

/* End of a file */
