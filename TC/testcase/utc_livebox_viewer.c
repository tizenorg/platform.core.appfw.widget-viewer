/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <tet_api.h>
#include <stdlib.h>

#include <livebox.h>
#include <livebox-service.h>
#include <livebox-errno.h>

#define MUSIC_APP "org.tizen.music-player"
#define EMAIL_APP "org.tizen.email"
#define EMAIL_LIVEBOX EMAIL_APP ".livebox"
#define MUSIC_LIVEBOX MUSIC_APP ".livebox"
#define MUSIC_EASYBOX "org.tizen.music-player.easymode.livebox"

enum {
	POSITIVE_TC_IDX = 0x01,
	NEGATIVE_TC_IDX,
};

static void startup(void)
{
	int ret;
	/* start of TC */
	ret = livebox_init_with_options(NULL, 1, 0.01, 1);
	tet_printf("\n TC start: ret = %d", ret);
}


static void cleanup(void)
{
	/* end of TC */
	tet_printf("\n TC end");
	livebox_fini();
}

void (*tet_startup)(void) = startup;
void (*tet_cleanup)(void) = cleanup;

static void utc_livebox_client_paused_n(void)
{
	/*!
	 * \note
	 * Unable to test negative case
	 */
	dts_pass("livebox_client_paused", "skip negative test");
}

static void utc_livebox_client_paused_p(void)
{
	int ret;

	ret = livebox_client_paused();
	dts_check_eq("livebox_client_paused", ret, LB_STATUS_SUCCESS, "Success");
}

static void utc_livebox_client_resumed_n(void)
{
	/*!
	 * \note
	 * Unable to test negative case
	 */
	dts_pass("livebox_client_resumed", "skip negative test");
}

static void utc_livebox_client_resumed_p(void)
{
	int ret;

	ret = livebox_client_resumed();
	dts_check_eq("livebox_client_resumed", ret, LB_STATUS_SUCCESS, "Success");
}

static void create_ret_cb(struct livebox *handle, int ret, void *data)
{
	dts_check_eq("livebox_add", ret, LB_STATUS_SUCCESS, "Request to add a new box");
}

static void utc_livebox_add_p(void)
{
	struct livebox *handle;

	handle = livebox_add(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, create_ret_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_add", handle, NULL, "handle must not be NULL");
	}
}

static void utc_livebox_add_n(void)
{
	struct livebox *handle;

	handle = livebox_add(NULL, NULL, NULL, NULL, -1.0f, create_ret_cb, NULL);
	if (handle == NULL) {
		dts_check_eq("livebox_add", handle, NULL, "Handle is NULL");
	}
}

static void create_ret_with_size_cb(struct livebox *handle, int ret, void *data)
{
	dts_check_eq("livebox_add_with_size", ret, LB_STATUS_SUCCESS, "Request to add a new box with size");
}

static void utc_livebox_add_with_size_n(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(NULL, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, create_ret_with_size_cb, NULL);
	dts_check_eq("livebox_add_with_size", handle, NULL, "Error");
}

static void utc_livebox_add_with_size_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, create_ret_with_size_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_add_with_size", handle, NULL, "Error");
	}
}

static void utc_livebox_del_n(void)
{
	int ret;

	ret = livebox_del(NULL, NULL, NULL);
	dts_check_ne("livebox_del", ret, LB_STATUS_SUCCESS, "Failed");
}

static void del_ret_cb(struct livebox *handle, int ret, void *data)
{
	dts_check_eq("livebox_del", ret, LB_STATUS_SUCCESS, "Success");
}

static void create_cb_for_testing_del(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_del", ret, LB_STATUS_SUCCESS, "create failed");
		return;
	}

	ret = livebox_del(handle, del_ret_cb, NULL);
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_del", ret, LB_STATUS_SUCCESS, "Success");
	}
}

static void utc_livebox_del_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, create_cb_for_testing_del, NULL);
	if (!handle) {
		dts_check_ne("livebox_del", handle, NULL, "Failed to create a box");
	}
}

static void utc_livebox_del_NEW_n(void)
{
	int ret;

	ret = livebox_del_NEW(NULL, LB_DELETE_PERMANENTLY, NULL, NULL);
	dts_check_eq("livebox_del_NEW", ret, LB_STATUS_ERROR_INVALID, "Invalid");
}

static void del_NEW_ret_cb(struct livebox *handle, int ret, void *data)
{
	dts_check_eq("livebox_del_NEW", ret, LB_STATUS_SUCCESS, "Success");
}

static void create_cb_for_testing_del_NEW(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_del_NEW", ret, LB_STATUS_SUCCESS, "Create failed");
		return;
	}

	ret = livebox_del_NEW(handle, LB_DELETE_PERMANENTLY, del_NEW_ret_cb, NULL);
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_del_NEW", ret, LB_STATUS_SUCCESS, "Success");
	}
}

static void utc_livebox_del_NEW_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, create_cb_for_testing_del_NEW, NULL);
	if (!handle) {
		dts_check_ne("livebox_del_NEW", handle, NULL, "Failed to add a new box\n");
	}
}

static int event_handler(struct livebox *handler, enum livebox_event_type event, void *data)
{
	return 0;
}

static void utc_livebox_set_event_handler_n(void)
{
	int ret;

	ret = livebox_set_event_handler(NULL, NULL);
	dts_check_eq("livebox_set_event_handler", ret, LB_STATUS_ERROR_INVALID, "Invalid");
}

static void utc_livebox_set_event_handler_p(void)
{
	int ret;

	ret = livebox_set_event_handler(event_handler, (void *)123);
	dts_check_eq("livebox_set_event_handler", ret, LB_STATUS_ERROR_INVALID, "Invalid");
}

static void utc_livebox_unset_event_handler_n(void)
{
	/*!
	 * \note
	 * Unable to unset event handler
	 */
	dts_pass("livebox_unset_event_handler", "skip negative test");
}

static void utc_livebox_unset_event_handler_p(void)
{
	void *data;

	data = livebox_unset_event_handler(event_handler);
	dts_check_eq("livebox_unset_event_handler", data, (void *)123, "Unset");
}

static int fault_handler(enum livebox_fault_type type, const char *pkgname, const char *id, const char *func_name, void *data)
{
	return 0;
}

static void utc_livebox_set_fault_handler_n(void)
{
	int ret;
	ret = livebox_set_fault_handler(NULL, NULL);
	dts_check_eq("livebox_set_fault_handler", ret, LB_STATUS_ERROR_INVALID, "Invalid");
}

static void utc_livebox_set_fault_handler_p(void)
{
	int ret;
	ret = livebox_set_fault_handler(fault_handler, (void *)123);
	dts_check_eq("livebox_set_fault_handler", ret, LB_STATUS_SUCCESS, "Success");
}

static void utc_livebox_unset_fault_handler_n(void)
{
	/*!
	 * Unable to test negative case
	 */
	dts_pass("livebox_unset_fault_handler", "skip negative test");
}

static void utc_livebox_unset_fault_handler_p(void)
{
	void *data;

	data = livebox_unset_fault_handler(fault_handler);
	dts_check_eq("livebox_unset_fault_handler", data, (void *)123, "Unset success");
}

static void utc_livebox_activate_n(void)
{
	int ret;

	ret = livebox_activate(NULL, NULL, NULL);
	dts_check_eq("livebox_activate", ret, LB_STATUS_ERROR_INVALID, "Invalid");
}

static void activate_ret_cb(struct livebox *handle, int ret, void *data)
{
	dts_check_eq("livebox_activate", ret, LB_STATUS_SUCCESS, "Success");
}

static void utc_livebox_activate_p(void)
{
	int ret;

	ret = livebox_activate(MUSIC_LIVEBOX, activate_ret_cb, NULL);
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_activate", ret, LB_STATUS_SUCCESS, "Success");
	}
}

static void resize_cb(struct livebox *handle, int ret, void *data)
{
	dts_check_eq("livebox_resize", ret, LB_STATUS_SUCCESS, "Success");
}

static void resize_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret == LB_STATUS_SUCCESS) {
		ret = livebox_resize(handle, LB_SIZE_TYPE_2x2, resize_cb, NULL);
		if (ret != LB_STATUS_SUCCESS) {
			dts_check_eq("livebox_resize", ret, LB_STATUS_SUCCESS, "Success");
		}
	} else {
		dts_check_eq("livebox_resize", ret, LB_STATUS_SUCCESS, "resize,create,callback");
	}
}

static void utc_livebox_resize_n(void)
{
	int ret;
	ret = livebox_resize(NULL, LB_SIZE_TYPE_2x2, NULL, NULL);
	dts_check_eq("livebox_resize", ret, LB_STATUS_ERROR_INVALID, "resize");
}

static void utc_livebox_resize_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, resize_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_resize", handle, NULL, "Resize");
	}
}

static void utc_livebox_click_n(void)
{
	int ret;

	ret = livebox_click(NULL, 0.5f, 0.5f);
	dts_check_eq("livebox_click", ret, LB_STATUS_ERROR_INVALID, "Success");
}

