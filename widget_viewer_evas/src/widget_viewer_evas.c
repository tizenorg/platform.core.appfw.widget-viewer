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
#include <cynara-client.h>
#include <fcntl.h>

#include <widget_errno.h>

#include <widget_instance.h>
#include <widget_viewer.h>
#include <compositor.h>
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
#define WIDGET_VIEWER_EVAS_RESOURCE_EDJ "/opt/share/widget_viewer_evas/res/edje/widget_viewer_evas.edj"
#endif

#if !defined(WIDGET_VIEWER_EVAS_RESOURCE_OVERLAY_LOADING)
#define WIDGET_VIEWER_EVAS_RESOURCE_OVERLAY_LOADING "overlay"
#endif

#if !defined(WIDGET_VIEWER_EVAS_RESOURCE_DEFAULT_IMG)
#define WIDGET_VIEWER_EVAS_RESOURCE_DEFAULT_IMG "/opt/share/widget_viewer_evas/res/image/unknown.png"
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
	bool initialized;
	Evas_Object *win;
	GHashTable *widget_table;
} s_info = {
	.w = 0,
	.h = 0,
	.initialized = false,
	.win = NULL,
	.widget_table = NULL,
};

struct widget_info {
	char *widget_id;
	char *instance_id;
	char *content_info;
	const char *title;
	bundle *b;
	int pid;
	double period;

	int disable_preview;
	int disable_loading;
	int permanent_delete;
	int visibility_freeze;

	Evas_Object *layout;
};

static inline bool is_widget_feature_enabled(void)
{
	static bool feature = false;
	static bool retrieved = false;
	int ret;

	if (retrieved == true)
		return feature;

	ret = system_info_get_platform_bool(
			"http://tizen.org/feature/shell.appwidget", &feature);
	if (ret != SYSTEM_INFO_ERROR_NONE) {
		ErrPrint("failed to get system info");
		return false;
	}

	retrieved = true;

	return feature;
}

#define SMACK_LABEL_LEN 255
static int __check_privilege(const char *privilege)
{
	cynara *p_cynara;

	int fd = 0;
	int ret = 0;

	char subject_label[SMACK_LABEL_LEN +1] = "";
	char uid[10] = {0, };
	char *client_session = "";

	ret = cynara_initialize(&p_cynara, NULL);
	if (ret != CYNARA_API_SUCCESS)
		return -1;

	fd = open("/proc/self/attr/current", O_RDONLY);
	if (fd < 0) {
		ret = -1;
		goto ERROR;
	}

	ret = read(fd, subject_label, SMACK_LABEL_LEN);
	if (ret < 0) {
		ErrPrint("read is failed");
		close(fd);
		goto ERROR;
	}
	close(fd);

	snprintf(uid, 10, "%d", getuid());

	ret = cynara_check(p_cynara, subject_label, client_session, uid, privilege);
	if (ret != CYNARA_API_ACCESS_ALLOWED) {
		ret = -1;
		goto ERROR;
	}

	ret = 0;

ERROR:
	if (p_cynara)
		cynara_finish(p_cynara);
	return ret;
}

static void smart_callback_call(Evas_Object *obj, const char *signal, void *cbdata)
{
	if (!obj) {
		DbgPrint("widget is deleted, ignore smart callback call\n");
		return;
	}
	evas_object_smart_callback_call(obj, signal, cbdata);
}

