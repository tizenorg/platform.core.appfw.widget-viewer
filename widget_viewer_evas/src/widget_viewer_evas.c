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
#include <widget_service_internal.h>
#include <widget_instance.h>
#include <widget_viewer.h>
#include <compositor.h>
#include <Pepper_Efl.h>
#include <aul_app_com.h>

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

enum {
	WIDGET_STATE_DETACHED,
	WIDGET_STATE_ATTACHED
};

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
	const char *title;
	char *content_info;
	int pid;
	double period;

	int disable_preview;
	int disable_loading;
	int disable_overlay;
	int permanent_delete;
	int visibility_freeze;
	int state;
	int cancel_click;
	bool restart;

	GQueue *event_queue;

	Evas_Object *layout;
};

static void __flush_event_queue(struct widget_info *info);

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
		DbgPrint("widget is deleted, ignore smart callback call");
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
		ErrPrint("invalid parameters");
		return;
	}

	surface = obj;
	if (!surface) {
		ErrPrint("Invalid parameters");
		return;
	}

	info = g_hash_table_lookup(s_info.widget_table, instance_id);
	if (!info) {
		ErrPrint("Unable to find a proper object");
		evas_object_del(surface);
		return;
	}

	if (info->restart)
		return;

	if (strcmp(event, "added") == 0) {
		int x, y, w, h;
		DbgPrint("widget added: %s", instance_id);
		evas_object_geometry_get(surface, &x, &y, &w, &h);
		DbgPrint("widget geometry:%d %d %d %d", x, y, w, h);
		elm_object_part_content_set(info->layout, "pepper,widget", surface);

		if (!info->disable_preview)
			elm_object_signal_emit(info->layout, "disable", "preview");

		if (!info->disable_loading)
			elm_object_signal_emit(info->layout, "disable", "loading,text");

		info->state = WIDGET_STATE_ATTACHED;

		__flush_event_queue(info);
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
			elm_object_signal_emit(info->layout, "enable", "loading,text");

		event_info.error =  WIDGET_ERROR_NONE;
		event_info.widget_app_id = info->widget_id;
		event_info.event = WIDGET_EVENT_DELETED;

		smart_callback_call(info->layout, WIDGET_SMART_SIGNAL_WIDGET_DELETED, &event_info);
	} else {
		ErrPrint("undefiend event occured");
		return;
	}
}

static void __display_overlay_text(struct widget_info *info)
{
	if (!info) {
		ErrPrint("Unable to get the info");
		return;
	}
	if (!info->disable_overlay) {
		elm_object_part_text_set(info->layout, "text", T_("IDS_HS_BODY_UNABLE_TO_LOAD_DATA_TAP_TO_RETRY"));
		elm_object_signal_emit(info->layout, "enable", "overlay,text");
		elm_object_signal_emit(info->layout, "disable", "preview");
	}
}

static void __push_event_queue(struct widget_info *info, int event)
{
	if (info->event_queue == NULL)
		info->event_queue = g_queue_new();

	if (info->event_queue == NULL) {
		ErrPrint("out of memory");
		return;
	}

	DbgPrint("add event to queue:%d", event);
	g_queue_push_tail(info->event_queue, GINT_TO_POINTER(event));
}

static int __restart_terminated_widget(const char *widget_id)
{

	GHashTableIter iter;
	gpointer key, value;
	struct widget_info *widget_instance_info;
	int w, h;
	int target_pid = 0;

	g_hash_table_iter_init(&iter, s_info.widget_table);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		widget_instance_info = (struct widget_info *)value;
		if (widget_instance_info->restart &&
				(strcmp(widget_instance_info->widget_id, widget_id) == 0)) {
			target_pid = widget_instance_info->pid;
			break;
		}
	}

	if (target_pid == 0) {
		DbgPrint("can not find widget");
		return 0;
	}

	g_hash_table_iter_init(&iter, s_info.widget_table);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		widget_instance_info = (struct widget_info *)value;
		if (widget_instance_info->pid == target_pid) {
			evas_object_geometry_get(widget_instance_info->layout, NULL, NULL, &w, &h);
			DbgPrint("Widget launch  %s, %d, %d", widget_instance_info->instance_id, w, h);
			widget_instance_info->pid = widget_instance_launch(widget_instance_info->instance_id, widget_instance_info->content_info, w, h);
			widget_instance_info->restart = false;
		}
	}

	return 0;
}