static void click_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_click", ret, LB_STATUS_SUCCESS, "click,create");
	} else {
		ret = livebox_click(handle, 0.5f, 0.5f);
		dts_check_eq("livebox_click", ret, LB_STATUS_SUCCESS, "click");
	}
}

static void utc_livebox_click_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, click_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_click", handle, NULL, "click,create");
	}
}

static void utc_livebox_set_group_n(void)
{
	int ret;

	ret = livebox_set_group(NULL, NULL, NULL, NULL, NULL);
	dts_check_eq("livebox_set_group", ret, LB_STATUS_ERROR_INVALID, "Invalid");
}

static void set_group_cb(struct livebox *handle, int ret, void *data)
{
	dts_check_eq("livebox_set_group", ret, LB_STATUS_SUCCESS, "Success");
}

static void set_group_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_set_group", ret, LB_STATUS_SUCCESS, "set_group,create_cb");
	} else {
		ret = livebox_set_group(handle, "my,cluster", "my,category", set_group_cb, NULL);
		if (ret != LB_STATUS_SUCCESS) {
			dts_check_eq("livebox_set_group", ret, LB_STATUS_SUCCESS, "set_group,request");
		}
	}
}

static void utc_livebox_set_group_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, set_group_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_set_group", handle, NULL, "create,set,group");
	}
}

static void utc_livebox_get_group_n(void)
{
	int ret;

	ret = livebox_get_group(NULL, NULL, NULL);
	dts_check_eq("livebox_get_group", ret, LB_STATUS_ERROR_INVALID, "Success");
}

static void get_group_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_get_group", ret, LB_STATUS_SUCCESS, "get_group,create");
	} else {
		const char *cluster;
		const char *category;

		ret = livebox_get_group(handle, &cluster, &category);
		dts_check_eq("livebox_get_group", ret, LB_STATUS_SUCCESS, "Success");
	}
}

static void utc_livebox_get_group_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, get_group_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_get_group", handle, NULL, "get_group,create");
	}
}

static void utc_livebox_period_n(void)
{
	double period;

	period = livebox_period(NULL);
	dts_check_eq("livebox_period", period, 0.0f, "Success");
}

static void period_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_period", ret, LB_STATUS_SUCCESS, "period,create");
	} else {
		double period;

		period = livebox_period(handle);
		dts_check_ge("livebox_period", period, 0.0f, "Success");
	}
}

static void utc_livebox_period_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, period_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_add_with_size", handle, NULL, "livebox_add_with_size");
	}
}

static void utc_livebox_set_period_n(void)
{
	int ret;

	ret = livebox_set_period(NULL, 20.0f, NULL, NULL);
	dts_check_gt("livebox_period", ret, LB_STATUS_ERROR_INVALID, "Invalid");
}

static void set_period_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_period", ret, LB_STATUS_SUCCESS, "Success");
	} else {
		ret = livebox_set_period(handle, 20.0f, NULL, NULL);
		dts_check_eq("livebox_period", ret, LB_STATUS_SUCCESS, "Success");
	}
}

static void utc_livebox_set_period_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, set_period_create_cb, NULL);
	if (!handle) {
		dts_check_eq("livebox_set_period", handle, NULL, "set_period,create");
	}
}

static void utc_livebox_lb_type_n(void)
{
	int ret;

	ret = livebox_lb_type(NULL);
	dts_check_eq("livebox_lb_type", ret, LB_TYPE_INVALID, "Success");
}

static void lb_type_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_lb_type", ret, LB_STATUS_SUCCESS, "lb_type,create");
	} else {
		ret = livebox_lb_type(handle);
		dts_check_ne("livebox_lb_type", ret, LB_TYPE_INVALID, "Success");
	}
}

static void utc_livebox_lb_type_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, lb_type_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_lb_type", handle, NULL, "lb,type,create");
	}
}

static void utc_livebox_is_user_n(void)
{
	int ret;

	ret = livebox_is_user(NULL);
	dts_check_eq("livebox_is_user", ret, LB_STATUS_ERROR_INVALID, "Success");
}

static void livebox_is_user_create_cb(struct livebox *handle, int ret, void *data)
{
	return;
}

static void utc_livebox_is_user_p(void)
{
	struct livebox *handle;
	int ret;

	handle = livebox_add_with_size(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, livebox_is_user_create_cb, NULL);
	if (handle == NULL) {
		dts_check_ne("livebox_is_user", handle, NULL, "handle is NULL");
		return;
	}

	ret = livebox_is_user(handle);
	dts_check_eq("livebox_is_user", ret, 1, "Success");
}

static void utc_livebox_content_n(void)
{
	const char *content;

	content = livebox_content(NULL);
	dts_check_eq("livebox_content", content, NULL, "Success");
}

static void content_create_cb(struct livebox *handle, int ret, void *data)
{
	const char *content;

	content = livebox_content(handle);
	dts_check_ne("livebox_content", content, NULL, "Success");
}

static void utc_livebox_content_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, content_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_content", handle, NULL, "Create for content\n");
	}
}

static void utc_livebox_category_title_n(void)
{
	const char *title;
	title = livebox_category_title(NULL);
	dts_check_eq("livebox_category_title", title, NULL, "Success");
}

static void category_create_cb(struct livebox *handle, int ret, void *data)
{
	const char *title;
	title = livebox_category_title(handle);
	dts_check_ne("livebox_category_title", title, NULL, "Success");
}

static void utc_livebox_category_title_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, category_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_category_title", handle, NULL, "Create for category_title");
	}
}

static void utc_livebox_filename_n(void)
{
	const char *filename;
	filename = livebox_filename(NULL);
	dts_check_eq("livebox_filename", filename, NULL, "Success");
}

static void filename_create_cb(struct livebox *handle, int ret, void *data)
{
	const char *filename;
	filename = livebox_filename(handle);
	dts_check_ne("livebox_filename", filename, NULL, "Success");
}

static void utc_livebox_filename_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, filename_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_category_title", handle, NULL, "Create for category_title");
	}
}

static void utc_livebox_pkgname_n(void)
{
	const char *pkgname;
	pkgname = livebox_pkgname(NULL);
	dts_check_eq("livebox_pkgname", pkgname, NULL, "Success");
}

static void utc_livebox_pkgname_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, NULL, NULL);
	if (!handle) {
		dts_check_ne("livebox_pkgname", handle, NULL, "Create for pkgname");
	} else {
		const char *pkgname;

		pkgname = livebox_pkgname(handle);
		dts_check_ne("livebox_pkgname", pkgname, NULL, "pkgname");
	}
}

static void utc_livebox_priority_n(void)
{
	double priority;

	priority = livebox_priority(NULL);
	dts_check_eq("livebox_priority", priority, 0.0f, "Success");
}

static void priority_create_cb(struct livebox *handle, int ret, void *data)
{
	double priority;

	priority = livebox_priority(handle);
	dts_check_ge("livebox_priority", priority, 0.0f, "Success");
}

static void utc_livebox_priority_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, priority_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_priority", handle, NULL, "Create for priority");
	}
}

static void utc_livebox_acquire_fb_n(void)
{
	void *data;

	data = livebox_acquire_fb(NULL);
	dts_check_eq("livebox_acquire_fb", data, NULL, "Success");
}

static void acquire_fb_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_acquire_fb", ret, LB_STATUS_SUCCESS, "acquire_fb");
	} else {
		void *data;
		data = livebox_acquire_fb(handle);
		dts_check_ne("livebox_acquire_fb", data, NULL, "acquire_fb");
		livebox_release_fb(data);
	}
}

static void utc_livebox_acquire_fb_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, acquire_fb_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_priority", handle, NULL, "Create for priority");
	}
}

static void utc_livebox_release_fb_n(void)
{
	int ret;

	ret = livebox_release_fb(NULL);
	dts_check_eq("livebox_release_fb", ret, LB_STATUS_ERROR_INVALID, "Invalid");
}

static void release_fb_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_release_fb", ret, LB_STATUS_SUCCESS, "release_fb");
	} else {
		void *data;

		data = livebox_acquire_fb(handle);

		ret = livebox_release_fb(data);
		dts_check_eq("livebox_release_fb", ret, LB_STATUS_SUCCESS, "release_fb");
	}
}

static void utc_livebox_release_fb_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, release_fb_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_priority", handle, NULL, "Create for priority");
	}
}

static void refcnt_fb_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_fb_refcnt", ret, LB_STATUS_SUCCESS, "refcnt");
	} else {
		void *data;

		data = livebox_acquire_fb(handle);

		ret = livebox_fb_refcnt(data);
		dts_check_ge("livebox_fb_refcnt", ret, 0, "refcnt");

		(void)livebox_release_fb(data);
	}
}

static void utc_livebox_fb_refcnt_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, refcnt_fb_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_priority", handle, NULL, "Create for priority");
	}
}