static void widget_object_cb(const char *instance_id, const char *event, Evas_Object *obj, void *data)
{
	struct widget_info *info;
	struct widget_evas_event_info event_info;
	Evas_Object *surface;

	if (!event) {
		ErrPrint("invalid parameters\n");
		return;
	}

	surface = obj;
	if (!surface) {
		ErrPrint("Invalid parameters\n");
		return;
	}

	info = g_hash_table_lookup(s_info.widget_table, instance_id);
	if (!info) {
		ErrPrint("Unable to find a proper object\n");
		evas_object_del(surface);
		return;
	}

	if (strcmp(event, "added") == 0) {
		DbgPrint("widget added: %s", instance_id);
		elm_object_part_content_set(info->layout, "pepper,widget", surface);

		if (!info->disable_preview)
			elm_object_signal_emit(info->layout, "disable", "preview");

		if (!info->disable_loading)
			elm_object_signal_emit(info->layout, "disable", "text");

		event_info.error = WIDGET_ERROR_NONE;
		event_info.widget_app_id = info->widget_id;
		event_info.event = WIDGET_EVENT_CREATED;

		smart_callback_call(info->layout, WIDGET_SMART_SIGNAL_WIDGET_CREATED, &event_info);
		/**
		 * @note
		 * After swallow this widget object to the layout,
		 * It will be automatically resized by EDJE.
		 */
	} else if (strcmp(event, "removed") == 0) {
		DbgPrint("widget removed: %s", instance_id);
		elm_object_part_content_set(info->layout, "pepper,widget", NULL);

		if (info->disable_preview)
			elm_object_signal_emit(info->layout, "enable", "previewe");

		if (info->disable_loading)
			elm_object_signal_emit(info->layout, "enable", "text");

		event_info.error =  WIDGET_ERROR_NONE;
		event_info.widget_app_id = info->widget_id;
		event_info.event = WIDGET_EVENT_DELETED;

		smart_callback_call(info->layout, WIDGET_SMART_SIGNAL_WIDGET_DELETED, &event_info);
	} else {
		ErrPrint("undefiend event occured");
		return;
	}

}

static int instance_event_cb(const char *widget_id, const char *instance_id, int event, void *data)
{
	struct widget_info *info;
	struct widget_evas_event_info event_info;
	const char *smart_signal;

	info = g_hash_table_lookup(s_info.widget_table, instance_id);
	if (!info) {
		ErrPrint("Unable to find a proper object\n");
		return -1;
	}

	DbgPrint("update: %s (%d)", instance_id, event);

	event_info.error = WIDGET_ERROR_NONE;
	event_info.widget_app_id = info->widget_id;

	switch (event) {
	case WIDGET_INSTANCE_EVENT_UPDATE:
		event_info.event = WIDGET_EVENT_WIDGET_UPDATED;
		smart_signal = WIDGET_SMART_SIGNAL_UPDATED;
		break;
	case WIDGET_INSTANCE_EVENT_PERIOD_CHANGED:
		event_info.event = WIDGET_EVENT_PERIOD_CHANGED;
		smart_signal = WIDGET_SMART_SIGNAL_PERIOD_CHANGED;
		break;
	case WIDGET_INSTANCE_EVENT_SIZE_CHANGED:
		event_info.event = WIDGET_EVENT_WIDGET_SIZE_CHANGED;
		smart_signal = WIDGET_SMART_SIGNAL_WIDGET_RESIZED;
		break;
	case WIDGET_INSTANCE_EVENT_EXTRA_UPDATED:
		event_info.event = WIDGET_EVENT_EXTRA_INFO_UPDATED;
		smart_signal = WIDGET_SMART_SIGNAL_EXTRA_INFO_UPDATED;
		break;
	case WIDGET_INSTANCE_EVENT_FAULT:
		event_info.event = WIDGET_FAULT_DEACTIVATED;
		smart_signal = WIDGET_SMART_SIGNAL_WIDGET_FAULTED;
		info->pid = -1;
		break;

	default:
		/* unhandled event */
		return 0;
	}

	if (info->layout)
		smart_callback_call(info->layout, smart_signal, &event_info);
	else
		ErrPrint("object is not ready");

	return 0;
}

EAPI int widget_viewer_evas_init(Evas_Object *win)
{
	char app_id[255];

	if (!is_widget_feature_enabled())
		return WIDGET_ERROR_NOT_SUPPORTED;

	if (__check_privilege("http://tizen.org/privilege/widget.viewer") < 0)
		return WIDGET_ERROR_PERMISSION_DENIED;

	if (!win) {
		ErrPrint("win object is invalid\n");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	if (!bindtextdomain(PKGNAME, WIDGET_VIEWER_EVAS_RESOURCE_PO)) {
		ErrPrint("bindtextdomain: %d\n", errno);
	} else {
		DbgPrint("%s - %s\n", PKGNAME, WIDGET_VIEWER_EVAS_RESOURCE_PO);
	}

	s_info.win = win;

	_compositor_init(win); /* YOU MUST CALL _compositor_init() PRIOR TO widget_instance_init() */

	if (aul_app_get_appid_bypid(getpid(), app_id, sizeof(app_id)) != AUL_R_OK) {
		ErrPrint("failed to get appid of pid:%d", getpid());
		return -1;
	}

	widget_instance_init(app_id);
	widget_instance_listen_event(instance_event_cb, NULL);

	s_info.initialized = true;

	s_info.widget_table = g_hash_table_new(g_str_hash, g_str_equal);

	return WIDGET_ERROR_NONE;
}

EAPI int widget_viewer_evas_fini(void)
{
	if (!is_widget_feature_enabled())
		return WIDGET_ERROR_NOT_SUPPORTED;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return WIDGET_ERROR_FAULT;
	}

	_compositor_fini();
	widget_instance_unlisten_event(instance_event_cb);
	widget_instance_fini();

	s_info.initialized = false;

	return WIDGET_ERROR_NONE;
}