static int __handle_restart_widget_request(const char *widget_id)
{
	GHashTableIter iter;
	gpointer key, value;
	struct widget_info *widget_instance_info;
	int target_pid = 0;

	g_hash_table_iter_init(&iter, s_info.widget_table);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		widget_instance_info = (struct widget_info *)value;
		if (strcmp(widget_instance_info->widget_id, widget_id) == 0) {
			target_pid = widget_instance_info->pid;
			break;
		}
	}

	if (target_pid == 0) {
		DbgPrint("can not find widget");
		return 0;
	}

	g_hash_table_iter_init(&iter, s_info.widget_table);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		widget_instance_info = (struct widget_info *)value;
		if (widget_instance_info->pid == target_pid)
			widget_instance_info->restart = true;
	}
	aul_terminate_pid_async(target_pid);
	DbgPrint("Widget __handle_restart_widget_request id : %s, pid : %d", widget_id, target_pid);

	return 0;
}

static int __instance_event_cb(const char *widget_id, const char *instance_id, int event, void *data)
{
	struct widget_info *info;
	struct widget_evas_event_info event_info;
	const char *smart_signal = NULL;
	char *content_info;
	widget_instance_h handle;

	DbgPrint("widget_id : %s (%d)", widget_id, event);
	if (event == WIDGET_INSTANCE_EVENT_APP_RESTART_REQUEST) {
		__handle_restart_widget_request(widget_id);
		return 0;
	}

	info = g_hash_table_lookup(s_info.widget_table, instance_id);
	if (!info) {
		ErrPrint("Unable to find a proper object");
		return -1;
	}

	if (info->state == WIDGET_STATE_DETACHED) {
		__push_event_queue(info, event);
		return 0;
	}

	DbgPrint("update: %s (%d)", instance_id, event);

	event_info.error = WIDGET_ERROR_NONE;
	event_info.widget_app_id = info->widget_id;

	switch (event) {
	case WIDGET_INSTANCE_EVENT_CREATE:
		event_info.event = WIDGET_EVENT_CREATED;
		smart_signal = WIDGET_SMART_SIGNAL_WIDGET_CREATED;
		break;
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
		handle = widget_instance_get_instance(info->widget_id, info->instance_id);
		if (!handle) {
			ErrPrint("instance handle of widget(%s) is invalid data", info->instance_id);
			break;
		}

		if (widget_instance_get_content(handle, &content_info) < 0) {
			ErrPrint("Failed to get content of widget(%s)", info->instance_id);
			break;
		}

		if (content_info) {
			if (info->content_info)
				free(info->content_info);

			info->content_info = strdup(content_info);
		}
		break;
	case WIDGET_INSTANCE_EVENT_FAULT:
		event_info.event = WIDGET_FAULT_DEACTIVATED;
		smart_signal = WIDGET_SMART_SIGNAL_WIDGET_FAULTED;
		info->pid = -1;
		info->state = WIDGET_STATE_DETACHED;
		__display_overlay_text(info);
		break;
	default:
		/* unhandled event */
		return 0;
	}

	if (!info->layout) {
		ErrPrint("object is not ready");
		return 0;
	}

	smart_callback_call(info->layout, smart_signal, &event_info);

	return 0;
}

static int __lifecycle_event_cb(const char *widget_id, widget_lifecycle_event_e lifecycle_event, const char *widget_instance_id, void *data)
{
	int ret;
	DbgPrint("__lifecycle_event_cb %s (%d)", widget_id, lifecycle_event);
	if (lifecycle_event == WIDGET_LIFE_CYCLE_EVENT_APP_DEAD) {
		ret = __restart_terminated_widget(widget_id);
		return ret;
	}
	return 0;
}