static void utc_livebox_fb_refcnt_n(void)
{
	int ret;

	ret = livebox_fb_refcnt(NULL);
	dts_check_eq("livebox_fb_refcnt", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void acquire_pdfb_pd_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_acquire_pdfb", ret, LB_STATUS_SUCCESS, "create pd failed");
		return;
	}

	data = livebox_acquire_pdfb(handle);
	dts_check_ne("livebox_acquire_pdfb", data, NULL, "acquire_pdfb");
	(void)livebox_release_pdfb(data);
}

static void acquire_pdfb_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_acquire_pdfb", ret, LB_STATUS_SUCCESS, "refcnt");
	} else {
		ret = livebox_create_pd(handle, acquire_pdfb_pd_cb, NULL);
		if (ret != LB_STATUS_SUCCESS) {
			dts_check_eq("livebox_acquire_pdfb", ret, LB_STATUS_SUCCESS, "refcnt");
		}
	}
}

static void utc_livebox_acquire_pdfb_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, acquire_pdfb_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_priority", handle, NULL, "Create for priority");
	}
}

static void utc_livebox_acquire_pdfb_n(void)
{
	void *data;

	data = livebox_acquire_pdfb(NULL);
	dts_check_eq("livebox_acquire_pdfb", data, NULL, "invalid");
}

static void release_pdfb_pd_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_release_pdfb", ret, LB_STATUS_SUCCESS, "create pd");
		return;
	}

	data = livebox_acquire_pdfb(handle);
	ret = livebox_release_pdfb(data);
	dts_check_eq("livebox_release_pdfb", ret, LB_STATUS_SUCCESS, "release_pdfb");
}

static void release_pdfb_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_acquire_pdfb", ret, LB_STATUS_SUCCESS, "refcnt");
	} else {
		ret = livebox_create_pd(handle, release_pdfb_pd_cb, NULL);
		if (ret != LB_STATUS_SUCCESS) {
			dts_check_eq("livebox_acquire_pdfb", ret, LB_STATUS_SUCCESS, "refcnt");
		}
	}
}

static void utc_livebox_release_pdfb_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, release_pdfb_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_priority", handle, NULL, "Create for priority");
	}
}

static void utc_livebox_release_pdfb_n(void)
{
	int ret;
	ret = livebox_release_pdfb(NULL);
	dts_check_eq("livebox_release_pdfb", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void utc_livebox_pdfb_refcnt_n(void)
{
	int ret;
	ret = livebox_pdfb_refcnt(NULL);
	dts_check_eq("livebox_pdfb_refcnt", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void refcnt_pdfb_pd_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_pdfb_refcnt", ret, LB_STATUS_SUCCESS, "create pd");
		return;
	}

	data = livebox_acquire_pdfb(handle);
	ret = livebox_pdfb_refcnt(data);
	dts_check_ge("livebox_pdfb_refcnt", ret, 0, "refcnt_pdfb");
	(void)livebox_release_pdfb(data);
}

static void refcnt_pdfb_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_pdfb_refcnt", ret, LB_STATUS_SUCCESS, "refcnt");
	} else {
		ret = livebox_create_pd(handle, refcnt_pdfb_pd_cb, NULL);
		if (ret != LB_STATUS_SUCCESS) {
			dts_check_eq("livebox_pdfb_refcnt", ret, LB_STATUS_SUCCESS, "refcnt");
		}
	}
}

static void utc_livebox_pdfb_refcnt_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, refcnt_pdfb_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_pdfb_refcnt", handle, NULL, "Create for pdfb refcnt");
	}
}

static void size_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_size", ret, LB_STATUS_SUCCESS, "size");
	} else {
		int size;
		size = livebox_size(handle);
		dts_check_eq("livebox_size", size, LB_SIZE_TYPE_1x1, "size");
	}
}

static void utc_livebox_size_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, size_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_size", handle, NULL, "Create for size");
	}
}

static void utc_livebox_size_n(void)
{
	int ret;
	ret = livebox_size(NULL);
	dts_check_eq("livebox_size", ret, LB_STATUS_ERROR_INVALID, "Invalid");
}

static void utc_livebox_get_pdsize_n(void)
{
	int ret;
	ret = livebox_get_pdsize(NULL, NULL, NULL);
	dts_check_eq("livebox_get_pdsize", ret, LB_STATUS_ERROR_INVALID, "Invalid");
}

static void pdsize_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_get_pdsize", ret, LB_STATUS_SUCCESS, "pdsize");
	} else {
		int w;
		int h;

		ret = livebox_get_pdsize(handle, &w, &h);
		dts_check_eq("livebox_get_pdsize", ret, LB_STATUS_SUCCESS, "pdsize");
	}
}

static void utc_livebox_get_pdsize_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, pdsize_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_get_pdsize", handle, NULL, "Create for priority");
	}
}

static void utc_livebox_get_supported_sizes_n(void)
{
	int ret;

	ret = livebox_get_supported_sizes(NULL, NULL, NULL);
	dts_check_eq("livebox_get_supported_sizes", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void size_list_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_get_supported_sizes", ret, LB_STATUS_SUCCESS, "size_list");
	} else {
		int cnt = 20;
		int size_list[20];

		ret = livebox_get_supported_sizes(handle, &cnt, size_list);
		dts_check_eq("livebox_get_supported_sizes", ret, LB_STATUS_SUCCESS, "size_list");
	}
}

static void utc_livebox_get_supported_sizes_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, size_list_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_get_supported_sizes", handle, NULL, "Create for supported_sizes");
	}
}

static void utc_livebox_lbfb_bufsz_n(void)
{
	int ret;
	ret = livebox_lbfb_bufsz(NULL);
	dts_check_eq("livebox_lbfb_bufsz", ret, LB_STATUS_ERROR_INVALID, "Invalid");
}

static void lbfb_bufsz_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_lbfb_bufsz", ret, LB_STATUS_SUCCESS, "size_list");
	} else {
		int bufsz;
		bufsz = livebox_lbfb_bufsz(handle);
		dts_check_gt("livebox_lbfb_bufsz", bufsz, 0, "lbfb_bufsz");
	}
}

static void utc_livebox_lbfb_bufsz_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, lbfb_bufsz_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_lbfb_bufsz", handle, NULL, "Create for supported_sizes");
	}
}

static void utc_livebox_pdfb_bufsz_n(void)
{
	int ret;
	ret = livebox_pdfb_bufsz(NULL);
	dts_check_eq("livebox_pdfb_bufsz", ret, LB_STATUS_ERROR_INVALID, "Invalid");
}

static void pdfb_bufsz_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_pdfb_bufsz", ret, LB_STATUS_SUCCESS, "size_list");
	} else {
		int bufsz;
		bufsz = livebox_pdfb_bufsz(handle);
		dts_check_gt("livebox_pdfb_bufsz", bufsz, 0, "pdfb_bufsz");
	}
}

static void utc_livebox_pdfb_bufsz_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, pdfb_bufsz_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_pdfb_bufsz", handle, NULL, "Create for supported_sizes");
	}
}

static void utc_livebox_content_event_n(void)
{
	int ret;

	ret = livebox_content_event(NULL, PD_MOUSE_DOWN, 0.0f, 0.0f);
	dts_check_eq("livebox_content_event", ret, LB_STATUS_ERROR_INVALID, "Invalid");
}

static void content_event_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_content_event", ret, LB_STATUS_SUCCESS, "content_event");
	} else {
		ret = livebox_content_event(handle, PD_MOUSE_DOWN, 0.0f, 0.0f);
		dts_check_eq("livebox_content_event", ret, LB_STATUS_SUCCESS, "content_event");
	}
}

static void utc_livebox_content_event_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, content_event_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_content_event", handle, NULL, "Create for content_event");
	}
}

static void utc_livebox_mouse_event_n(void)
{
	int ret;
	ret = livebox_mouse_event(NULL, PD_MOUSE_DOWN, 0.0f, 0.0f);
	dts_check_eq("livebox_mouse_event", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void mouse_event_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_content_event", ret, LB_STATUS_SUCCESS, "content_event");
	} else {
		ret = livebox_content_event(handle, PD_MOUSE_DOWN, 0.0f, 0.0f);
		dts_check_eq("livebox_content_event", ret, LB_STATUS_SUCCESS, "content_event");
	}
}

static void utc_livebox_mouse_event_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, mouse_event_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_content_event", handle, NULL, "Create for content_event");
	}
}

static void utc_livebox_access_event_n(void)
{
	int ret;
	ret = livebox_access_event(NULL, ACCESS_EVENT_ACTION_DOWN, 0.0f, 0.0f, NULL, NULL);
	dts_check_eq("livebox_mouse_event", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void access_event_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_access_event", ret, LB_STATUS_SUCCESS, "access_event");
	} else {
		ret = livebox_access_event(handle, ACCESS_EVENT_ACTION_DOWN, 0.0f, 0.0f, NULL, NULL);
		dts_check_eq("livebox_access_event", ret, LB_STATUS_SUCCESS, "access_event");
	}
}

static void utc_livebox_access_event_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, access_event_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_access_event", handle, NULL, "Create for access_event");
	}
}

