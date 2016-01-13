/*
 * Copyright 2014  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <widget_errno.h>
#include "widget_viewer.h"

#define EAPI __attribute__((visibility("default")))
#if !defined(WIDGET_COUNT_OF_SIZE_TYPE)
	#define WIDGET_COUNT_OF_SIZE_TYPE 13
#endif

EAPI int widget_viewer_init(void *disp, int prevent_overwrite, double event_filter, int use_thread)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_fini(void)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI widget_h widget_viewer_add_widget(const char *pkgname, const char *content, const char *cluster, const char *category, double period, widget_size_type_e type, widget_ret_cb cb, void *data)
{
	set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
	return NULL;
}

EAPI int widget_viewer_get_period(widget_h handle, double *period)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_set_period(widget_h handle, double period, widget_ret_cb cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_delete_widget(widget_h handle, widget_delete_type_e type, widget_ret_cb cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_add_fault_handler(widget_fault_handler_cb widget_cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI void *widget_viewer_remove_fault_handler(widget_fault_handler_cb widget_cb)
{
	set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
	return NULL;
}

EAPI int widget_viewer_add_event_handler(widget_event_handler_cb widget_cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI void *widget_viewer_remove_event_handler(widget_event_handler_cb widget_cb)
{
	set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
	return NULL;
}

EAPI int widget_viewer_set_update_mode(widget_h handle, int active_update, widget_ret_cb cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_is_active_update(widget_h handle)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_resize_widget(widget_h handle, widget_size_type_e type, widget_ret_cb cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_send_click_event(widget_h handle, const char *event, double x, double y)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_has_glance_bar(widget_h handle)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_glance_bar_is_created(widget_h handle)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_create_glance_bar(widget_h handle, double x, double y, widget_ret_cb cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_move_glance_bar(widget_h handle, double x, double y)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_activate_faulted_widget(const char *pkgname, widget_ret_cb cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_destroy_glance_bar(widget_h handle, widget_ret_cb cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_feed_access_event(widget_h handle, widget_access_event_type_e type, widget_access_event_info_s info, widget_ret_cb cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_feed_mouse_event(widget_h handle, widget_mouse_event_type_e type, widget_mouse_event_info_s info)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_feed_key_event(widget_h handle, widget_key_event_type_e type, widget_key_event_info_s info, widget_ret_cb cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI const char *widget_viewer_get_filename(widget_h handle)
{
	set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
	return NULL;
}

EAPI int widget_viewer_get_glance_bar_size(widget_h handle, int *w, int *h)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_get_size_type(widget_h handle, widget_size_type_e *size_type)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_set_group(widget_h handle, const char *cluster, const char *category, widget_ret_cb cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_get_group(widget_h handle, const char **cluster, const char **category)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_get_supported_sizes(widget_h handle, int *cnt, widget_size_type_e *size_list)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI const char *widget_viewer_get_pkgname(widget_h handle)
{
	set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
	return NULL;
}

EAPI int widget_viewer_get_priority(widget_h handle, double *priority)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_delete_cluster(const char *cluster, widget_ret_cb cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_delete_category(const char *cluster, const char *category, widget_ret_cb cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_get_type(widget_h handle, int gbar, widget_type_e *widget_type)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_set_text_handler(widget_h handle, int gbar, struct widget_script_operators *ops)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_acquire_extra_resource_id(widget_h handle, int gbar, int idx, widget_ret_cb cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_acquire_resource_id(widget_h handle, int gbar, widget_ret_cb cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_release_resource_id(widget_h handle, int gbar, unsigned int resource_id)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI unsigned int widget_extra_resource_id(const widget_h handle, int gbar, int idx)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_get_resource_id(const widget_h handle, int gbar, unsigned int *resouce_id)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI void *widget_viewer_acquire_buffer(widget_h handle, int gbar)
{
	set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
	return NULL;
}

EAPI int widget_viewer_release_buffer(void *buffer)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_get_buffer_reference_count(void *buffer)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_get_buffer_size(widget_h handle, int gbar)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_is_created_by_user(widget_h handle)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_set_pinup(widget_h handle, int flag, widget_ret_cb cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_is_pinned_up(widget_h handle)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_has_pinup(widget_h handle)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_set_data(widget_h handle, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI void *widget_viewer_get_data(widget_h handle)
{
	set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
	return NULL;
}

EAPI const char *widget_viewer_get_content_string(widget_h handle)
{
	set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
	return NULL;
}

EAPI const char *widget_viewer_get_title_string(widget_h handle)
{
	set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
	return NULL;
}

EAPI int widget_viewer_emit_text_signal(widget_h handle, widget_text_signal_s event_info, widget_ret_cb cb, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_subscribe_group(const char *cluster, const char *category)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_unsubscribe_group(const char *cluster, const char *category)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_subscribe_category(const char *category)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_unsubscribe_category(const char *category)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_refresh(widget_h handle, int force)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_refresh_group(const char *cluster, const char *category, int force)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_set_visibility(widget_h handle, widget_visible_state_e state)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI widget_visible_state_e widget_viewer_get_visibility(widget_h handle)
{
	return 0;
}

EAPI int widget_viewer_notify_paused_status_of_viewer(void)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_notify_resumed_status_of_viewer(void)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_notify_orientation_of_viewer(int orientation)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_sync_buffer(widget_h handle, int gbar)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI const char *widget_viewer_get_alternative_icon(widget_h handle)
{
	set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
	return NULL;
}

EAPI const char *widget_viewer_get_alternative_name(widget_h handle)
{
	set_last_result(WIDGET_ERROR_NOT_SUPPORTED);
	return NULL;
}

EAPI int widget_viewer_acquire_buffer_lock(widget_h handle, int is_gbar)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_release_buffer_lock(widget_h handle, int is_gbar)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_set_option(widget_option_type_e option, int state)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_get_option(widget_option_type_e option)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_set_auto_launch_handler(widget_auto_launch_handler_cb widget_launch_handler, void *data)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_get_damaged_region(widget_h handle, int gbar, const widget_damage_region_s *region)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_get_affected_extra_buffer(widget_h handle, int gbar, int *idx, unsigned int *resource_id)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

EAPI int widget_viewer_get_instance_id(widget_h handle, char **instance_id)
{
	return WIDGET_ERROR_NOT_SUPPORTED;
}

/* End of a file */