static void __flush_event_queue(struct widget_info *info)
{
	if (!info->event_queue) {
		DbgPrint("event queue is empty. nothing to flush");
		return;
	}

	while (!g_queue_is_empty(info->event_queue)) {
		int event = GPOINTER_TO_INT(g_queue_pop_head(info->event_queue));
		DbgPrint("call pending event %d", event);
		__instance_event_cb(info->widget_id, info->instance_id, event, NULL);
	}

	g_queue_free(info->event_queue);
	info->event_queue = NULL;
}

EAPI int widget_viewer_evas_init(Evas_Object *win)
{
	char app_id[255];

	if (!is_widget_feature_enabled())
		return WIDGET_ERROR_NOT_SUPPORTED;

	if (__check_privilege("http://tizen.org/privilege/widget.viewer") < 0)
		return WIDGET_ERROR_PERMISSION_DENIED;

	if (!win) {
		ErrPrint("win object is invalid");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	if (!bindtextdomain(PKGNAME, WIDGET_VIEWER_EVAS_RESOURCE_PO)) {
		ErrPrint("bindtextdomain: %d", errno);
	} else {
		DbgPrint("%s - %s", PKGNAME, WIDGET_VIEWER_EVAS_RESOURCE_PO);
	}

	s_info.win = win;

	_compositor_init(win); /* YOU MUST CALL _compositor_init() PRIOR TO widget_instance_init() */
	_compositor_start_visibility_notify();

	if (aul_app_get_appid_bypid(getpid(), app_id, sizeof(app_id)) != AUL_R_OK) {
		ErrPrint("failed to get appid of pid:%d", getpid());
		return -1;
	}

	widget_instance_init(app_id);
	widget_instance_listen_event(__instance_event_cb, NULL);

	s_info.initialized = true;

	s_info.widget_table = g_hash_table_new(g_str_hash, g_str_equal);

	return WIDGET_ERROR_NONE;
}

EAPI int widget_viewer_evas_fini(void)
{
	if (!is_widget_feature_enabled())
		return WIDGET_ERROR_NOT_SUPPORTED;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized");
		return WIDGET_ERROR_FAULT;
	}

	_compositor_fini();
	widget_instance_unlisten_event(__instance_event_cb);
	widget_instance_fini();

	s_info.initialized = false;

	return WIDGET_ERROR_NONE;
}

EAPI int widget_viewer_evas_notify_resumed_status_of_viewer(void)
{
	if (!is_widget_feature_enabled())
		return WIDGET_ERROR_NOT_SUPPORTED;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized");
		return WIDGET_ERROR_FAULT;
	}

	return WIDGET_ERROR_NONE;
}

EAPI int widget_viewer_evas_notify_paused_status_of_viewer(void)
{
	if (!is_widget_feature_enabled())
		return WIDGET_ERROR_NOT_SUPPORTED;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized");
		return WIDGET_ERROR_FAULT;
	}

	return WIDGET_ERROR_NONE;
}

EAPI int widget_viewer_evas_notify_orientation_of_viewer(int orientation)
{
	if (!is_widget_feature_enabled())
		return WIDGET_ERROR_NOT_SUPPORTED;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized");
		return WIDGET_ERROR_FAULT;
	}

	if (orientation < 0 || orientation > 360) {
		ErrPrint("orientation is invalid parameter");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	return WIDGET_ERROR_NONE;
}