static void utc_livebox_key_event_n(void)
{
	int ret;
	ret = livebox_key_event(NULL, PD_KEY_DOWN, 13, NULL, NULL);
	dts_check_eq("livebox_key_event", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void key_event_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_key_event", ret, LB_STATUS_SUCCESS, "key_event");
	} else {
		ret = livebox_key_event(handle, LB_KEY_DOWN, 13, NULL, NULL);
		dts_check_eq("livebox_key_event", ret, LB_STATUS_SUCCESS, "key_event");
	}
}

static void utc_livebox_key_event_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, key_event_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_key_event", handle, NULL, "Create for key_event");
	}
}

static void utc_livebox_set_pinup_n(void)
{
	int ret;
	ret = livebox_set_pinup(NULL, 0, NULL, NULL);
	dts_check_eq("livebox_set_pinup", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void pinup_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_set_pinup", ret, LB_STATUS_SUCCESS, "key_event");
	} else {
		ret = livebox_set_pinup(handle, 0, NULL, NULL);
		dts_check_eq("livebox_set_pinup", ret, LB_STATUS_ERROR_ALREADY, "already");
	}
}

static void utc_livebox_set_pinup_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, pinup_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_set_pinup", handle, NULL, "Create for pinup");
	}
}

static void utc_livebox_is_pinned_up_n(void)
{
	int ret;
	ret = livebox_is_pinned_up(NULL);
	dts_check_eq("livebox_is_pinned_up", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void is_pinup_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_is_pinned_up", ret, LB_STATUS_SUCCESS, "key_event");
	} else {
		ret = livebox_is_pinned_up(handle);
		dts_check_eq("livebox_is_pinned_up", ret, LB_STATUS_ERROR_ALREADY, "already");
	}
}

static void utc_livebox_is_pinned_up_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, is_pinup_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_is_pinned_up", handle, NULL, "Create for is_pinned_up");
	}
}

static void utc_livebox_has_pinup_n(void)
{
	int ret;
	ret = livebox_has_pinup(NULL);
	dts_check_eq("livebox_has_pinup", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void has_pinup_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_has_pinup", ret, LB_STATUS_SUCCESS, "has_pinup");
	} else {
		ret = livebox_has_pinup(handle);
		dts_check_eq("livebox_has_pinup", ret, 0, "not supported");
	}
}

static void utc_livebox_has_pinup_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, has_pinup_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_has_pinup", handle, NULL, "Create for has_pinup");
	}
}

static void has_pd_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_has_pd", ret, LB_STATUS_SUCCESS, "has_pd");
	} else {
		ret = livebox_has_pd(handle);
		dts_check_eq("livebox_has_pd", ret, 1, "PD supported");
	}
}

static void utc_livebox_has_pd_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, has_pd_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_has_pinup", handle, NULL, "Create for has_pinup");
	}
}

static void utc_livebox_has_pd_n(void)
{
	int ret;
	ret = livebox_has_pd(NULL);
	dts_check_eq("livebox_has_pd", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void utc_livebox_create_pd_n(void)
{
	int ret;
	ret = livebox_create_pd(NULL, NULL, NULL);
	dts_check_eq("livebox_create_pd", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void create_pd_cb(struct livebox *handle, int ret, void *data)
{
	dts_check_eq("livebox_create_pd", ret, LB_STATUS_SUCCESS, "create_pd");
	(void)livebox_destroy_pd(handle, NULL, NULL);
}

static void create_pd_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_create_pd", ret, LB_STATUS_SUCCESS, "create_pd");
	} else {
		ret = livebox_create_pd(handle, create_pd_cb, NULL);
		if (ret != LB_STATUS_SUCCESS) {
			dts_check_eq("livebox_create_pd", ret, LB_STATUS_SUCCESS, "create_pd");
		}
	}
}

static void utc_livebox_create_pd_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, create_pd_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_has_pinup", handle, NULL, "Create for has_pinup");
	}
}

static void utc_livebox_create_pd_with_position_n(void)
{
	int ret;
	ret = livebox_create_pd_with_position(NULL, 0.0f, 0.0f, NULL, NULL);
	dts_check_eq("livebox_create_pd_with_position", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void create_pd_pos_cb(struct livebox *handle, int ret, void *data)
{
	dts_check_eq("livebox_create_pd_with_position", ret, LB_STATUS_SUCCESS, "PD,create");
	(void)livebox_destroy_pd(handle, NULL, NULL);
}

static void create_pd_pos_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_create_pd_with_position", ret, LB_STATUS_SUCCESS, "create_pd_w/h_position");
	} else {
		ret = livebox_create_pd_with_position(handle, 0.0f, 0.0f, create_pd_pos_cb, NULL);
		if (ret != LB_STATUS_SUCCESS) {
			dts_check_eq("livebox_create_pd_with_position", ret, LB_STATUS_SUCCESS, "create_pd_w/h_position");
		}
	}
}

static void utc_livebox_create_pd_with_position_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, create_pd_pos_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_create_pd_with_position", handle, NULL, "Create for create_pd_with_position");
	}
}

static void utc_livebox_move_pd_n(void)
{
	int ret;
	ret = livebox_move_pd(NULL, 0.0f, 0.0f);
	dts_check_eq("livebox_move_pd", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void create_move_pd_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_move_pd", ret, LB_STATUS_SUCCESS, "invalid");
	} else {
		ret = livebox_move_pd(handle, 0.5f, 0.5f);
		dts_check_eq("livebox_move_pd", ret, LB_STATUS_SUCCESS, "invalid");
	}
}

static void move_pd_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_move_pd", ret, LB_STATUS_SUCCESS, "invalid");
	} else {
		ret = livebox_create_pd_with_position(handle, 0.0f, 0.0f, create_move_pd_cb, NULL);
		if (ret != LB_STATUS_SUCCESS) {
			dts_check_eq("livebox_move_pd", ret, LB_STATUS_SUCCESS, "invalid");
		}
	}
}

static void utc_livebox_move_pd_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, move_pd_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_move_pd", handle, NULL, "Create for move_pd");
	}
}

static void utc_livebox_destroy_pd_n(void)
{
	int ret;
	ret = livebox_destroy_pd(NULL, NULL, NULL);
	dts_check_eq("livebox_destroy_pd", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void destroy_pd_cb(struct livebox *handle, int ret, void *data)
{
	dts_check_eq("livebox_destroy_pd", ret, LB_STATUS_SUCCESS, "destroy_pd");
}

static void destroy_pd_create_pd_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_destroy_pd", ret, LB_STATUS_SUCCESS, "destroy_pd");
	} else {
		ret = livebox_destroy_pd(handle, destroy_pd_cb, NULL);
		if (ret != LB_STATUS_SUCCESS) {
			dts_check_eq("livebox_destroy_pd", ret, LB_STATUS_SUCCESS, "destroy_pd");
		}
	}
}

static void destroy_pd_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_destroy_pd", ret, LB_STATUS_SUCCESS, "destroy_pd");
	} else {
		ret = livebox_create_pd_with_position(handle, 0.0f, 0.0f, destroy_pd_create_pd_cb, NULL);
		if (ret != LB_STATUS_SUCCESS) {
			dts_check_eq("livebox_destroy_pd", ret, LB_STATUS_SUCCESS, "invalid");
		}
	}
}

static void utc_livebox_destroy_pd_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, destroy_pd_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_destroy_pd", handle, NULL, "Create for destroy_pd");
	}
}

static void utc_livebox_pd_is_created_n(void)
{
	int ret;
	ret = livebox_pd_is_created(NULL);
	dts_check_eq("livebox_pd_is_created", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void pd_is_created_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_pd_is_created", ret, LB_STATUS_SUCCESS, "pd_is_created");
	} else {
		ret = livebox_pd_is_created(handle);
		dts_check_eq("livebox_pd_is_created", ret, 0, "pd_is_created");
		(void)livebox_del(handle, NULL, NULL);
	}
}

static void utc_livebox_pd_is_created_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, pd_is_created_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_pd_is_created", handle, NULL, "create for pd_is_created");
	}
}

static void utc_livebox_pd_type_n(void)
{
	int type;
	type = livebox_pd_type(NULL);
	dts_check_eq("livebox_pd_type", type, PD_TYPE_INVALID, "invalid");
}

static void pd_type_created_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_pd_type", ret, LB_STATUS_SUCCESS, "pd_type");
	} else {
		ret = livebox_pd_type(handle);
		dts_check_ne("livebox_pd_type", ret, PD_TYPE_INVALID, "pd_type");
	}
}

static void utc_livebox_pd_type_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, pd_type_created_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_pd_type", handle, NULL, "create for pd_is_created");
	}
}

static void utc_livebox_is_exists_n(void)
{
	int ret;
	ret = livebox_is_exists(NULL);
	dts_check_eq("livebox_is_exists", ret, 0, "not exists");
}

static void utc_livebox_is_exists_p(void)
{
	int ret;
	ret = livebox_is_exists(MUSIC_APP);
	dts_check_eq("livebox_is_exists", ret, 1, "exists");
}