EAPI int widget_viewer_evas_notify_resumed_status_of_viewer(void)
{
	if (!is_widget_feature_enabled())
		return WIDGET_ERROR_NOT_SUPPORTED;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return WIDGET_ERROR_FAULT;
	}

	return WIDGET_ERROR_NONE;
}

EAPI int widget_viewer_evas_notify_paused_status_of_viewer(void)
{
	if (!is_widget_feature_enabled())
		return WIDGET_ERROR_NOT_SUPPORTED;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return WIDGET_ERROR_FAULT;
	}

	return WIDGET_ERROR_NONE;
}

EAPI int widget_viewer_evas_notify_orientation_of_viewer(int orientation)
{
	if (!is_widget_feature_enabled())
		return WIDGET_ERROR_NOT_SUPPORTED;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return WIDGET_ERROR_FAULT;
	}

	if (orientation < 0 || orientation > 360) {
		ErrPrint("orientation is invalid parameter\n");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	return WIDGET_ERROR_NONE;
}

static void del_cb(void *data, Evas *e, Evas_Object *layout, void *event_info)
{
	struct widget_info *info = data;
	struct widget_evas_event_info evas_info;

	evas_info.error = WIDGET_ERROR_NONE;
	evas_info.widget_app_id = info->widget_id;
	evas_info.event = WIDGET_EVENT_CREATED;

	smart_callback_call(info->layout, WIDGET_SMART_SIGNAL_WIDGET_DELETED, &evas_info);

	if (info->widget_id && info->instance_id) {
		widget_instance_terminate(info->widget_id, info->instance_id);
		info->pid = 0;
	}

	if (info->permanent_delete)
		widget_instance_destroy(info->widget_id, info->instance_id);

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
	Evas_Object *preview = NULL;
	struct widget_info *info = data;
	char *preview_path = NULL;
	int x, y, w, h;
	widget_size_type_e size_type;

	if (!info || !layout) {
		ErrPrint("Failed to load the info(%p) or layout(%p)\n", info, layout);
		return;
	}

	evas_object_geometry_get(layout, &x, &y, &w, &h);

	if (info->pid == 0) {
		/**
		 * @note
		 * Create a new instance in this case.
		 */

		if (!info->disable_preview) {
			widget_service_get_size_type(w, h, &size_type);
			preview_path = widget_service_get_preview_image_path(info->widget_id, size_type);
			if (preview_path) {
				preview = elm_image_add(layout);
				if (preview) {
					elm_image_file_set(preview, preview_path, NULL);
					elm_object_part_content_set(layout, "preview", preview);
				}

				free(preview_path);

			}

		}

		if (!info->disable_loading) {
			elm_object_part_text_set(layout, "text", T_("IDS_ST_POP_LOADING_ING"));
		}

		_compositor_set_handler(info->instance_id, widget_object_cb, NULL);
		info->pid = widget_instance_launch(info->widget_id, info->instance_id, info->b, w, h);
		if (info->pid < 0) {
			struct widget_evas_event_info event_info;
			ErrPrint("Failed to launch an widget\n");
			event_info.error = info->pid;
			event_info.widget_app_id = info->widget_id;
			event_info.event = WIDGET_EVENT_CREATED;

			smart_callback_call(info->layout, WIDGET_SMART_SIGNAL_WIDGET_CREATE_ABORTED, &event_info);
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

	info = (struct widget_info *)calloc(1, sizeof(*info));
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

	if (content_info) {
		info->content_info = strdup(content_info);
		if (!info->content_info) {
			free(info->instance_id);
			free(info->widget_id);
			free(info);
			return NULL;
		}
	}

	info->layout = elm_layout_add(parent);
	if (!info->layout) {
		if (info->content_info)
			free(info->content_info);

		free(info->instance_id);
		free(info->widget_id);
		free(info);
		return NULL;
	}

	if (elm_layout_file_set(info->layout, WIDGET_VIEWER_EVAS_RESOURCE_EDJ, "layout") == EINA_FALSE) {
		evas_object_del(info->layout);

		if (info->content_info)
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
	info->disable_preview = 0;
	info->disable_loading = 0;
	info->visibility_freeze = 0;

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

	if (!s_info.initialized) {
		ErrPrint("Widget viewer evas is not initialized\n");
		return NULL;
	}

	if (!parent) {
		ErrPrint("parent(window) object is invalid\n");
		return NULL;
	}

	if (!widget_id) {
		ErrPrint("widget package id is invalid\n");
		return NULL;
	}

	if (content_info) {
		bundle_info = (bundle_raw *) content_info;
		b = bundle_decode(bundle_info, strlen(content_info));
		if (b == NULL) {
			ErrPrint("Invalid content format: [%s]\n", content_info);
		}
	}

	if (b)
		bundle_get_str(b, WIDGET_K_INSTANCE, &instance_id);

	if (!instance_id) {
		if (widget_instance_create(widget_id, &instance_id) < 0) {
			if (b)
				bundle_free(b);

			return NULL;
		}

		if (!instance_id) {
			ErrPrint("Failed to get instance_id: %s\n", widget_id);
			if (b)
				bundle_free(b);

			return NULL;
		}

		info = create_info(parent, widget_id, instance_id, content_info);
		if (!info) {
			ErrPrint("Unable to create an information object\n");
			widget_instance_destroy(widget_id, instance_id);
			if (b)
				bundle_free(b);

			return NULL;
		}

		info->b = b;
		info->pid = 0;
		info->period = period;

		g_hash_table_insert(s_info.widget_table, instance_id, info);
	} else {
		info = g_hash_table_lookup(s_info.widget_table, instance_id);
		if (!info) {
			info = create_info(parent, widget_id, instance_id, content_info);
			if (!info) {
				if (b)
					bundle_free(b);
				return NULL;
			}

			info->b = b;
			info->pid = 0;
			info->period = period;

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
	if (!is_widget_feature_enabled())
		return WIDGET_ERROR_NOT_SUPPORTED;

	if (type < WIDGET_VIEWER_EVAS_MANUAL_PAUSE_RESUME || type > WIDGET_VIEWER_EVAS_UNKNOWN) {
		ErrPrint("type is invalid\n");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	return WIDGET_ERROR_NONE;
}

EAPI int widget_viewer_evas_pause_widget(Evas_Object *widget)
{
	struct widget_info *info;
	int ret = WIDGET_ERROR_NONE;

	if (!is_widget_feature_enabled())
		return WIDGET_ERROR_NOT_SUPPORTED;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return WIDGET_ERROR_FAULT;
	}

	if (!widget) {
		ErrPrint("widget object is invalid\n");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info\n", widget);
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	if (info->visibility_freeze) {
		ErrPrint("widget(%p) is freezing visibility(%d)\n", widget, info->visibility_freeze);
		return WIDGET_ERROR_DISABLED;
	}

	ret = widget_instance_pause(info->widget_id, info->instance_id);
	if (ret < 0) {
		ErrPrint("Fail to pause the widget(%p):(%d)\n", widget, ret);
		return ret;
	}

	return WIDGET_ERROR_NONE;
}

EAPI int widget_viewer_evas_resume_widget(Evas_Object *widget)
{
	struct widget_info *info;
	int ret = WIDGET_ERROR_NONE;

	if (!is_widget_feature_enabled())
		return WIDGET_ERROR_NOT_SUPPORTED;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return WIDGET_ERROR_FAULT;
	}

	if (!widget) {
		ErrPrint("widget object is invalid\n");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info\n", widget);
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	if (info->visibility_freeze) {
		ErrPrint("widget(%p) is freezing visibility(%d)\n", widget, info->visibility_freeze);
		return WIDGET_ERROR_DISABLED;
	}

	ret = widget_instance_resume(info->widget_id, info->instance_id);
	if (ret < 0) {
		ErrPrint("Fail to resume the widget(%p):(%d)\n", widget, ret);
		return ret;
	}

	return WIDGET_ERROR_NONE;
}

EAPI const char *widget_viewer_evas_get_content_info(Evas_Object *widget)
{
	struct widget_info *info;
	widget_instance_h handle = NULL;
	bundle_raw *content_info = NULL;
	int content_len = 0;
	bundle *b = NULL;


	if (!is_widget_feature_enabled())
		return NULL;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return NULL;
	}

	if (!widget) {
		ErrPrint("widget object is invalid\n");
		return NULL;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info\n", widget);
		return NULL;
	}

	if (!info->widget_id && !info->instance_id) {
		ErrPrint("widget id(%s) or instance id(%s) is invalid data\n", info->widget_id, info->instance_id);
		return NULL;
	}

	handle = widget_instance_get_instance(info->widget_id, info->instance_id);

	if (!handle) {
		ErrPrint("instance handle of widget(%s) is invalid data\n", info->instance_id);
		return NULL;
	}

	if (widget_instance_get_content(handle, &b) < 0) {
		ErrPrint("Failed to get content of widget(%s)\n", info->instance_id);
		return NULL;
	}

	if (b == NULL) {
		ErrPrint("content of widget(%s) is invalid data\n", info->instance_id);
		return NULL;
	}

	if (bundle_encode(b, &content_info, &content_len) < 0) {
		ErrPrint("Failed to encode (%s)\n", info->instance_id);
		return NULL;
	}

	if (info->content_info)
		free(info->content_info);
	info->content_info = (char *)content_info;


	return info->content_info;
}

EAPI const char *widget_viewer_evas_get_title_string(Evas_Object *widget)
{
	Evas_Object *pepper_obj = NULL;
	const char *title = NULL;
	struct widget_info *info;

	if (!is_widget_feature_enabled())
		return NULL;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return NULL;
	}

	if (!widget) {
		ErrPrint("widget object is invalid\n");
		return NULL;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info\n", widget);
		return NULL;
	}

	pepper_obj = elm_object_part_content_get(widget, "pepper,widget");
	if (!pepper_obj) {
		ErrPrint("widget object is invalid\n");
		return NULL;
	}

	title = pepper_efl_object_title_get(pepper_obj);
	if (!title) {
		//title = widget_service_get_app_id_of_setup_app(info->widget_id);
		title = widget_service_get_name(info->widget_id, NULL);
	}

	info->title = title;

	return title;
}

EAPI const char *widget_viewer_evas_get_widget_id(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled())
		return NULL;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return NULL;
	}

	if (!widget) {
		ErrPrint("widget object is invalid\n");
		return NULL;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info\n", widget);
		return NULL;
	}

	return info->widget_id;
}

EAPI double widget_viewer_evas_get_period(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled())
		return -1.0f;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return -1.0f;
	}

	if (!widget) {
		ErrPrint("widget object is invalid\n");
		return -1.0f;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info\n", widget);
		return -1.0f;
	}

	return info->period;
}