static void del_cb(void *data, Evas *e, Evas_Object *layout, void *event_info)
{
	struct widget_info *info = data;
	struct widget_evas_event_info evas_info;

	if (info->restart)
		return;

	DbgPrint("delete: layout(%p)", layout);

	evas_info.error = WIDGET_ERROR_NONE;
	evas_info.widget_app_id = info->widget_id;
	evas_info.event = WIDGET_EVENT_CREATED;

	smart_callback_call(info->layout, WIDGET_SMART_SIGNAL_WIDGET_DELETED, &evas_info);

	if (info->instance_id) {
		if (info->permanent_delete)
			widget_instance_destroy(info->instance_id);
		else
			widget_instance_terminate(info->instance_id);
	}

	evas_object_data_del(layout, WIDGET_INFO_TAG);
	g_hash_table_remove(s_info.widget_table, info->instance_id);

	info->layout = NULL;
	if (info->event_queue) {
		g_queue_free(info->event_queue);
		info->event_queue = NULL;
	}

	free(info->widget_id);
	free(info->instance_id);
	free(info->content_info);
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
		ErrPrint("Failed to load the info(%p) or layout(%p)", info, layout);
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
		info->pid = widget_instance_launch(info->instance_id, info->content_info, w, h);
		if (info->pid < 0) {
			struct widget_evas_event_info event_info;
			ErrPrint("Failed to launch an widget");
			event_info.error = info->pid;
			event_info.widget_app_id = info->widget_id;
			event_info.event = WIDGET_EVENT_CREATED;

			smart_callback_call(info->layout, WIDGET_SMART_SIGNAL_WIDGET_CREATE_ABORTED, &event_info);
			__display_overlay_text(info);
			return;
		}
	} else {
		/**
		 * @note
		 * Layout will be resized, consequently, the pepper object also will be resized.
		 */
	}
}

static void _clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	struct widget_info *info = data;
	if (!info) {
		ErrPrint("Failed to get info data");
		return;
	}
	if (info->cancel_click) {
		info->cancel_click = 0;
		return;
	}
	widget_viewer_evas_activate_faulted_widget(info->layout);
}

static inline struct widget_info *create_info(Evas_Object *parent, const char *widget_id, const char *instance_id, const char *content_info)
{
	struct widget_info *info;

	info = (struct widget_info *)calloc(1, sizeof(*info));
	if (!info) {
		ErrPrint("malloc: %s", strerror(errno));
		goto out;
	}

	info->widget_id = strdup(widget_id);
	if (!info->widget_id) {
		goto out;
	}

	info->instance_id = strdup(instance_id);
	if (!info->instance_id) {
		goto out;
	}

	info->layout = elm_layout_add(parent);
	if (!info->layout) {
		goto out;
	}

	if (elm_layout_file_set(info->layout, WIDGET_VIEWER_EVAS_RESOURCE_EDJ, "layout") == EINA_FALSE) {
		evas_object_del(info->layout);
		goto out;
	}

	evas_object_data_set(info->layout, WIDGET_INFO_TAG, info);

	evas_object_event_callback_add(info->layout, EVAS_CALLBACK_RESIZE, resize_cb, info);
	evas_object_event_callback_add(info->layout, EVAS_CALLBACK_DEL, del_cb, info);
	elm_object_signal_callback_add(info->layout, "clicked", "reload", _clicked_cb, info);

	info->permanent_delete = 0;
	info->disable_preview = 0;
	info->disable_loading = 0;
	info->visibility_freeze = 0;
	info->cancel_click = 0;
	info->state = WIDGET_STATE_DETACHED;
	info->event_queue = NULL;
	info->restart = false;

	return info;

out:
	if (info) {
		if (info->instance_id)
			free(info->instance_id);

		if (info->widget_id)
			free(info->widget_id);

		free(info);
	}

	return NULL;
}