static void utc_livebox_set_text_handler_n(void)
{
	int ret;
	ret = livebox_set_text_handler(NULL, NULL);
	dts_check_eq("livebox_set_text_handler", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void set_text_handler_created_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_set_text_handler", ret, LB_STATUS_SUCCESS, "set_text_handler");
	} else {
		struct livebox_script_operators ops = {
			.update_begin = NULL,
			.update_end = NULL,
			.update_text = NULL,
			.update_image = NULL,
			.update_script = NULL,
			.update_signal = NULL,
			.update_drag = NULL,
			.update_info_size = NULL,
			.update_info_category = NULL,
			.update_access = NULL,
			.operate_access = NULL,
		};

		ret = livebox_set_text_handler(handle, &ops);
		dts_check_eq("livebox_set_text_handler", ret, LB_STATUS_SUCCESS, "set_text_handler");
	}
}

static void utc_livebox_set_text_handler_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, set_text_handler_created_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_set_text_handler", handle, NULL, "create for pd_is_created");
	}
}

static void utc_livebox_set_pd_text_handler_n(void)
{
	int ret;
	ret = livebox_set_pd_text_handler(NULL, NULL);
	dts_check_eq("livebox_set_pd_text_handler", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void set_pd_text_handler_created_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_set_pd_text_handler", ret, LB_STATUS_SUCCESS, "set_text_handler");
	} else {
		struct livebox_script_operators ops = {
			.update_begin = NULL,
			.update_end = NULL,
			.update_text = NULL,
			.update_image = NULL,
			.update_script = NULL,
			.update_signal = NULL,
			.update_drag = NULL,
			.update_info_size = NULL,
			.update_info_category = NULL,
			.update_access = NULL,
			.operate_access = NULL,
		};

		ret = livebox_set_pd_text_handler(handle, &ops);
		dts_check_eq("livebox_set_pd_text_handler", ret, LB_STATUS_SUCCESS, "set_text_handler");
	}
}

static void utc_livebox_set_pd_text_handler_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, set_pd_text_handler_created_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_set_pd_text_handler", handle, NULL, "create for pd_is_created");
	}
}

static void utc_livebox_emit_text_signal_n(void)
{
	int ret;
	ret = livebox_emit_text_signal(NULL, NULL, NULL, 0.0f, 0.0f, 0.0f, 0.0f, NULL, NULL);
	dts_check_eq("livebox_emit_text_signal", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void emit_signal_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_emit_text_signal", ret, LB_STATUS_SUCCESS, "set_text_handler");
	} else {
		ret = livebox_emit_text_signal(handle, NULL, NULL, 0.0f, 0.0f, 0.0f, 0.0f, NULL, NULL);
		dts_check_eq("livebox_emit_text_signal", ret, LB_STATUS_ERROR_INVALID, "text_handler");
	}
}

static void utc_livebox_emit_text_signal_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, emit_signal_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_emit_text_signal", handle, NULL, "create for pd_is_created");
	}
}

static void utc_livebox_set_data_n(void)
{
	int ret;
	ret = livebox_set_data(NULL, NULL);
	dts_check_eq("livebox_set_data", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void utc_livebox_set_data_p(void)
{
	struct livebox *handle;
	int ret;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, emit_signal_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_set_data", handle, NULL, "create for pd_is_created");
		return;
	}

	ret = livebox_set_data(handle, (void *)123);
	dts_check_eq("livebox_set_data", ret, LB_STATUS_SUCCESS, "set_data");
}

static void utc_livebox_get_data_p(void)
{
	struct livebox *handle;
	void *data;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, emit_signal_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_set_data", handle, NULL, "create for pd_is_created");
		return;
	}

	data = livebox_get_data(handle);
	dts_check_eq("livebox_get_data", data, NULL, "get_data");
}

static void utc_livebox_get_data_n(void)
{
	void *data;
	data = livebox_get_data(NULL);
	dts_check_eq("livebox_get_data", data, NULL, "data");
}

static void utc_livebox_subscribe_group_n(void)
{
	/*!
	 * \note
	 * Unable to test negative case
	 */
	dts_pass("livebox_subscribe_group", "pass negative test");
}

static void utc_livebox_subscribe_group_p(void)
{
	int ret;
	ret = livebox_subscribe_group("my,cluster", "my,category");
	dts_check_eq("livebox_subscribe_group", ret, LB_STATUS_SUCCESS, "subscribe");
}

static void utc_livebox_unsubscribe_group_n(void)
{
	/*!
	 * \note
	 * Unable to test negative case
	 */
	dts_pass("livebox_unsubscribe_group", "pass negative test");
}

static void utc_livebox_unsubscribe_group_p(void)
{
	int ret;
	ret = livebox_unsubscribe_group("my,cluster", "my,category");
	dts_check_eq("livebox_unsubscribe_group", ret, LB_STATUS_SUCCESS, "unsubscribe");
}

static void utc_livebox_refresh_group_n(void)
{
	int ret;
	ret = livebox_refresh_group(NULL, NULL, 0);
	dts_check_eq("livebox_refresh_group", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void utc_livebox_refresh_group_p(void)
{
	int ret;
	ret = livebox_refresh_group("my,cluster", "my,category", 0);
	dts_check_eq("livebox_refresh_group", ret, LB_STATUS_SUCCESS, "refresh_group");
}

static void refresh_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_refresh", ret, LB_STATUS_SUCCESS, "refresh");
	} else {
		ret = livebox_refresh(handle, 0);
		dts_check_eq("livebox_refresh", ret, LB_STATUS_SUCCESS, "refresh");
	}
}

static void utc_livebox_refresh_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, refresh_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_set_data", handle, NULL, "create for pd_is_created");
		return;
	}
}

static void utc_livebox_refresh_n(void)
{
	int ret;
	ret = livebox_refresh(NULL, 0);
	dts_check_eq("livebox_refresh", ret, LB_STATUS_ERROR_INVALID, "invalid");
}


static void utc_livebox_lb_pixmap_n(void)
{
	int ret;
	ret = livebox_lb_pixmap(NULL);
	dts_check_eq("livebox_lb_pixmap", ret, 0, "invalid");
}

static void lb_pixmap_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_lb_pixmap", ret, LB_STATUS_SUCCESS, "lb_pixmap");
	} else {
		ret = livebox_lb_pixmap(handle);
		dts_check_ne("livebox_lb_pixmap", ret, 0, "lb_pixmap");
	}
}

static void utc_livebox_lb_pixmap_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, lb_pixmap_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_lb_pixmap", handle, NULL, "create for lb_pixmap");
		return;
	}
}

static void pd_pixmap_create_pd_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_pd_pixmap", ret, LB_STATUS_SUCCESS, "invalid");
	} else {
		ret = livebox_pd_pixmap(handle);
		dts_check_ne("livebox_pd_pixmap", ret, 0, "pd_pixmap");
	}
}

static void pd_pixmap_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_pd_pixmap", ret, LB_STATUS_SUCCESS, "invalid");
	} else {
		ret = livebox_create_pd(handle, pd_pixmap_create_pd_cb, NULL);
		if (ret != LB_STATUS_SUCCESS) {
			dts_check_eq("livebox_pd_pixmap", ret, LB_STATUS_SUCCESS, "invalid");
		}
	}
}

static void utc_livebox_pd_pixmap_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, pd_pixmap_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_pd_pixmap", handle, NULL, "create for pd_pixmap");
	}
}

static void utc_livebox_pd_pixmap_n(void)
{
	int ret;
	ret = livebox_pd_pixmap(NULL);
	dts_check_eq("livebox_pd_pixmap", ret, 0, "invalid");
}

static void utc_livebox_acquire_pd_pixmap_n(void)
{
	int ret;
	ret = livebox_acquire_pd_pixmap(NULL, NULL, NULL);
	dts_check_eq("livebox_acquire_pd_pixmap", ret, 0, "invalid");
}

static void acquire_pd_pixmap_cb(struct livebox *handle, int pixmap, void *data)
{
	dts_check_ne("livebox_acquire_pd_pixmap", pixmap, 0, "acquire_pd_pixmap");
	(void)livebox_release_pd_pixmap(handle, pixmap);
}

static void pd_acquire_pixmap_pd_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_acquire_pd_pixmap", ret, LB_STATUS_SUCCESS, "invalid");
	} else {
		ret = livebox_acquire_pd_pixmap(handle, acquire_pd_pixmap_cb, NULL);
		if (ret != LB_STATUS_SUCCESS) {
			dts_check_eq("livebox_acquire_pd_pixmap", ret, LB_STATUS_SUCCESS, "invalid");
		}
	}
}

static void pd_acquire_pixmap_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_acquire_pd_pixmap", ret, LB_STATUS_SUCCESS, "invalid");
	} else {
		ret = livebox_create_pd(handle, pd_acquire_pixmap_pd_create_cb, NULL);
		if (ret != LB_STATUS_SUCCESS) {
			dts_check_eq("livebox_acquire_pd_pixmap", ret, LB_STATUS_SUCCESS, "invalid");
		}
	}
}