EAPI void widget_viewer_evas_cancel_click_event(Evas_Object *widget)
{
	Evas_Object *pepper_obj = NULL;
	struct widget_info *info;

	if (!is_widget_feature_enabled())
		return;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return;
	}

	if (!widget) {
		ErrPrint("widget object is invalid\n");
		return;
	}


	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info\n", widget);
		return;
	}

	pepper_obj = elm_object_part_content_get(widget, "pepper,widget");
	if (!pepper_obj) {
		ErrPrint("widget object is invalid\n");
		return;
	}

	if (!pepper_efl_object_touch_cancel(pepper_obj)) {
		ErrPrint("Fail to cancel the click event");
		return;
	}

	return;
}

EAPI int widget_viewer_evas_feed_mouse_up_event(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled())
		return WIDGET_ERROR_NOT_SUPPORTED;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return WIDGET_ERROR_FAULT;
	}

	if (!widget) {
		ErrPrint("widget object is invalid\n");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info\n", widget);
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

	if (!is_widget_feature_enabled())
		return;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return;
	}

	if (!widget) {
		ErrPrint("widget object is invalid\n");
		return;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info\n", widget);
		return;
	}

	info->disable_preview = 1;
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

	if (!is_widget_feature_enabled())
		return;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return;
	}

	if (!widget) {
		ErrPrint("widget object is invalid\n");
		return;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info\n", widget);
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

	if (!is_widget_feature_enabled())
		return;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return;
	}

	if (!widget) {
		ErrPrint("widget object is invalid\n");
		return;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info\n", widget);
		return;
	}

	info->disable_loading = 1;
	elm_object_signal_emit(info->layout, "disable", "text");
	return;
}