EAPI Evas_Object *widget_viewer_evas_add_widget(Evas_Object *parent, const char *widget_id, const char *content_info, double period)
{
	char *instance_id = NULL;
	struct widget_info *widget_instance_info = NULL;

	if (!is_widget_feature_enabled()) {
		set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
		ErrPrint("Widget Feature is disabled");
		return NULL;
	}

	if (!s_info.initialized) {
		set_last_result(WIDGET_ERROR_FAULT);
		ErrPrint("Widget viewer evas is not initialized");
		return NULL;
	}

	if (!parent) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("parent(window) object is invalid");
		return NULL;
	}

	if (!widget_id) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget package id is invalid");
		return NULL;
	}

	if (widget_instance_create(widget_id, &instance_id) < 0) {
		set_last_result(WIDGET_ERROR_FAULT);
		return NULL;
	}

	if (!instance_id) {
		set_last_result(WIDGET_ERROR_FAULT);
		ErrPrint("Failed to get instance_id: %s", widget_id);
		return NULL;
	}

	widget_instance_info = create_info(parent, widget_id, instance_id, content_info);
	if (!widget_instance_info) {
		set_last_result(WIDGET_ERROR_FAULT);
		ErrPrint("Unable to create an information object");
		widget_instance_destroy(instance_id);
		return NULL;
	}

	if (content_info)
		widget_instance_info->content_info = strdup(content_info);

	widget_instance_info->pid = 0;
	widget_instance_info->period = period;

	g_hash_table_insert(s_info.widget_table, widget_instance_info->instance_id, widget_instance_info);
	widget_service_set_lifecycle_event_cb(widget_id, __lifecycle_event_cb, NULL);

	/**
	 * @note
	 * In this case, user(developer) can get a same object using this API.
	 * If he knows the widget_id and instance_id, he can get the object of it.
	 * Same Evas_Object.
	 */
	return widget_instance_info->layout;
}

EAPI int widget_viewer_evas_set_option(widget_evas_conf_e type, int value)
{
	if (!is_widget_feature_enabled())
		return WIDGET_ERROR_NOT_SUPPORTED;

	if (type < WIDGET_VIEWER_EVAS_MANUAL_PAUSE_RESUME || type > WIDGET_VIEWER_EVAS_UNKNOWN) {
		ErrPrint("type is invalid");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	if (type == WIDGET_VIEWER_EVAS_MANUAL_PAUSE_RESUME) {
		if (value)
			_compositor_stop_visibility_notify();
		else
			_compositor_start_visibility_notify();
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
		ErrPrint("widget viewer evas is not initialized");
		return WIDGET_ERROR_FAULT;
	}

	if (!widget) {
		ErrPrint("widget object is invalid");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info", widget);
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	if (info->visibility_freeze) {
		ErrPrint("widget(%p) is freezing visibility(%d)", widget, info->visibility_freeze);
		return WIDGET_ERROR_DISABLED;
	}

	ret = widget_instance_pause(info->instance_id);
	if (ret < 0) {
		ErrPrint("Fail to pause the widget(%p):(%d)", widget, ret);
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
		ErrPrint("widget viewer evas is not initialized");
		return WIDGET_ERROR_FAULT;
	}

	if (!widget) {
		ErrPrint("widget object is invalid");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info", widget);
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	if (info->visibility_freeze) {
		ErrPrint("widget(%p) is freezing visibility(%d)", widget, info->visibility_freeze);
		return WIDGET_ERROR_DISABLED;
	}

	ret = widget_instance_resume(info->instance_id);
	if (ret < 0) {
		ErrPrint("Fail to resume the widget(%p):(%d)", widget, ret);
		return ret;
	}

	return WIDGET_ERROR_NONE;
}

EAPI const char *widget_viewer_evas_get_content_info(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
		return NULL;
	}

	if (!s_info.initialized) {
		set_last_result(WIDGET_ERROR_FAULT);
		ErrPrint("widget viewer evas is not initialized");

		return NULL;
	}

	if (!widget) {
		ErrPrint("widget object is invalid");
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget(%p) don't have the info", widget);
		return NULL;
	}

	return (const char*)info->content_info;
}

EAPI const char *widget_viewer_evas_get_title_string(Evas_Object *widget)
{
	Evas_Object *pepper_obj = NULL;
	const char *title = NULL;
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
		return NULL;
	}

	if (!s_info.initialized) {
		set_last_result(WIDGET_ERROR_FAULT);
		ErrPrint("widget viewer evas is not initialized");
		return NULL;
	}

	if (!widget) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget object is invalid");
		return NULL;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget(%p) don't have the info", widget);
		return NULL;
	}

	pepper_obj = elm_object_part_content_get(widget, "pepper,widget");
	if (!pepper_obj) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget object is invalid");
		return NULL;
	}

	title = pepper_efl_object_title_get(pepper_obj);
	if (!title) {
		title = widget_service_get_name(info->widget_id, NULL);
	}

	info->title = title;

	return title;
}