static void utc_livebox_acquire_pd_pixmap_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, pd_acquire_pixmap_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_pd_pixmap", handle, NULL, "create for pd_pixmap");
	}
}

static void utc_livebox_release_pd_pixmap_n(void)
{
	int ret;
	ret = livebox_release_pd_pixmap(NULL, 0);
	dts_check_eq("livebox_release_pd_pixmap", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void release_pd_pixmap_cb(struct livebox *handle, int pixmap, void *data)
{
	int ret;

	if (pixmap == 0) {
		dts_check_ne("livebox_release_pd_pixmap", pixmap, 0, "release_pd_pixmap");
		return;
	}

	ret = livebox_release_pd_pixmap(handle, pixmap);
	dts_check_eq("livebox_release_pd_pixmap", ret, LB_STATUS_SUCCESS, "release_pd_pixmap");
}

static void pd_release_pixmap_pd_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_release_pd_pixmap", ret, LB_STATUS_SUCCESS, "release_pd_pixmap");
	} else {
		ret = livebox_acquire_pd_pixmap(handle, release_pd_pixmap_cb, NULL);
		if (ret != LB_STATUS_SUCCESS) {
			dts_check_eq("livebox_release_pd_pixmap", ret, LB_STATUS_SUCCESS, "release_pd_pixmap");
		}
	}
}

static void pd_release_pixmap_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_release_pd_pixmap", ret, LB_STATUS_SUCCESS, "release_pd_pixmap");
	} else {
		ret = livebox_create_pd(handle, pd_release_pixmap_pd_create_cb, NULL);
		if (ret != LB_STATUS_SUCCESS) {
			dts_check_eq("livebox_release_pd_pixmap", ret, LB_STATUS_SUCCESS, "release_pd_pixmap");
		}
	}
}

static void utc_livebox_release_pd_pixmap_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, pd_release_pixmap_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_release_pd_pixmap", handle, NULL, "create for release_pd_pixmap");
	}
}

static void acquire_lb_pixmap_cb(struct livebox *handle, int pixmap, void *data)
{
	dts_check_ne("livebox_acquire_lb_pixmap", pixmap, 0, "acquire_lb_pixmap");
	(void)livebox_release_lb_pixmap(handle, pixmap);
}

static void lb_acquire_pixmap_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_acquire_lb_pixmap", ret, LB_STATUS_SUCCESS, "invalid");
	} else {
		ret = livebox_acquire_lb_pixmap(handle, acquire_lb_pixmap_cb, NULL);
		if (ret != LB_STATUS_SUCCESS) {
			dts_check_eq("livebox_acquire_lb_pixmap", ret, LB_STATUS_SUCCESS, "invalid");
		}
	}
}

static void utc_livebox_acquire_lb_pixmap_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, lb_acquire_pixmap_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_acquire_lb_pixmap", handle, NULL, "create for lb_pixmap");
	}
}

static void utc_livebox_acquire_lb_pixmap_n(void)
{
	int ret;
	ret = livebox_acquire_lb_pixmap(NULL, NULL, NULL);
	dts_check_eq("livebox_acquire_lb_pixmap", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void release_lb_pixmap_cb(struct livebox *handle, int pixmap, void *data)
{
	if (pixmap == 0) {
		dts_check_ne("livebox_release_lb_pixmap", pixmap, 0, "release_lb_pixmap");
	} else {
		int ret;
		ret = livebox_release_lb_pixmap(handle, pixmap);
		dts_check_eq("livebox_release_lb_pixmap", ret, LB_STATUS_SUCCESS, "release_lb_pixmap");
	}
}

static void lb_release_pixmap_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_release_lb_pixmap", ret, LB_STATUS_SUCCESS, "invalid");
	} else {
		ret = livebox_acquire_lb_pixmap(handle, release_lb_pixmap_cb, NULL);
		if (ret != LB_STATUS_SUCCESS) {
			dts_check_eq("livebox_release_lb_pixmap", ret, LB_STATUS_SUCCESS, "invalid");
		}
	}
}

static void utc_livebox_release_lb_pixmap_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, lb_release_pixmap_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_release_lb_pixmap", handle, NULL, "create for lb_pixmap");
	}
}

static void utc_livebox_release_lb_pixmap_n(void)
{
	int ret;
	ret = livebox_release_lb_pixmap(NULL, 0);
	dts_check_eq("livebox_release_lb_pixmap", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void set_visibility_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_ne("livebox_set_visibility", ret, LB_STATUS_SUCCESS, "visibility_set");
	} else {
		ret = livebox_set_visibility(handle, LB_SHOW);
		dts_check_eq("livebox_set_visibility", ret, LB_STATUS_SUCCESS, "visibility_set");
	}
}

static void utc_livebox_set_visibility_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, set_visibility_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_set_visibility", handle, NULL, "create for set_visibility");
	}
}

static void utc_livebox_set_visibility_n(void)
{
	int ret;
	ret = livebox_set_visibility(NULL, LB_SHOW);
	dts_check_eq("livebox_set_visibility", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void visibility_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_ne("livebox_visibility", ret, LB_STATUS_SUCCESS, "visibility");
	} else {
		ret = livebox_visibility(handle);
		dts_check_eq("livebox_visibility", ret, LB_HIDE_WITH_PAUSE, "visibility_set");
	}
}

static void utc_livebox_visibility_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, visibility_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_visibility", handle, NULL, "create for visibility");
	}
}

static void utc_livebox_visibility_n(void)
{
	int ret;

	ret = livebox_visibility(NULL);
	dts_check_eq("livebox_visibility", ret, LB_VISIBLE_ERROR, "invalid");
}

static void utc_livebox_set_update_mode_n(void)
{
	int ret;

	ret = livebox_set_update_mode(NULL, 0, NULL, NULL);
	dts_check_eq("livebox_set_update_mode", ret,LB_STATUS_ERROR_INVALID, "invalid");
}

static void set_update_mode_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_set_update_mode", ret, LB_STATUS_SUCCESS, "set_update_mode");
	} else {
		ret = livebox_set_update_mode(handle, 0, NULL, NULL);
		dts_check_eq("livebox_set_update_mode", ret, LB_STATUS_ERROR_ALREADY, "set_update_mode");
	}
}

static void utc_livebox_set_update_mode_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, set_update_mode_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_set_update_mode", handle, NULL, "create for set_update_mode");
	}
}

static void utc_livebox_is_active_update_n(void)
{
	int ret;
	ret = livebox_is_active_update(NULL);
	dts_check_eq("livebox_is_active_update", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void is_active_update_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_is_active_update", ret, LB_STATUS_SUCCESS, "is_activate_update");
	} else {
		ret = livebox_is_active_update(handle);
		dts_check_eq("livebox_is_active_update", ret, 0, "is_activate_update");
	}
}

static void utc_livebox_is_active_update_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, is_active_update_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_is_active_update", handle, NULL, "create for is_active_update");
	}
}

static void utc_livebox_set_manual_sync_n(void)
{
	/*!
	 * \note
	 * Unable to test negative case
	 */
	dts_pass("livebox_set_manual_sync", "negative pass");
}

static void utc_livebox_set_manual_sync_p(void)
{
	livebox_set_option(LB_OPTION_MANUAL_SYNC, 0);
	dts_pass("livebox_set_manual_sync", "Pass!");
}

static void utc_livebox_manual_sync_p(void)
{
	int ret;

	ret = livebox_option(LB_OPTION_MANUAL_SYNC);
	dts_check_eq("livebox_manual_sync", ret, 0, "ok");
}

static void utc_livebox_manual_sync_n(void)
{
	/*!
	 * \note
	 * Unable to test negative case
	 */
	dts_pass("livebox_manual_sync", "negative Pass");
}

static void utc_livebox_set_frame_drop_for_resizing_n(void)
{
	/*!
	 * \note
	 * Unable to test negative case
	 */
	dts_pass("livebox_set_frame_drop_for_resizing", "pass");
}

static void utc_livebox_set_frame_drop_for_resizing_p(void)
{
	livebox_set_option(LB_OPTION_FRAME_DROP_FOR_RESIZE, 1);
	dts_pass("livebox_set_frame_drop_for_resizing", "pass");
}

static void utc_livebox_frame_drop_for_resizing_n(void)
{
	/*!
	 * \note
	 * Unable to test negative case
	 */
	dts_pass("livebox_frame_drop_for_resizing", "pass");
}

static void utc_livebox_frame_drop_for_resizing_p(void)
{
	int ret;

	ret = livebox_option(LB_OPTION_FRAME_DROP_FOR_RESIZE);
	dts_check_eq("livebox_frame_drop_for_resizing", ret, 1, "drop_for_resizing");
}