EAPI void widget_viewer_evas_activate_faulted_widget(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled())
		return;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return;
	}

	if (!widget) {
		ErrPrint("widget object is invalid\n");
		return;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info\n", widget);
		return;
	}

	if (info->pid < 0) {

		struct widget_evas_event_info event_info;
		int w;
		int h;

		evas_object_geometry_get(info->layout, NULL, NULL, &w, &h);

		if (info->disable_preview)
			elm_object_signal_emit(info->layout, "enable", "preview");

		if (info->disable_loading)
			elm_object_signal_emit(info->layout, "enable", "text");

		info->pid = widget_instance_launch(info->widget_id, info->instance_id, info->b, w, h);
		if (info->pid < 0) {
			ErrPrint("Failed to launch an widget\n");
			event_info.error = info->pid;
			event_info.widget_app_id = info->widget_id;
			event_info.event = WIDGET_EVENT_CREATED;

			smart_callback_call(info->layout, WIDGET_SMART_SIGNAL_WIDGET_CREATE_ABORTED, &event_info);

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

	if (!is_widget_feature_enabled())
		return false;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return false;
	}

	if (!widget) {
		ErrPrint("widget object is invalid\n");
		return false;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info\n", widget);
		return false;
	}

	return info->pid < 0 ? true : false;
}