EAPI const char *widget_viewer_evas_get_widget_id(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		return NULL;
	}

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized");
		return NULL;
	}

	if (!widget) {
		ErrPrint("widget object is invalid");
		return NULL;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info", widget);
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
		ErrPrint("widget viewer evas is not initialized");
		return -1.0f;
	}

	if (!widget) {
		ErrPrint("widget object is invalid");
		return -1.0f;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info", widget);
		return -1.0f;
	}

	return info->period;
}

EAPI void widget_viewer_evas_cancel_click_event(Evas_Object *widget)
{
	Evas_Object *pepper_obj = NULL;
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
		return;
	}

	if (!s_info.initialized) {
		set_last_result(WIDGET_ERROR_FAULT);
		ErrPrint("widget viewer evas is not initialized");
		return;
	}

	if (!widget) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget object is invalid");
		return;
	}


	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget(%p) don't have the info", widget);
		return;
	}

	info->cancel_click = 1;

	pepper_obj = elm_object_part_content_get(widget, "pepper,widget");
	if (!pepper_obj) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget object is invalid");
		return;
	}

	if (!pepper_efl_object_touch_cancel(pepper_obj)) {
		set_last_result(WIDGET_ERROR_FAULT);
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
		ErrPrint("widget viewer evas is not initialized");
		return WIDGET_ERROR_FAULT;
	}

	if (!widget) {
		ErrPrint("widget object is invalid");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info", widget);
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
		set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
		return;
	}

	if (!s_info.initialized) {
		set_last_result(WIDGET_ERROR_FAULT);
		ErrPrint("widget viewer evas is not initialized");
		return;
	}

	if (!widget) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget object is invalid");
		return;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget(%p) don't have the info", widget);
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

	if (!is_widget_feature_enabled()) {
		set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
		return;
	}

	if (!s_info.initialized) {
		set_last_result(WIDGET_ERROR_FAULT);
		ErrPrint("widget viewer evas is not initialized");
		return;
	}

	if (!widget) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget object is invalid");
		return;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget(%p) don't have the info", widget);
		return;
	}

	info->disable_overlay = 1;
	elm_object_signal_emit(info->layout, "disable", "overlay,text");
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
		set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
		return;
	}

	if (!s_info.initialized) {
		set_last_result(WIDGET_ERROR_FAULT);
		ErrPrint("widget viewer evas is not initialized");
		return;
	}

	if (!widget) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget object is invalid");
		return;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget(%p) don't have the info", widget);
		return;
	}

	info->disable_loading = 1;
	elm_object_signal_emit(info->layout, "disable", "loading,text");
	return;
}

EAPI void widget_viewer_evas_activate_faulted_widget(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
		return;
	}

	if (!s_info.initialized) {
		set_last_result(WIDGET_ERROR_FAULT);
		ErrPrint("widget viewer evas is not initialized");
		return;
	}

	if (!widget) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget object is invalid");
		return;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget(%p) don't have the info", widget);
		return;
	}

	if (info->pid < 0) {

		struct widget_evas_event_info event_info;
		int w;
		int h;

		evas_object_geometry_get(info->layout, NULL, NULL, &w, &h);

		if (!info->disable_preview)
			elm_object_signal_emit(info->layout, "enable", "preview");

		if (!info->disable_loading) {
			elm_object_part_text_set(info->layout, "text", T_("IDS_ST_POP_LOADING_ING"));
			elm_object_signal_emit(info->layout, "enable", "text");
		}

		info->pid = widget_instance_launch(info->instance_id, info->content_info, w, h);
		if (info->pid < 0) {
			ErrPrint("Failed to launch an widget");
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

	if (!is_widget_feature_enabled()) {
		set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
		return false;
	}

	if (!s_info.initialized) {
		set_last_result(WIDGET_ERROR_FAULT);
		ErrPrint("widget viewer evas is not initialized");
		return false;
	}

	if (!widget) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget object is invalid");
		return false;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget(%p) don't have the info", widget);
		return false;
	}

	return info->pid < 0 ? true : false;
}