static void utc_livebox_sync_pd_fb_n(void)
{
	int ret;
	ret = livebox_sync_pd_fb(NULL);
	dts_check_eq("livebox_sync_pd_fb", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void sync_pd_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_sync_pd_fb", ret, LB_STATUS_SUCCESS, "sync_pd_fb");
	} else {
		ret = livebox_sync_pd_fb(handle);
		dts_check_eq("livebox_sync_pd_fb", ret, LB_STATUS_SUCCESS, "sync_pd_fb");
	}
}

static void utc_livebox_sync_pd_fb_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, sync_pd_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_sync_pd_fb", handle, NULL, "create for sync_pd_fb");
	}
}

static void sync_lb_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_sync_lb_fb", ret, LB_STATUS_SUCCESS, "sync_lb_fb");
	} else {
		ret = livebox_sync_lb_fb(handle);
		dts_check_eq("livebox_sync_lb_fb", ret, LB_STATUS_SUCCESS, "sync_lb_fb");
	}
}

static void utc_livebox_sync_lb_fb_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, sync_lb_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_sync_pd_fb", handle, NULL, "create for sync_pd_fb");
	}
}

static void utc_livebox_sync_lb_fb_n(void)
{
	int ret;
	ret = livebox_sync_lb_fb(NULL);
	dts_check_eq("livebox_sync_lb_fb", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void alt_icon_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_alt_name", ret, LB_STATUS_SUCCESS, "alt_name");
	} else {
		const char *alt_icon;

		alt_icon = livebox_alt_icon(handle);
		dts_check_ne("livebox_alt_icon", alt_icon, NULL, "alt_icon");
	}
}

static void utc_livebox_alt_icon_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, alt_icon_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_alt_icon", handle, NULL, "alt_icon");
	}
}

static void utc_livebox_alt_icon_n(void)
{
	const char *alt_icon;

	alt_icon = livebox_alt_icon(NULL);
	dts_check_eq("livebox_alt_icon", alt_icon, NULL, "invalid");
}

static void alt_name_create_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_alt_name", ret, LB_STATUS_SUCCESS, "alt_name");
	} else {
		const char *alt_name;

		alt_name = livebox_alt_name(handle);
		dts_check_ne("livebox_alt_name", alt_name, NULL, "alt_name");
	}
}

static void utc_livebox_alt_name_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(MUSIC_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, alt_name_create_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_alt_name", handle, NULL, "alt_name");
	}
}

static void utc_livebox_alt_name_n(void)
{
	const char *alt_name;
	alt_name = livebox_alt_name(NULL);
	dts_check_eq("livebox_alt_icon", alt_name, NULL, "invalid");
}

static void utc_livebox_acquire_fb_lock_n(void)
{
	int ret;
	ret = livebox_acquire_fb_lock(NULL, 0);
	dts_check_eq("livebox_acquire_fb_lock", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void acquire_fb_lock_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_acquire_fb_lock", ret, LB_STATUS_SUCCESS, "acquire_fb_lock");
	} else {
		ret = livebox_acquire_fb_lock(handle, 0);
		dts_check_eq("livebox_acquire_fb_lock", ret, LB_STATUS_SUCCESS, "acquire_fb_lock");
		(void)livebox_release_fb_lock(handle, 0);
	}
}

static void utc_livebox_acquire_fb_lock_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, acquire_fb_lock_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_acquire_fb_lock", handle, NULL, "create for acquire_fb_lock");
	}
}

static void utc_livebox_release_fb_lock_n(void)
{
	int ret;
	ret = livebox_release_fb_lock(NULL, 0);
	dts_check_eq("livebox_release_fb_lock", ret, LB_STATUS_ERROR_INVALID, "invalid");
}

static void release_fb_lock_cb(struct livebox *handle, int ret, void *data)
{
	if (ret != LB_STATUS_SUCCESS) {
		dts_check_eq("livebox_release_fb_lock", ret, LB_STATUS_SUCCESS, "release_fb_lock");
	} else {
		ret = livebox_acquire_fb_lock(handle, 0);
		if (ret != LB_STATUS_SUCCESS) {
			dts_check_eq("livebox_release_fb_lock", ret, LB_STATUS_SUCCESS, "release_fb_lock");
			return;
		}
	
		ret = livebox_release_fb_lock(handle, 0);
		dts_check_eq("livebox_acquire_fb_lock", ret, LB_STATUS_SUCCESS, "release_fb_lock");
	}
}

static void utc_livebox_release_fb_lock_p(void)
{
	struct livebox *handle;

	handle = livebox_add_with_size(EMAIL_LIVEBOX, NULL, NULL, NULL, -1.0f, LB_SIZE_TYPE_1x1, release_fb_lock_cb, NULL);
	if (!handle) {
		dts_check_ne("livebox_release_fb_lock", handle, NULL, "create for release_fb_lock");
	}
}