EAPI int widget_viewer_evas_freeze_visibility(Evas_Object *widget, widget_visibility_status_e status)
{
	struct widget_info *info;
	int ret = 0;

	if (!is_widget_feature_enabled())
		return WIDGET_ERROR_NOT_SUPPORTED;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return WIDGET_ERROR_FAULT;
	}

	if (!widget) {
		ErrPrint("widget object is invalid\n");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info\n", widget);
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	if (status == WIDGET_VISIBILITY_STATUS_SHOW_FIXED) {
		ret = widget_instance_resume(info->widget_id, info->instance_id);
		if (ret < 0) {
			ErrPrint("Fail to resume the widget(%p):(%d)\n", widget, ret);
			return ret;
		}
		info->visibility_freeze = status;
	} else if (status == WIDGET_VISIBILITY_STATUS_HIDE_FIXED) {
		ret = widget_instance_pause(info->widget_id, info->instance_id);
		if (ret < 0) {
			ErrPrint("Fail to pause the widget(%p):(%d)\n", widget, ret);
			return ret;
		}
		info->visibility_freeze = status;
	} else {
		ErrPrint("status value is invalid\n");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	return WIDGET_ERROR_NONE;
}

EAPI int widget_viewer_evas_thaw_visibility(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled())
		return WIDGET_ERROR_NOT_SUPPORTED;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return WIDGET_ERROR_FAULT;
	}

	if (!widget) {
		ErrPrint("widget object is invalid\n");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info\n", widget);
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	info->visibility_freeze = 0;

	return WIDGET_ERROR_NONE;
}

EAPI bool widget_viewer_evas_is_visibility_frozen(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled())
		return false;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return false;
	}

	if (!widget) {
		ErrPrint("widget object is invalid\n");
		return false;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info\n", widget);
		return false;
	}

	if (info->visibility_freeze)
		return true;

	return false;
}

EAPI bool widget_viewer_evas_is_widget(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled())
		return false;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return false;
	}

	if (!widget) {
		ErrPrint("widget object is invalid\n");
		return false;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info\n", widget);
		return false;
	}

	return true;
}

EAPI void widget_viewer_evas_set_permanent_delete(Evas_Object *widget, int flag)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled())
		return;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized\n");
		return;
	}

	if (!widget) {
		ErrPrint("widget object is invalid\n");
		return;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info\n", widget);
		return;
	}

	info->permanent_delete = flag;
	return;
}

/* End of a file */