EAPI int widget_viewer_evas_freeze_visibility(Evas_Object *widget, widget_visibility_status_e status)
{
	struct widget_info *info;
	int ret = 0;
	Evas_Object *pepper_obj;

	if (!is_widget_feature_enabled()) {
		return WIDGET_ERROR_NOT_SUPPORTED;
	}

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized");
		return WIDGET_ERROR_FAULT;
	}

	if (!widget) {
		ErrPrint("widget object is invalid");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info", widget);
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	pepper_obj = elm_object_part_content_get(widget, "pepper,widget");
	if (!pepper_obj) {
		ErrPrint("widget object is invalid\n");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	if (status == WIDGET_VISIBILITY_STATUS_SHOW_FIXED) {
		ret = _compositor_freeze_visibility(pepper_obj, VISIBILITY_TYPE_UNOBSCURED);
		if (ret < 0) {
			ErrPrint("Fail to resume the widget(%p):(%d)", widget, ret);
			return ret;
		}
		info->visibility_freeze = status;
	} else if (status == WIDGET_VISIBILITY_STATUS_HIDE_FIXED) {
		ret = _compositor_freeze_visibility(pepper_obj, VISIBILITY_TYPE_FULLY_OBSCURED);
		if (ret < 0) {
			ErrPrint("Fail to pause the widget(%p):(%d)", widget, ret);
			return ret;
		}
		info->visibility_freeze = status;
	} else {

		ErrPrint("status value is invalid");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	return WIDGET_ERROR_NONE;
}

EAPI int widget_viewer_evas_thaw_visibility(Evas_Object *widget)
{
	struct widget_info *info;
	Evas_Object *pepper_obj;

	if (!is_widget_feature_enabled())
		return WIDGET_ERROR_NOT_SUPPORTED;

	if (!s_info.initialized) {
		ErrPrint("widget viewer evas is not initialized");
		return WIDGET_ERROR_FAULT;
	}

	if (!widget) {
		ErrPrint("widget object is invalid");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		ErrPrint("widget(%p) don't have the info", widget);
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	pepper_obj = elm_object_part_content_get(widget, "pepper,widget");
	if (!pepper_obj) {
		ErrPrint("widget object is invalid\n");
		return WIDGET_ERROR_INVALID_PARAMETER;
	}

	_compositor_thaw_visibility(pepper_obj);
	info->visibility_freeze = 0;

	return WIDGET_ERROR_NONE;
}

EAPI bool widget_viewer_evas_is_visibility_frozen(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
		return false;
	}

	if (!s_info.initialized) {
		set_last_result(WIDGET_ERROR_FAULT);
		ErrPrint("widget viewer evas is not initialized");
		return false;
	}

	if (!widget) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget object is invalid");
		return false;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget(%p) don't have the info", widget);
		return false;
	}

	if (info->visibility_freeze)
		return true;

	return false;
}

EAPI bool widget_viewer_evas_is_widget(Evas_Object *widget)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
		return false;
	}

	if (!s_info.initialized) {
		set_last_result(WIDGET_ERROR_FAULT);
		ErrPrint("widget viewer evas is not initialized");
		return false;
	}

	if (!widget) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget object is invalid");
		return false;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget(%p) don't have the info", widget);
		return false;
	}

	return true;
}

EAPI void widget_viewer_evas_set_permanent_delete(Evas_Object *widget, int flag)
{
	struct widget_info *info;

	if (!is_widget_feature_enabled()) {
		set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
		return;
	}

	if (!s_info.initialized) {
		set_last_result(WIDGET_ERROR_FAULT);
		ErrPrint("widget viewer evas is not initialized");
		return;
	}

	if (!widget) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget object is invalid");
		return;
	}

	info = evas_object_data_get(widget, WIDGET_INFO_TAG);
	if (!info) {
		set_last_result(WIDGET_ERROR_INVALID_PARAMETER);
		ErrPrint("widget(%p) don't have the info", widget);
		return;
	}

	info->permanent_delete = flag;
	return;
}

/* End of a file */