struct tet_testlist tet_testlist[] = {
	{ utc_livebox_client_paused_n, NEGATIVE_TC_IDX },
	{ utc_livebox_client_paused_p, POSITIVE_TC_IDX },
	{ utc_livebox_client_resumed_n, NEGATIVE_TC_IDX },
	{ utc_livebox_client_resumed_p, POSITIVE_TC_IDX },
	{ utc_livebox_add_p, POSITIVE_TC_IDX },
	{ utc_livebox_add_n, NEGATIVE_TC_IDX },
	{ utc_livebox_add_with_size_n, NEGATIVE_TC_IDX },
	{ utc_livebox_add_with_size_p, POSITIVE_TC_IDX },
	{ utc_livebox_del_n, NEGATIVE_TC_IDX },
	{ utc_livebox_del_p, POSITIVE_TC_IDX },
	{ utc_livebox_del_NEW_n, NEGATIVE_TC_IDX },
	{ utc_livebox_del_NEW_p, POSITIVE_TC_IDX },
	{ utc_livebox_set_event_handler_n, NEGATIVE_TC_IDX },
	{ utc_livebox_set_event_handler_p, POSITIVE_TC_IDX },
	{ utc_livebox_unset_event_handler_n, NEGATIVE_TC_IDX },
	{ utc_livebox_unset_event_handler_p, POSITIVE_TC_IDX },
	{ utc_livebox_set_fault_handler_n, NEGATIVE_TC_IDX },
	{ utc_livebox_set_fault_handler_p, POSITIVE_TC_IDX },
	{ utc_livebox_unset_fault_handler_n, NEGATIVE_TC_IDX },
	{ utc_livebox_unset_fault_handler_p, POSITIVE_TC_IDX },
	{ utc_livebox_activate_n, NEGATIVE_TC_IDX },
	{ utc_livebox_activate_p, POSITIVE_TC_IDX },
	{ utc_livebox_resize_n, NEGATIVE_TC_IDX },
	{ utc_livebox_resize_p, POSITIVE_TC_IDX },
	{ utc_livebox_click_n, NEGATIVE_TC_IDX },
	{ utc_livebox_click_p, POSITIVE_TC_IDX },
	{ utc_livebox_set_group_n, NEGATIVE_TC_IDX },
	{ utc_livebox_set_group_p, POSITIVE_TC_IDX },
	{ utc_livebox_get_group_n, NEGATIVE_TC_IDX },
	{ utc_livebox_get_group_p, POSITIVE_TC_IDX },
	{ utc_livebox_period_n, NEGATIVE_TC_IDX },
	{ utc_livebox_period_p, POSITIVE_TC_IDX },
	{ utc_livebox_set_period_n, NEGATIVE_TC_IDX },
	{ utc_livebox_set_period_p, POSITIVE_TC_IDX },
	{ utc_livebox_lb_type_n, NEGATIVE_TC_IDX },
	{ utc_livebox_lb_type_p, POSITIVE_TC_IDX },
	{ utc_livebox_is_user_n, NEGATIVE_TC_IDX },
	{ utc_livebox_is_user_p, POSITIVE_TC_IDX },
	{ utc_livebox_content_n, NEGATIVE_TC_IDX },
	{ utc_livebox_content_p, POSITIVE_TC_IDX },
	{ utc_livebox_category_title_n, NEGATIVE_TC_IDX },
	{ utc_livebox_category_title_p, POSITIVE_TC_IDX },
	{ utc_livebox_filename_n, NEGATIVE_TC_IDX },
	{ utc_livebox_filename_p, POSITIVE_TC_IDX },
	{ utc_livebox_pkgname_n, NEGATIVE_TC_IDX },
	{ utc_livebox_pkgname_p, POSITIVE_TC_IDX },
	{ utc_livebox_priority_n, NEGATIVE_TC_IDX },
	{ utc_livebox_priority_p, POSITIVE_TC_IDX },
	{ utc_livebox_acquire_fb_n, NEGATIVE_TC_IDX },
	{ utc_livebox_acquire_fb_p, POSITIVE_TC_IDX },
	{ utc_livebox_release_fb_n, NEGATIVE_TC_IDX },
	{ utc_livebox_release_fb_p, POSITIVE_TC_IDX },
	{ utc_livebox_fb_refcnt_p, POSITIVE_TC_IDX },
	{ utc_livebox_fb_refcnt_n, NEGATIVE_TC_IDX },
	{ utc_livebox_acquire_pdfb_p, POSITIVE_TC_IDX },
	{ utc_livebox_acquire_pdfb_n, NEGATIVE_TC_IDX },
	{ utc_livebox_release_pdfb_p, POSITIVE_TC_IDX },
	{ utc_livebox_release_pdfb_n, NEGATIVE_TC_IDX },
	{ utc_livebox_pdfb_refcnt_n, NEGATIVE_TC_IDX },
	{ utc_livebox_pdfb_refcnt_p, POSITIVE_TC_IDX },
	{ utc_livebox_size_p, POSITIVE_TC_IDX },
	{ utc_livebox_size_n, NEGATIVE_TC_IDX },
	{ utc_livebox_get_pdsize_n, NEGATIVE_TC_IDX },
	{ utc_livebox_get_pdsize_p, POSITIVE_TC_IDX },
	{ utc_livebox_get_supported_sizes_n, NEGATIVE_TC_IDX },
	{ utc_livebox_get_supported_sizes_p, POSITIVE_TC_IDX },
	{ utc_livebox_lbfb_bufsz_n, NEGATIVE_TC_IDX },
	{ utc_livebox_lbfb_bufsz_p, POSITIVE_TC_IDX },
	{ utc_livebox_pdfb_bufsz_n, NEGATIVE_TC_IDX },
	{ utc_livebox_pdfb_bufsz_p, POSITIVE_TC_IDX },
	{ utc_livebox_content_event_n, NEGATIVE_TC_IDX },
	{ utc_livebox_content_event_p, POSITIVE_TC_IDX },
	{ utc_livebox_mouse_event_n, NEGATIVE_TC_IDX },
	{ utc_livebox_mouse_event_p, POSITIVE_TC_IDX },
	{ utc_livebox_access_event_n, NEGATIVE_TC_IDX },
	{ utc_livebox_access_event_p, POSITIVE_TC_IDX },
	{ utc_livebox_key_event_n, NEGATIVE_TC_IDX },
	{ utc_livebox_key_event_p, POSITIVE_TC_IDX },
	{ utc_livebox_set_pinup_n, NEGATIVE_TC_IDX },
	{ utc_livebox_set_pinup_p, POSITIVE_TC_IDX },
	{ utc_livebox_is_pinned_up_n, NEGATIVE_TC_IDX },
	{ utc_livebox_is_pinned_up_p, POSITIVE_TC_IDX },
	{ utc_livebox_has_pinup_n, NEGATIVE_TC_IDX },
	{ utc_livebox_has_pinup_p, POSITIVE_TC_IDX },
	{ utc_livebox_has_pd_p, POSITIVE_TC_IDX },
	{ utc_livebox_has_pd_n, NEGATIVE_TC_IDX },
	{ utc_livebox_create_pd_n, NEGATIVE_TC_IDX },
	{ utc_livebox_create_pd_p, POSITIVE_TC_IDX },
	{ utc_livebox_create_pd_with_position_n, NEGATIVE_TC_IDX },
	{ utc_livebox_create_pd_with_position_p, POSITIVE_TC_IDX },
	{ utc_livebox_move_pd_n, NEGATIVE_TC_IDX },
	{ utc_livebox_move_pd_p, POSITIVE_TC_IDX },
	{ utc_livebox_destroy_pd_n, NEGATIVE_TC_IDX },
	{ utc_livebox_destroy_pd_p, POSITIVE_TC_IDX },
	{ utc_livebox_pd_is_created_n, NEGATIVE_TC_IDX },
	{ utc_livebox_pd_is_created_p, POSITIVE_TC_IDX },
	{ utc_livebox_pd_type_n, NEGATIVE_TC_IDX },
	{ utc_livebox_pd_type_p, POSITIVE_TC_IDX },
	{ utc_livebox_is_exists_n, NEGATIVE_TC_IDX },
	{ utc_livebox_is_exists_p, POSITIVE_TC_IDX },
	{ utc_livebox_set_text_handler_n, NEGATIVE_TC_IDX },
	{ utc_livebox_set_text_handler_p, POSITIVE_TC_IDX },
	{ utc_livebox_set_pd_text_handler_n, NEGATIVE_TC_IDX },
	{ utc_livebox_set_pd_text_handler_p, POSITIVE_TC_IDX },
	{ utc_livebox_emit_text_signal_n, NEGATIVE_TC_IDX },
	{ utc_livebox_emit_text_signal_p, POSITIVE_TC_IDX },
	{ utc_livebox_set_data_n, NEGATIVE_TC_IDX },
	{ utc_livebox_set_data_p, POSITIVE_TC_IDX },
	{ utc_livebox_get_data_p, POSITIVE_TC_IDX },
	{ utc_livebox_get_data_n, NEGATIVE_TC_IDX },
	{ utc_livebox_subscribe_group_n, NEGATIVE_TC_IDX },
	{ utc_livebox_subscribe_group_p, POSITIVE_TC_IDX },
	{ utc_livebox_unsubscribe_group_n, NEGATIVE_TC_IDX },
	{ utc_livebox_unsubscribe_group_p, POSITIVE_TC_IDX },
	{ utc_livebox_refresh_group_n, NEGATIVE_TC_IDX },
	{ utc_livebox_refresh_group_p, POSITIVE_TC_IDX },
	{ utc_livebox_refresh_p, POSITIVE_TC_IDX },
	{ utc_livebox_refresh_n, NEGATIVE_TC_IDX },
	{ utc_livebox_lb_pixmap_n, NEGATIVE_TC_IDX },
	{ utc_livebox_lb_pixmap_p, POSITIVE_TC_IDX },
	{ utc_livebox_pd_pixmap_p, POSITIVE_TC_IDX },
	{ utc_livebox_pd_pixmap_n, NEGATIVE_TC_IDX },
	{ utc_livebox_acquire_pd_pixmap_n, NEGATIVE_TC_IDX },
	{ utc_livebox_acquire_pd_pixmap_p, POSITIVE_TC_IDX },
	{ utc_livebox_release_pd_pixmap_n, NEGATIVE_TC_IDX },
	{ utc_livebox_release_pd_pixmap_p, POSITIVE_TC_IDX },
	{ utc_livebox_acquire_lb_pixmap_p, POSITIVE_TC_IDX },
	{ utc_livebox_acquire_lb_pixmap_n, NEGATIVE_TC_IDX },
	{ utc_livebox_release_lb_pixmap_p, POSITIVE_TC_IDX },
	{ utc_livebox_release_lb_pixmap_n, NEGATIVE_TC_IDX },
	{ utc_livebox_set_visibility_p, POSITIVE_TC_IDX },
	{ utc_livebox_set_visibility_n, NEGATIVE_TC_IDX },
	{ utc_livebox_visibility_p, POSITIVE_TC_IDX },
	{ utc_livebox_visibility_n, NEGATIVE_TC_IDX },
	{ utc_livebox_set_update_mode_n, NEGATIVE_TC_IDX },
	{ utc_livebox_set_update_mode_p, POSITIVE_TC_IDX },
	{ utc_livebox_is_active_update_n, NEGATIVE_TC_IDX },
	{ utc_livebox_is_active_update_p, POSITIVE_TC_IDX },
	{ utc_livebox_set_manual_sync_n, NEGATIVE_TC_IDX },
	{ utc_livebox_set_manual_sync_p, POSITIVE_TC_IDX },
	{ utc_livebox_manual_sync_p, POSITIVE_TC_IDX },
	{ utc_livebox_manual_sync_n, NEGATIVE_TC_IDX },
	{ utc_livebox_set_frame_drop_for_resizing_n, NEGATIVE_TC_IDX },
	{ utc_livebox_set_frame_drop_for_resizing_p, POSITIVE_TC_IDX },
	{ utc_livebox_frame_drop_for_resizing_n, NEGATIVE_TC_IDX },
	{ utc_livebox_frame_drop_for_resizing_p, POSITIVE_TC_IDX },
	{ utc_livebox_sync_pd_fb_n, NEGATIVE_TC_IDX },
	{ utc_livebox_sync_pd_fb_p, POSITIVE_TC_IDX },
	{ utc_livebox_sync_lb_fb_p, POSITIVE_TC_IDX },
	{ utc_livebox_sync_lb_fb_n, NEGATIVE_TC_IDX },
	{ utc_livebox_alt_icon_p, POSITIVE_TC_IDX },
	{ utc_livebox_alt_icon_n, NEGATIVE_TC_IDX },
	{ utc_livebox_alt_name_p, POSITIVE_TC_IDX },
	{ utc_livebox_alt_name_n, NEGATIVE_TC_IDX },
	{ utc_livebox_acquire_fb_lock_n, NEGATIVE_TC_IDX },
	{ utc_livebox_acquire_fb_lock_p, POSITIVE_TC_IDX },
	{ utc_livebox_release_fb_lock_n, NEGATIVE_TC_IDX },
	{ utc_livebox_release_fb_lock_p, POSITIVE_TC_IDX },
	{ NULL, 0 },
};

