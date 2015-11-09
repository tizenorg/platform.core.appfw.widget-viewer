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

#include <Elementary.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Edje.h>

#include <pkgmgr-info.h>
#include <system_info.h>

#include <widget_errno.h>

#include "util.h"

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

/*!
 * \note
 * Enable this to apply shadow effect to image object (for text widget)
 * #define SUPPORT_IMAGE_EFFECT
 */

int errno;

EAPI int widget_viewer_evas_init(Evas_Object *win)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_fini(void)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_notify_resumed_status_of_viewer(void)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_notify_paused_status_of_viewer(void)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_notify_orientation_of_viewer(int orientation)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI Evas_Object *widget_viewer_evas_add_widget(Evas_Object *parent, const char *widget_id, const char *content_info, double period)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_set_view_port(Evas_Object *widget, int x, int y, int w, int h)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_get_view_port(Evas_Object *widget, int *x, int *y, int *w, int *h)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_set_option(widget_evas_conf_e type, int value)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_pause_widget(Evas_Object *widget)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_resume_widget(Evas_Object *widget)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_destroy_glance_bar(Evas_Object *widget)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI const char *widget_viewer_evas_get_content_info(Evas_Object *widget)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI const char *widget_viewer_evas_get_title_string(Evas_Object *widget)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI const char *widget_viewer_evas_get_widget_id(Evas_Object *widget)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI double widget_viewer_evas_get_period(Evas_Object *widget)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI void widget_viewer_evas_cancel_click_event(Evas_Object *widget)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_feed_mouse_up_event(Evas_Object *widget)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_feed_access_event(Evas_Object *widget, int type, void *_info, void (*ret_cb)(Evas_Object *obj, int ret, void *data), void *cbdata)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

/**
 * @note
 * While displaying "loading" overlay, there is a preview image.
 * But if this function is called, the preview image will not be displayed.
 * Only the help text will be there. if it is not disabled of course.
 */
EAPI void widget_viewer_evas_disable_preview(Evas_Object *widget)
{
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
	return;
}

EAPI void widget_viewer_evas_activate_faulted_widget(Evas_Object *widget)
{
	return;
}

EAPI bool widget_viewer_evas_is_faulted(Evas_Object *widget)
{
	return false;
}

EAPI int widget_viewer_evas_set_raw_event_callback(widget_evas_raw_event_type_e type, raw_event_cb cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_unset_raw_event_callback(widget_evas_raw_event_type_e type, raw_event_cb cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_freeze_visibility(Evas_Object *widget, widget_visibility_status_e status)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_thaw_visibility(Evas_Object *widget)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI bool widget_viewer_evas_is_visibility_frozen(Evas_Object *widget)
{
	return false;
}

EAPI int widget_viewer_evas_dump_to_file(Evas_Object *widget, const char *filename)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI bool widget_viewer_evas_is_widget(Evas_Object *widget)
{
	return false;
}

EAPI void widget_viewer_evas_set_permanent_delete(Evas_Object *widget, int flag)
{
	return;
}

EAPI int widget_viewer_evas_subscribe_group(const char *cluster, const char *sub_cluster)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_unsubscribe_group(const char *cluster, const char *sub_cluster)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_subscribe_category(const char *category)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_unsubscribe_category(const char *category)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_emit_text_signal(Evas_Object *widget, widget_text_signal_s event_info, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_get_instance_id(Evas_Object *widget, char **instance_id)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_set_widget_option(Evas_Object *widget, widget_option_widget_e option, int value)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_set_preview_image(Evas_Object *widget, widget_size_type_e type, const char *preview)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_evas_hide_overlay(Evas_Object *widget)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

/**
 * @note
 * This function must has to be called after the object is faulted.
 * And this will not reset the faulted flag. it just dislable the faulted overlay if it is exists.
 *
 * The best usage of this function is called from smart-callback.
 * Any faulted smart callback can call this.
 * Then the widget_viewer_evas will check the flag to decide whether enable the faulted overlay or not.
 */
EAPI int widget_viewer_evas_hide_faulted_overlay_once(Evas_Object *widget)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

/* End of a file */
