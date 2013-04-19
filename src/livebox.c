/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
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

#include <stdio.h>
#include <errno.h>
#include <stdlib.h> /* malloc */
#include <string.h> /* strdup */
#include <math.h>

#include <aul.h>
#include <dlog.h>

#include <com-core_packet.h>
#include <packet.h>
#include <livebox-service.h>
#include <livebox-errno.h>

#include "debug.h"
#include "fb.h"
#include "livebox.h"
#include "livebox_internal.h"
#include "dlist.h"
#include "util.h"
#include "master_rpc.h"
#include "client.h"
#include "critical_log.h"

#define EAPI __attribute__((visibility("default")))
#define MINIMUM_EVENT	s_info.event_filter

#if defined(FLOG)
FILE *__file_log_fp;
#endif

static struct info {
	struct dlist *livebox_list;
	struct dlist *event_list;
	struct dlist *fault_list;
	int init_count;
	int prevent_overwrite;
	double event_filter;
} s_info = {
	.livebox_list = NULL,
	.event_list = NULL,
	.fault_list = NULL,
	.init_count = 0,
	.prevent_overwrite = 0,
	.event_filter = 0.02f,
};

struct cb_info {
	ret_cb_t cb;
	void *data;
};

struct event_info {
	int (*handler)(struct livebox *handler, enum livebox_event_type event, void *data);
	void *user_data;
};

struct fault_info {
	int (*handler)(enum livebox_fault_type event, const char *pkgname, const char *filename, const char *func, void *data);
	void *user_data;
};

static inline void default_create_cb(struct livebox *handler, int ret, void *data)
{
	DbgPrint("Default created event handler: %d\n", ret);
}

static inline void default_delete_cb(struct livebox *handler, int ret, void *data)
{
	DbgPrint("Default deleted event handler: %d\n", ret);
}

static inline void default_pinup_cb(struct livebox *handler, int ret, void *data)
{
	DbgPrint("Default pinup event handler: %d\n", ret);
}

static inline void default_group_changed_cb(struct livebox *handler, int ret, void *data)
{
	DbgPrint("Default group changed event handler: %d\n", ret);
}

static inline void default_period_changed_cb(struct livebox *handler, int ret, void *data)
{
	DbgPrint("Default period changed event handler: %d\n", ret);
}

static inline void default_pd_created_cb(struct livebox *handler, int ret, void *data)
{
	DbgPrint("Default PD created event handler: %d\n", ret);
}

static inline void default_pd_destroyed_cb(struct livebox *handler, int ret, void *data)
{
	DbgPrint("Default PD destroyed event handler: %d\n", ret);
}

static inline void default_lb_size_changed_cb(struct livebox *handler, int ret, void *data)
{
	DbgPrint("Default LB size changed event handler: %d\n", ret);
}

static inline void default_update_mode_cb(struct livebox *handler, int ret, void *data)
{
	DbgPrint("Default update mode set event handler: %d\n", ret);
}

static inline void default_access_event_cb(struct livebox *handler, int ret, void *data)
{
	DbgPrint("Default access event handler: %d\n", ret);
}

static inline __attribute__((always_inline)) struct cb_info *create_cb_info(ret_cb_t cb, void *data)
{
	struct cb_info *info;

	info = malloc(sizeof(*info));
	if (!info) {
		CRITICAL_LOG("Heap: %s\n", strerror(errno));
		return NULL;
	}

	info->cb = cb;
	info->data = data;
	return info;
}

static inline void destroy_cb_info(struct cb_info *info)
{
	free(info);
}

static void update_mode_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;

	if (!result) {
		ret = LB_STATUS_ERROR_FAULT;
		goto errout;
	} else if (packet_get(result, "i", &ret) != 1) {
		ErrPrint("Invalid argument\n");
		ret = LB_STATUS_ERROR_INVALID;
		goto errout;
	}

	if (ret < 0) {
		DbgPrint("Resize request is failed: %d\n", ret);
		goto errout;
	}

	return;

errout:
	handler->update_mode_cb(handler, ret, handler->update_mode_cbdata);
	handler->update_mode_cb = NULL;
	handler->update_mode_cbdata = NULL;
	return;
}

static void resize_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;
	struct cb_info *info = data;
	ret_cb_t cb;
	void *cbdata;

	cb = info->cb;
	cbdata = info->data;
	destroy_cb_info(info);

	if (!result) {
		ret = LB_STATUS_ERROR_FAULT;
		goto errout;
	} else if (packet_get(result, "i", &ret) != 1) {
		ErrPrint("Invalid argument\n");
		ret = LB_STATUS_ERROR_INVALID;
		goto errout;
	}

	/*!
	 * \note
	 * In case of resize request,
	 * The livebox handler will not have resized value right after this callback,
	 * It can only get the new size when it makes updates.
	 *
	 * So the user can only get the resized value(result) from the first update event
	 * after this request.
	 */
	if (ret < 0) {
		DbgPrint("Resize request is failed: %d\n", ret);
		goto errout;
	}

	DbgPrint("Resize request is successfully sent\n");
	return;

errout:
	handler->size_changed_cb(handler, ret, handler->size_cbdata);
	handler->size_changed_cb = NULL;
	handler->size_cbdata = NULL;
}

static void text_signal_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;
	void *cbdata;
	struct cb_info *info = data;
	ret_cb_t cb;

	cbdata = info->data;
	cb = info->cb;
	destroy_cb_info(info);

	if (!result) {
		ret = LB_STATUS_ERROR_FAULT;
	} else if (packet_get(result, "i", &ret) != 1) {
		ErrPrint("Invalid argument\n");
		ret = LB_STATUS_ERROR_INVALID;
	}

	if (cb)
		cb(handler, ret, cbdata);
	return;
}

static void set_group_ret_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;

	if (!result) {
		ret = LB_STATUS_ERROR_FAULT;
		goto errout;
	} else if (packet_get(result, "i", &ret) != 1) {
		ErrPrint("Invalid argument\n");
		ret = LB_STATUS_ERROR_INVALID;
		goto errout;
	}

	if (ret < 0)
		goto errout;

	return;

errout:
	handler->group_changed_cb(handler, ret, handler->group_cbdata);
	handler->group_changed_cb = NULL;
	handler->group_cbdata = NULL;
}

static void period_ret_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;

	if (!result) {
		ret = LB_STATUS_ERROR_FAULT;
		goto errout;
	} else if (packet_get(result, "i", &ret) != 1) {
		ErrPrint("Invalid argument\n");
		ret = LB_STATUS_ERROR_INVALID;
		goto errout;
	}

	if (ret < 0)
		goto errout;

	DbgPrint("Succeed to send period change request, waiting result\n");
	return;

errout:
	handler->period_changed_cb(handler, ret, handler->period_cbdata);
	handler->period_changed_cb = NULL;
	handler->period_cbdata = NULL;
}

static void del_ret_cb(struct livebox *handler, const struct packet *result, void *data)
{
	struct cb_info *info = data;
	int ret;
	ret_cb_t cb;
	void *cbdata;

	cb = info->cb;
	cbdata = info->data;
	destroy_cb_info(info);

	if (!result) {
		ErrPrint("Connection lost?\n");
		ret = LB_STATUS_ERROR_FAULT;
	} else if (packet_get(result, "i", &ret) != 1) {
		ErrPrint("Invalid argument\n");
		ret = LB_STATUS_ERROR_INVALID;
	}

	if (ret == 0) {
		DbgPrint("Returns %d (waiting deleted event)\n", ret);
		handler->deleted_cb = cb;
		handler->deleted_cbdata = cbdata;
	} else if (cb) {
		cb(handler, ret, cbdata);
	}

	/*!
	 * \note
	 * Do not call the deleted callback from here.
	 * master will send the "deleted" event.
	 * Then invoke this callback.
	 *
	 * if (handler->deleted_cb)
	 * 	handler->deleted_cb(handler, ret, handler->deleted_cbdata);
	 */
}

static void new_ret_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;
	struct cb_info *info = data;
	ret_cb_t cb;
	void *cbdata;

	cb = info->cb;
	cbdata = info->data;
	destroy_cb_info(info);

	if (!result) {
		ret = LB_STATUS_ERROR_FAULT;
	} else if (packet_get(result, "i", &ret) != 1) {
		ret = LB_STATUS_ERROR_INVALID;
	}

	if (ret >= 0) {
		DbgPrint("new request is sent, just waiting the created event\n");
		handler->created_cb = cb;
		handler->created_cbdata = cbdata;

		/*!
		 * \note
		 * Don't go anymore ;)
		 */
		return;
	} else if (cb) {
		/*!
		 * \note
		 * It means the current instance is not created,
		 * so user has to know about this.
		 * notice it to user using "deleted" event.
		 */
		cb(handler, ret, cbdata);
	}

	lb_unref(handler);
}

static void pd_create_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;

	if (!result) {
		ret = LB_STATUS_ERROR_FAULT;
		goto errout;
	} else if (packet_get(result, "i", &ret) != 1) {
		ret = LB_STATUS_ERROR_INVALID;
		goto errout;
	}

	if (ret < 0) {
		DbgPrint("Failed to create a PD\n");
		goto errout;
	}

	return;

errout:
	handler->pd_created_cb(handler, ret, handler->pd_created_cbdata);
	handler->pd_created_cb = NULL;
	handler->pd_created_cbdata = NULL;
}

static void activated_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;
	struct cb_info *info = data;
	void *cbdata;
	ret_cb_t cb;
	const char *pkgname = "";

	cbdata = info->data;
	cb = info->cb;
	destroy_cb_info(info);

	if (!result) {
		ret = LB_STATUS_ERROR_FAULT;
	} else if (packet_get(result, "is", &ret, &pkgname) != 2) {
		ret = LB_STATUS_ERROR_INVALID;
	}

	if (cb)
		cb(handler, ret, cbdata);
}

static void pd_destroy_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;
	ret_cb_t cb;
	void *cbdata;
	struct cb_info *info = data;

	cbdata = info->data;
	cb = info->cb;
	destroy_cb_info(info);

	if (!result) {
		DbgPrint("Result is NIL (may connection lost)\n");
		ret = LB_STATUS_ERROR_FAULT;
	} else if (packet_get(result, "i", &ret) != 1) {
		DbgPrint("Invalid parameter\n");
		ret = LB_STATUS_ERROR_INVALID;
	}

	if (ret == 0) {
		DbgPrint("PD Destroyed callback prepared\n");
		handler->pd_destroyed_cb = cb;
		handler->pd_destroyed_cbdata = cbdata;
	} else if (cb) {
		DbgPrint("PD is not desroyed (forcely reset, pd flag)\n");
		handler->is_pd_created = 0;
		cb(handler, ret, cbdata);
	}
}

static void delete_cluster_cb(struct livebox *handler, const struct packet *result, void *data)
{
	struct cb_info *info = data;
	int ret;
	ret_cb_t cb;
	void *cbdata;

	cb = info->cb;
	cbdata = info->data;
	destroy_cb_info(info);

	if (!result) {
		ret = LB_STATUS_ERROR_FAULT;
	} else if (packet_get(result, "i", &ret) != 1) {
		ret = LB_STATUS_ERROR_INVALID;
	}

	DbgPrint("Delete category returns: %d\n", ret);

	if (cb)
		cb(handler, ret, cbdata);
}

static void delete_category_cb(struct livebox *handler, const struct packet *result, void *data)
{
	struct cb_info *info = data;
	int ret;
	ret_cb_t cb;
	void *cbdata;

	cb = info->cb;
	cbdata = info->data;
	destroy_cb_info(info);

	if (!result)
		ret = LB_STATUS_ERROR_FAULT;
	else if (packet_get(result, "i", &ret) != 1)
		ret = LB_STATUS_ERROR_INVALID;

	DbgPrint("Delete category returns: %d\n", ret);

	if (cb)
		cb(handler, ret, cbdata);
}

static void pixmap_acquired_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;
	ret_cb_t cb;
	void *cbdata;
	struct cb_info *info = data;

	cb = info->cb;
	cbdata = info->data;
	destroy_cb_info(info);

	if (!result)
		ret = 0; /* PIXMAP 0 means error */
	else if (packet_get(result, "i", &ret) != 1)
		ret = 0;

	if (cb)
		cb(handler, ret, cbdata);
}

static void pinup_done_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;

	if (!result) {
		ret = LB_STATUS_ERROR_FAULT;
		goto errout;
	} else if (packet_get(result, "i", &ret) != 1) {
		goto errout;
	}

	if (ret < 0)
		goto errout;

	return;

errout:
	handler->pinup_cb(handler, ret, handler->pinup_cbdata);
	handler->pinup_cb = NULL;
	handler->pinup_cbdata = NULL;
}

static void access_ret_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;

	if (!result) {
		ret = LB_STATUS_ERROR_FAULT;
		return;
	}

	if (packet_get(result, "i", &ret) != 1) {
		ret = LB_STATUS_ERROR_INVALID;
		return;
	}

	if (ret != LB_STATUS_SUCCESS) {
		goto errout;
	}

	DbgPrint("Access event is successfully sent. waiting result\n");
	return;

errout:
	handler->access_event_cb(handler, ret, handler->access_event_cbdata);
	handler->access_event_cb = NULL;
	handler->access_event_cbdata = NULL;
	return;
}

static int send_access_event(struct livebox *handler, const char *event, int x, int y)
{
	struct packet *packet;
	double timestamp;

	timestamp = util_timestamp();

	packet = packet_create(event, "ssdii", handler->pkgname, handler->id, timestamp, x, y);
	if (!packet) {
		ErrPrint("Failed to build packet\n");
		return LB_STATUS_ERROR_FAULT;
	}

	DbgPrint("Send: %dx%d\n", x, y);

	return master_rpc_async_request(handler, packet, 0, access_ret_cb, NULL);
}

static int send_mouse_event(struct livebox *handler, const char *event, int x, int y)
{
	struct packet *packet;
	double timestamp;

	timestamp = util_timestamp();
	packet = packet_create_noack(event, "ssdii", handler->pkgname, handler->id, timestamp, x, y);
	if (!packet) {
		ErrPrint("Failed to build param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	DbgPrint("Send: %dx%d\n", x, y);

	return master_rpc_request_only(handler, packet);
}

EAPI int livebox_init(void *disp)
{
	const char *env;

	if (s_info.init_count > 0) {
		s_info.init_count++;
		return LB_STATUS_SUCCESS;
	}
	env = getenv("PROVIDER_DISABLE_PREVENT_OVERWRITE");
	if (env && !strcasecmp(env, "true"))
		s_info.prevent_overwrite = 1;

	env = getenv("PROVIDER_EVENT_FILTER");
	if (env)
		sscanf(env, "%lf", &MINIMUM_EVENT);

#if defined(FLOG)
	char filename[BUFSIZ];
	snprintf(filename, sizeof(filename), "/tmp/%d.box.log", getpid());
	__file_log_fp = fopen(filename, "w+t");
	if (!__file_log_fp)
		__file_log_fp = fdopen(1, "w+t");
#endif
	critical_log_init("viewer");
	livebox_service_init();
	fb_init(disp);

	client_init();

	s_info.init_count++;
	return LB_STATUS_SUCCESS;
}

EAPI int livebox_fini(void)
{
	if (s_info.init_count <= 0) {
		DbgPrint("Didn't initialized\n");
		return LB_STATUS_ERROR_INVALID;
	}

	s_info.init_count--;
	if (s_info.init_count > 0) {
		DbgPrint("init count : %d\n", s_info.init_count);
		return LB_STATUS_SUCCESS;
	}

	client_fini();
	fb_fini();
	livebox_service_fini();
	critical_log_fini();
	return LB_STATUS_SUCCESS;
}

static inline char *lb_pkgname(const char *pkgname)
{
	char *lb;

	lb = livebox_service_pkgname(pkgname);
	if (!lb) {
		if (util_validate_livebox_package(pkgname) == 0)
			return strdup(pkgname);
	}

	return lb;
}

/*!
 * Just wrapping the livebox_add_with_size function.
 */
EAPI struct livebox *livebox_add(const char *pkgname, const char *content, const char *cluster, const char *category, double period, ret_cb_t cb, void *data)
{
	return livebox_add_with_size(pkgname, content, cluster, category, period, LB_SIZE_TYPE_UNKNOWN, cb, data);
}

EAPI struct livebox *livebox_add_with_size(const char *pkgname, const char *content, const char *cluster, const char *category, double period, int type, ret_cb_t cb, void *data)
{
	struct livebox *handler;
	struct packet *packet;
	int ret;
	int width = 0;
	int height = 0;
	struct cb_info *cbinfo;

	if (!pkgname || !cluster || !category) {
		ErrPrint("Invalid arguments: pkgname[%p], cluster[%p], category[%p]\n",
								pkgname, cluster, category);
		return NULL;
	}

	if (type != LB_SIZE_TYPE_UNKNOWN)
		livebox_service_get_size(type, &width, &height);

	handler = calloc(1, sizeof(*handler));
	if (!handler) {
		ErrPrint("Error: %s\n", strerror(errno));
		return NULL;
	}

	handler->pkgname = lb_pkgname(pkgname);
	if (!handler->pkgname) {
		free(handler);
		return NULL;
	}

	if (livebox_service_is_enabled(handler->pkgname) == 0) {
		DbgPrint("Livebox [%s](%s) is disabled package\n", handler->pkgname, pkgname);
		free(handler->pkgname);
		free(handler);
		return NULL;
	}

	if (content) {
		handler->content = strdup(content);
		if (!handler->content) {
			ErrPrint("Error: %s\n", strerror(errno));
			free(handler->pkgname);
			free(handler);
			return NULL;
		}
	} else {
		handler->content = livebox_service_content(handler->pkgname);
	}

	handler->cluster = strdup(cluster);
	if (!handler->cluster) {
		ErrPrint("Error: %s\n", strerror(errno));
		free(handler->content);
		free(handler->pkgname);
		free(handler);
		return NULL;
	}

	handler->category = strdup(category);
	if (!handler->category) {
		ErrPrint("Error: %s\n", strerror(errno));
		free(handler->cluster);
		free(handler->content);
		free(handler->pkgname);
		free(handler);
		return NULL;
	}

	if (!cb)
		cb = default_create_cb;

	/* Data provider will set this */
	handler->lb.type = _LB_TYPE_FILE;
	handler->pd.type = _PD_TYPE_SCRIPT;
	handler->lb.period = period;

	/* Used for handling the mouse event on a box */
	handler->lb.mouse_event = livebox_service_mouse_event(handler->pkgname);

	/* Cluster infomration is not determined yet */
	handler->nr_of_sizes = 0x01;

	handler->timestamp = util_timestamp();
	handler->is_user = 1;
	handler->visible = LB_SHOW;

	s_info.livebox_list = dlist_append(s_info.livebox_list, handler);

	packet = packet_create("new", "dssssdii", handler->timestamp, handler->pkgname, handler->content, cluster, category, period, width, height);
	if (!packet) {
		ErrPrint("Failed to create a new packet\n");
		free(handler->category);
		free(handler->cluster);
		free(handler->content);
		free(handler->pkgname);
		free(handler);
		return NULL;
	}

	cbinfo = create_cb_info(cb, data);
	if (!cbinfo) {
		ErrPrint("Failed to create a cbinfo\n");
		packet_destroy(packet);
		free(handler->category);
		free(handler->cluster);
		free(handler->content);
		free(handler->pkgname);
		free(handler);
		return NULL;
	}

	ret = master_rpc_async_request(handler, packet, 0, new_ret_cb, cbinfo);
	if (ret < 0) {
		ErrPrint("Failed to send a new packet\n");
		destroy_cb_info(cbinfo);
		free(handler->category);
		free(handler->cluster);
		free(handler->content);
		free(handler->pkgname);
		free(handler);
		return NULL;
	}

	DbgPrint("Successfully sent a new request ([%lf] %s)\n", handler->timestamp, handler->pkgname);
	handler->state = CREATE;
	return lb_ref(handler);
}

EAPI double livebox_period(struct livebox *handler)
{
	if (!handler || handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return 0.0f;
	}

	return handler->lb.period;
}

EAPI int livebox_set_period(struct livebox *handler, double period, ret_cb_t cb, void *data)
{
	struct packet *packet;
	int ret;

	if (!handler || handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->period_changed_cb) {
		ErrPrint("Previous request for changing period is not finished\n");
		return LB_STATUS_ERROR_BUSY;
	}

	if (!handler->is_user) {
		ErrPrint("CA Livebox is not able to change the period\n");
		return LB_STATUS_ERROR_PERMISSION;
	}

	if (handler->lb.period == period) {
		DbgPrint("No changes\n");
		return LB_STATUS_ERROR_ALREADY;
	}

	packet = packet_create("set_period", "ssd", handler->pkgname, handler->id, period);
	if (!packet) {
		ErrPrint("Failed to build a packet %s\n", handler->pkgname);
		return LB_STATUS_ERROR_FAULT;
	}

	if (!cb)
		cb = default_period_changed_cb;

	handler->period_changed_cb = cb;
	handler->period_cbdata = data;

	ret = master_rpc_async_request(handler, packet, 0, period_ret_cb, NULL);
	if (ret < 0) {
		handler->period_changed_cb = NULL;
		handler->period_cbdata = NULL;
	}

	return ret;
}

EAPI int livebox_del(struct livebox *handler, ret_cb_t cb, void *data)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE) {
		ErrPrint("Handler is already deleted\n");
		return LB_STATUS_ERROR_INVALID;
	}

	handler->state = DELETE;

	if (!handler->id) {
		/*!
		 * \note
		 * The id is not determined yet.
		 * It means a user didn't receive created event yet.
		 * Then just stop to delete procedure from here.
		 * Because the "created" event handler will release this.
		 * By the way, if the user adds any callback for getting return status of this,
		 * call it at here.
		 */
		if (cb)
			cb(handler, 0, data);
		return LB_STATUS_SUCCESS;
	}

	if (!cb)
		cb = default_delete_cb;

	return lb_send_delete(handler, cb, data);
}

EAPI int livebox_set_fault_handler(int (*cb)(enum livebox_fault_type, const char *, const char *, const char *, void *), void *data)
{
	struct fault_info *info;

	if (!cb)
		return LB_STATUS_ERROR_INVALID;

	info = malloc(sizeof(*info));
	if (!info) {
		CRITICAL_LOG("Heap: %s\n", strerror(errno));
		return LB_STATUS_ERROR_MEMORY;
	}

	info->handler = cb;
	info->user_data = data;

	s_info.fault_list = dlist_append(s_info.fault_list, info);
	return LB_STATUS_SUCCESS;
}

EAPI void *livebox_unset_fault_handler(int (*cb)(enum livebox_fault_type, const char *, const char *, const char *, void *))
{
	struct fault_info *info;
	struct dlist *l;

	dlist_foreach(s_info.fault_list, l, info) {
		if (info->handler == cb) {
			void *data;
			s_info.fault_list = dlist_remove(s_info.fault_list, l);
			data = info->user_data;
			free(info);

			return data;
		}
	}

	return NULL;
}

EAPI int livebox_set_event_handler(int (*cb)(struct livebox *, enum livebox_event_type, void *), void *data)
{
	struct event_info *info;

	if (!cb) {
		ErrPrint("Invalid argument cb is nil\n");
		return LB_STATUS_ERROR_INVALID;
	}

	info = malloc(sizeof(*info));
	if (!info) {
		CRITICAL_LOG("Heap: %s\n", strerror(errno));
		return LB_STATUS_ERROR_MEMORY;
	}

	info->handler = cb;
	info->user_data = data;

	s_info.event_list = dlist_append(s_info.event_list, info);
	return LB_STATUS_SUCCESS;
}

EAPI void *livebox_unset_event_handler(int (*cb)(struct livebox *, enum livebox_event_type, void *))
{
	struct event_info *info;
	struct dlist *l;

	dlist_foreach(s_info.event_list, l, info) {
		if (info->handler == cb) {
			void *data;

			s_info.event_list = dlist_remove(s_info.event_list, l);
			data = info->user_data;
			free(info);

			return data;
		}
	}

	return NULL;
}

EAPI int livebox_set_update_mode(struct livebox *handler, int active_update, ret_cb_t cb, void *data)
{
	struct packet *packet;
	int ret;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id)
		return LB_STATUS_ERROR_INVALID;

	if (handler->update_mode_cb) {
		ErrPrint("Previous update_mode cb is not finished yet\n");
		return LB_STATUS_ERROR_BUSY;
	}

	if (handler->is_active_update == active_update)
		return LB_STATUS_ERROR_ALREADY;

	if (!handler->is_user)
		return LB_STATUS_ERROR_PERMISSION;

	packet = packet_create("update_mode", "ssi", handler->pkgname, handler->id, active_update);
	if (!packet)
		return LB_STATUS_ERROR_FAULT;

	if (!cb)
		cb = default_update_mode_cb;

	handler->update_mode_cb = cb;
	handler->update_mode_cbdata = data;

	ret = master_rpc_async_request(handler, packet, 0, update_mode_cb, NULL);
	if (ret < 0) {
		handler->update_mode_cb = NULL;
		handler->update_mode_cbdata = NULL;
	}

	return ret;
}

EAPI int livebox_is_active_update(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id)
		return LB_STATUS_ERROR_INVALID;

	return handler->is_active_update;
}

EAPI int livebox_resize(struct livebox *handler, int type, ret_cb_t cb, void *data)
{
	struct packet *packet;
	int w;
	int h;
	int ret;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->size_changed_cb) {
		ErrPrint("Previous resize request is not finished yet\n");
		return LB_STATUS_ERROR_BUSY;
	}

	if (!handler->is_user) {
		ErrPrint("CA Livebox is not able to be resized\n");
		return LB_STATUS_ERROR_PERMISSION;
	}

	if (livebox_service_get_size(type, &w, &h) != 0) {
		ErrPrint("Invalid size type\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->lb.width == w && handler->lb.height == h) {
		DbgPrint("No changes\n");
		return LB_STATUS_ERROR_ALREADY;;
	}

	packet = packet_create("resize", "ssii", handler->pkgname, handler->id, w, h);
	if (!packet) {
		ErrPrint("Failed to build param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	if (!cb)
		cb = default_lb_size_changed_cb;

	handler->size_changed_cb = cb;
	handler->size_cbdata = data;

	ret = master_rpc_async_request(handler, packet, 0, resize_cb, NULL);
	if (ret < 0) {
		handler->size_changed_cb = NULL;
		handler->size_cbdata = NULL;
	}

	return ret;
}

EAPI int livebox_click(struct livebox *handler, double x, double y)
{
	struct packet *packet;
	double timestamp;
	int ret;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->lb.auto_launch) {
		DbgPrint("Auto-launch enabled: %s\n", handler->lb.auto_launch);
		if (aul_launch_app(handler->lb.auto_launch, NULL) < 0)
			ErrPrint("Failed to launch app %s\n", handler->lb.auto_launch);
	}

	timestamp = util_timestamp();
	packet = packet_create_noack("clicked", "sssddd", handler->pkgname, handler->id, "clicked", timestamp, x, y);
	if (!packet) {
		ErrPrint("Failed to build param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	ret = master_rpc_request_only(handler, packet);
	DbgPrint("Click request: %d\n", ret);

	if (!handler->lb.mouse_event && (handler->lb.type == _LB_TYPE_BUFFER || handler->lb.type == _LB_TYPE_SCRIPT)) {
		int ret; /* Shadow variable */
		ret = send_mouse_event(handler, "lb_mouse_down", x * handler->lb.width, y * handler->lb.height);
		if (ret < 0)
			DbgPrint("Failed to send Down: %d\n", ret);

		ret = send_mouse_event(handler, "lb_mouse_move", x * handler->lb.width, y * handler->lb.height);
		if (ret < 0)
			DbgPrint("Failed to send Move: %d\n", ret);

		ret = send_mouse_event(handler, "lb_mouse_up", x * handler->lb.width, y * handler->lb.height);
		if (ret < 0)
			DbgPrint("Failed to send Up: %d\n", ret);
	}

	return ret;
}

EAPI int livebox_has_pd(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return !!handler->pd.data.fb;
}

EAPI int livebox_pd_is_created(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->pd.data.fb || handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return handler->is_pd_created;
}

EAPI int livebox_create_pd(struct livebox *handler, ret_cb_t cb, void *data)
{
	return livebox_create_pd_with_position(handler, -1.0, -1.0, cb, data);
}

EAPI int livebox_create_pd_with_position(struct livebox *handler, double x, double y, ret_cb_t cb, void *data)
{
	struct packet *packet;
	int ret;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->pd.data.fb || handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->is_pd_created == 1) {
		DbgPrint("PD already created\n");
		return LB_STATUS_SUCCESS;
	}

	if (handler->pd_created_cb) {
		ErrPrint("Previous request is not completed yet\n");
		return LB_STATUS_ERROR_BUSY;
	}

	packet = packet_create("create_pd", "ssdd", handler->pkgname, handler->id, x, y);
	if (!packet) {
		ErrPrint("Failed to build param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	if (!cb)
		cb = default_pd_created_cb;

	handler->pd_created_cb = cb;
	handler->pd_created_cbdata = data;

	ret = master_rpc_async_request(handler, packet, 0, pd_create_cb, NULL);
	if (ret < 0) {
		handler->pd_created_cb = NULL;
		handler->pd_created_cbdata = NULL;
	}

	return ret;
}

EAPI int livebox_move_pd(struct livebox *handler, double x, double y)
{
	struct packet *packet;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->pd.data.fb || handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->is_pd_created) {
		DbgPrint("PD is not created\n");
		return LB_STATUS_ERROR_INVALID;
	}

	packet = packet_create_noack("pd_move", "ssdd", handler->pkgname, handler->id, x, y);
	if (!packet) {
		ErrPrint("Failed to build param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	return master_rpc_request_only(handler, packet);
}

EAPI int livebox_activate(const char *pkgname, ret_cb_t cb, void *data)
{
	struct packet *packet;
	struct cb_info *cbinfo;
	int ret;

	if (!pkgname)
		return LB_STATUS_ERROR_INVALID;

	packet = packet_create("activate_package", "s", pkgname);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	cbinfo = create_cb_info(cb, data);
	if (!cbinfo) {
		ErrPrint("Unable to create cbinfo\n");
		packet_destroy(packet);
		return LB_STATUS_ERROR_FAULT;
	}

	ret = master_rpc_async_request(NULL, packet, 0, activated_cb, cbinfo);
	if (ret < 0)
		destroy_cb_info(cbinfo);

	return ret;
}

EAPI int livebox_destroy_pd(struct livebox *handler, ret_cb_t cb, void *data)
{
	struct packet *packet;
	struct cb_info *cbinfo;
	int ret;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->pd.data.fb || handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->is_pd_created) {
		ErrPrint("PD is not created\n");
		return LB_STATUS_ERROR_INVALID;
	}

	packet = packet_create("destroy_pd", "ss", handler->pkgname, handler->id);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	if (!cb)
		cb = default_pd_destroyed_cb;

	cbinfo = create_cb_info(cb, data);
	if (!cbinfo) {
		packet_destroy(packet);
		return LB_STATUS_ERROR_FAULT;
	}

	ret = master_rpc_async_request(handler, packet, 0, pd_destroy_cb, cbinfo);
	if (ret < 0)
		destroy_cb_info(cbinfo);

	return ret;
}

EAPI int livebox_access_event(struct livebox *handler, enum access_event_type type, double x, double y, ret_cb_t cb, void *data)
{
	int w = 1;
	int h = 1;
	char cmd[32] = { '\0', };
	char *ptr = cmd;
	int ret;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->access_event_cb) {
		ErrPrint("Previous access event is not yet done\n");
		return LB_STATUS_ERROR_BUSY;
	}

	if (type & ACCESS_EVENT_PD_MASK) {
		if (!handler->is_pd_created) {
			ErrPrint("PD is not created\n");
			return LB_STATUS_ERROR_INVALID;
		}
		*ptr++ = 'p';
		*ptr++ = 'd';
		w = handler->pd.width;
		h = handler->pd.height;
	} else if (type & ACCESS_EVENT_LB_MASK) {
		*ptr++ = 'l';
		*ptr++ = 'b';
		w = handler->lb.width;
		h = handler->lb.height;
	} else {
		ErrPrint("Invalid event type\n");
		return LB_STATUS_ERROR_INVALID;
	}

	switch (type & ~ACCESS_EVENT_PD_MASK) {
	case ACCESS_EVENT_HIGHLIGHT:
		strcpy(ptr, "_access_hl");
		break;
	case ACCESS_EVENT_HIGHLIGHT_NEXT:
		strcpy(ptr, "_access_hl_next");
		break;
	case ACCESS_EVENT_HIGHLIGHT_PREV:
		strcpy(ptr, "_access_hl_prev");
		break;
	case ACCESS_EVENT_ACTIVATE:
		strcpy(ptr, "_access_activate");
		break;
	case ACCESS_EVENT_VALUE_CHANGE:
		strcpy(ptr, "_access_value_change");
		break;
	case ACCESS_EVENT_UNHIGHLIGHT:
		strcpy(ptr, "_access_unhighlight");
		break;
	case ACCESS_EVENT_SCROLL_DOWN:
		strcpy(ptr, "_access_scroll_down");
		break;
	case ACCESS_EVENT_SCROLL_MOVE:
		strcpy(ptr, "_access_scroll_move");
		break;
	case ACCESS_EVENT_SCROLL_UP:
		strcpy(ptr, "_access_scroll_up");
		break;
	default:
		return LB_STATUS_ERROR_INVALID;
	}

	if (!cb)
		cb = default_access_event_cb;

	handler->access_event_cb = cb;
	handler->access_event_cbdata = data;

	ret = send_access_event(handler, cmd, x * w, y * h);
	if (ret < 0) {
		handler->access_event_cb = NULL;
		handler->access_event_cbdata = NULL;
	}

	return ret;
}

EAPI int livebox_content_event(struct livebox *handler, enum content_event_type type, double x, double y)
{
	int w = 1;
	int h = 1;
	char cmd[32] = { '\0', };
	char *ptr = cmd;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (type & CONTENT_EVENT_PD_MASK) {
		int flag = 1;

		if (!handler->is_pd_created) {
			ErrPrint("PD is not created\n");
			return LB_STATUS_ERROR_INVALID;
		}

		if (type & CONTENT_EVENT_MOUSE_MASK) {
			if (!handler->pd.data.fb) {
				ErrPrint("Handler is not valid\n");
				return LB_STATUS_ERROR_INVALID;
			}

			if (type & CONTENT_EVENT_MOUSE_MOVE) {
				if (fabs(x - handler->pd.x) < MINIMUM_EVENT && fabs(y - handler->pd.y) < MINIMUM_EVENT)
					return LB_STATUS_ERROR_BUSY;
			} else if (type & CONTENT_EVENT_MOUSE_SET) {
				flag = 0;
			}
		}

		if (flag) {
			w = handler->pd.width;
			h = handler->pd.height;
			handler->pd.x = x;
			handler->pd.y = y;
		}
		*ptr++ = 'p';
		*ptr++ = 'd';
	} else {
		int flag = 1;

		if (type & CONTENT_EVENT_MOUSE_MASK) {
			if (!handler->lb.mouse_event) {
				ErrPrint("Box is not support the mouse event\n");
				return LB_STATUS_ERROR_INVALID;
			}

			if (!handler->lb.data.fb) {
				ErrPrint("Handler is not valid\n");
				return LB_STATUS_ERROR_INVALID;
			}

			if (type & CONTENT_EVENT_MOUSE_MOVE) {
				if (fabs(x - handler->lb.x) < MINIMUM_EVENT && fabs(y - handler->lb.y) < MINIMUM_EVENT)
					return LB_STATUS_ERROR_BUSY;
			} else if (type & CONTENT_EVENT_MOUSE_SET) {
				flag = 0;
			}
		}

		if (flag) {
			w = handler->lb.width;
			h = handler->lb.height;
			handler->lb.x = x;
			handler->lb.y = y;
		}
		*ptr++ = 'l';
		*ptr++ = 'b';
	}

	/*!
	 * Must be short than 29 bytes.
	 */
	switch ((type & ~CONTENT_EVENT_PD_MASK)) {
	case CONTENT_EVENT_MOUSE_ENTER | CONTENT_EVENT_MOUSE_MASK:
		strcpy(ptr, "_mouse_enter");
		break;
	case CONTENT_EVENT_MOUSE_LEAVE | CONTENT_EVENT_MOUSE_MASK:
		strcpy(ptr, "_mouse_leave");
		break;
	case CONTENT_EVENT_MOUSE_UP | CONTENT_EVENT_MOUSE_MASK:
		strcpy(ptr, "_mouse_up");
		break;
	case CONTENT_EVENT_MOUSE_DOWN | CONTENT_EVENT_MOUSE_MASK:
		strcpy(ptr, "_mouse_down");
		break;
	case CONTENT_EVENT_MOUSE_MOVE | CONTENT_EVENT_MOUSE_MASK:
		strcpy(ptr, "_mouse_move");
		break;
	case CONTENT_EVENT_MOUSE_SET | CONTENT_EVENT_MOUSE_MASK:
		strcpy(ptr, "_mouse_set");
		break;
	case CONTENT_EVENT_MOUSE_UNSET | CONTENT_EVENT_MOUSE_MASK:
		strcpy(ptr, "_mouse_unset");
		break;
	case CONTENT_EVENT_KEY_DOWN | CONTENT_EVENT_KEY_MASK:
		strcpy(ptr, "_key_down");
		break;
	case CONTENT_EVENT_KEY_UP | CONTENT_EVENT_KEY_MASK:
		strcpy(ptr, "_key_up");
		break;
	default:
		ErrPrint("Invalid event type\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return send_mouse_event(handler, cmd, x * w, y * h);
}

EAPI const char *livebox_filename(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return NULL;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return NULL;
	}

	if (handler->filename)
		return handler->filename;

	/* Oooops */
	return util_uri_to_path(handler->id);
}

EAPI int livebox_get_pdsize(struct livebox *handler, int *w, int *h)
{
	int _w;
	int _h;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!w)
		w = &_w;
	if (!h)
		h = &_h;

	if (!handler->is_pd_created) {
		DbgPrint("Buffer is not created yet [%dx%d]\n", *w, *h);
		*w = handler->pd.default_width;
		*h = handler->pd.default_height;
	} else {
		*w = handler->pd.width;
		*h = handler->pd.height;
	}

	return LB_STATUS_SUCCESS;
}

EAPI int livebox_size(struct livebox *handler)
{
	int w;
	int h;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	w = handler->lb.width;
	h = handler->lb.height;

	switch (handler->lb.type) {
	case _LB_TYPE_BUFFER:
	case _LB_TYPE_SCRIPT:
		if (!fb_is_created(handler->lb.data.fb)) {
			DbgPrint("Buffer is not created yet - reset size\n");
			w = 0;
			h = 0;
		}
		break;
	default:
		break;
	}

	return livebox_service_size_type(w, h);
}

EAPI int livebox_set_group(struct livebox *handler, const char *cluster, const char *category, ret_cb_t cb, void *data)
{
	struct packet *packet;
	int ret;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!cluster || !category || handler->state != CREATE || !handler->id) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->group_changed_cb) {
		ErrPrint("Previous group changing request is not finished yet\n");
		return LB_STATUS_ERROR_BUSY;
	}

	if (!handler->is_user) {
		ErrPrint("CA Livebox is not able to change the group\n");
		return LB_STATUS_ERROR_PERMISSION;
	}

	if (!strcmp(handler->cluster, cluster) && !strcmp(handler->category, category)) {
		DbgPrint("No changes\n");
		return LB_STATUS_ERROR_ALREADY;
	}

	packet = packet_create("change_group", "ssss", handler->pkgname, handler->id, cluster, category);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	if (!cb)
		cb = default_group_changed_cb;

	handler->group_changed_cb = cb;
	handler->group_cbdata = data; 

	ret = master_rpc_async_request(handler, packet, 0, set_group_ret_cb, NULL);
	if (ret < 0) {
		handler->group_changed_cb = NULL;
		handler->group_cbdata = NULL;
	}

	return ret;
}

EAPI int livebox_get_group(struct livebox *handler, char ** const cluster, char ** const category)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!cluster || !category || handler->state != CREATE || !handler->id) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	*cluster = handler->cluster;
	*category = handler->category;
	return LB_STATUS_SUCCESS;
}

EAPI int livebox_get_supported_sizes(struct livebox *handler, int *cnt, int *size_list)
{
	register int i;
	register int j;

	if (!handler || !size_list) {
		ErrPrint("Invalid argument, handler(%p), size_list(%p)\n", handler, size_list);
		return LB_STATUS_ERROR_INVALID;
	}

	if (!cnt || handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	for (j = i = 0; i < NR_OF_SIZE_LIST; i++) {
		if (handler->lb.size_list & (0x01 << i)) {
			if (j == *cnt)
				break;

			size_list[j++] = (0x01 << i);
		}
	}

	*cnt = j;
	return LB_STATUS_SUCCESS;
}

EAPI const char *livebox_pkgname(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return NULL;
	}

	if (handler->state != CREATE) {
		ErrPrint("Handler is not valid\n");
		return NULL;
	}

	return handler->pkgname;
}

EAPI double livebox_priority(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return 0.0f;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid (%p)\n", handler);
		return -1.0f;
	}

	return handler->lb.priority;
}

EAPI int livebox_delete_cluster(const char *cluster, ret_cb_t cb, void *data)
{
	struct packet *packet;
	struct cb_info *cbinfo;
	int ret;

	packet = packet_create("delete_cluster", "s", cluster);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	cbinfo = create_cb_info(cb, data);
	if (!cbinfo) {
		packet_destroy(packet);
		return LB_STATUS_ERROR_FAULT;
	}

	ret = master_rpc_async_request(NULL, packet, 0, delete_cluster_cb, cbinfo);
	if (ret < 0)
		destroy_cb_info(cbinfo);

	return ret;
}

EAPI int livebox_delete_category(const char *cluster, const char *category, ret_cb_t cb, void *data)
{
	struct packet *packet;
	struct cb_info *cbinfo;
	int ret;

	packet = packet_create("delete_category", "ss", cluster, category);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	cbinfo = create_cb_info(cb, data);
	if (!cbinfo) {
		packet_destroy(packet);
		return LB_STATUS_ERROR_FAULT;
	}

	ret = master_rpc_async_request(NULL, packet, 0, delete_category_cb, cbinfo);
	if (ret < 0)
		destroy_cb_info(cbinfo);

	return ret;
}

EAPI enum livebox_lb_type livebox_lb_type(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_TYPE_INVALID;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return LB_TYPE_INVALID;
	}

	switch (handler->lb.type) {
	case _LB_TYPE_FILE:
		return LB_TYPE_IMAGE;
	case _LB_TYPE_BUFFER:
	case _LB_TYPE_SCRIPT:
		{
			const char *id;
			id = fb_id(handler->lb.data.fb);
			if (id && !strncasecmp(id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP)))
				return LB_TYPE_PIXMAP;
		}
		return LB_TYPE_BUFFER;
	case _LB_TYPE_TEXT:
		return LB_TYPE_TEXT;
	default:
		break;
	}

	return LB_TYPE_INVALID;
}

EAPI enum livebox_pd_type livebox_pd_type(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return PD_TYPE_INVALID;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return PD_TYPE_INVALID;
	}

	switch (handler->pd.type) {
	case _PD_TYPE_TEXT:
		return PD_TYPE_TEXT;
	case _PD_TYPE_BUFFER:
	case _PD_TYPE_SCRIPT:
		{
			const char *id;
			id = fb_id(handler->pd.data.fb);
			if (id && !strncasecmp(id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP)))
				return PD_TYPE_PIXMAP;
		}
		return PD_TYPE_BUFFER;
	default:
		break;
	}

	return PD_TYPE_INVALID;
}

EAPI int livebox_set_pd_text_handler(struct livebox *handler, struct livebox_script_operators *ops)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	memcpy(&handler->pd.data.ops, ops, sizeof(*ops));
	return LB_STATUS_SUCCESS;
}

EAPI int livebox_set_text_handler(struct livebox *handler, struct livebox_script_operators *ops)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	memcpy(&handler->lb.data.ops, ops, sizeof(*ops));
	return LB_STATUS_SUCCESS;
}

EAPI int livebox_acquire_lb_pixmap(struct livebox *handler, ret_cb_t cb, void *data)
{
	struct packet *packet;
	struct cb_info *cbinfo;
	const char *id;
	int ret;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->lb.type != _LB_TYPE_SCRIPT && handler->lb.type != _LB_TYPE_BUFFER) {
		ErrPrint("Handler is not valid type\n");
		return LB_STATUS_ERROR_INVALID;
	}

	id = fb_id(handler->lb.data.fb);
	if (!id || strncasecmp(id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP)))
		return LB_STATUS_ERROR_INVALID;

	packet = packet_create("lb_acquire_pixmap", "ss", handler->pkgname, handler->id);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	cbinfo = create_cb_info(cb, data);
	if (!cbinfo) {
		packet_destroy(packet);
		return LB_STATUS_ERROR_FAULT;
	}

	ret = master_rpc_async_request(handler, packet, 0, pixmap_acquired_cb, cbinfo);
	if (ret < 0)
		destroy_cb_info(cbinfo);

	return ret;
}

EAPI int livebox_release_lb_pixmap(struct livebox *handler, int pixmap)
{
	struct packet *packet;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->lb.type != _LB_TYPE_SCRIPT && handler->lb.type != _LB_TYPE_BUFFER) {
		ErrPrint("Handler is not valid type\n");
		return LB_STATUS_ERROR_INVALID;
	}

	packet = packet_create_noack("lb_release_pixmap", "ssi", handler->pkgname, handler->id, pixmap);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return master_rpc_request_only(handler, packet);
}

EAPI int livebox_acquire_pd_pixmap(struct livebox *handler, ret_cb_t cb, void *data)
{
	struct packet *packet;
	struct cb_info *cbinfo;
	const char *id;
	int ret;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->pd.type != _PD_TYPE_SCRIPT && handler->pd.type != _PD_TYPE_BUFFER) {
		ErrPrint("Handler is not valid type\n");
		return LB_STATUS_ERROR_INVALID;
	}

	id = fb_id(handler->pd.data.fb);
	if (!id || strncasecmp(id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP)))
		return LB_STATUS_ERROR_INVALID;

	packet = packet_create("pd_acquire_pixmap", "ss", handler->pkgname, handler->id);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	cbinfo = create_cb_info(cb, data);
	if (!cbinfo) {
		packet_destroy(packet);
		return LB_STATUS_ERROR_FAULT;
	}

	ret = master_rpc_async_request(handler, packet, 0, pixmap_acquired_cb, cbinfo);
	if (ret < 0)
		destroy_cb_info(cbinfo);

	return ret;
}

EAPI int livebox_pd_pixmap(const struct livebox *handler)
{
	const char *id;
	int pixmap = 0;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return 0;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Invalid handler\n");
		return 0;
	}

	if (handler->pd.type != _PD_TYPE_SCRIPT && handler->pd.type != _PD_TYPE_BUFFER) {
		ErrPrint("Invalid handler\n");
		return 0;
	}

	id = fb_id(handler->pd.data.fb);
	if (id && sscanf(id, SCHEMA_PIXMAP "%d", &pixmap) != 1) {
		ErrPrint("PIXMAP Id is not valid\n");
		return 0;
	}

	return pixmap;
}

EAPI int livebox_lb_pixmap(const struct livebox *handler)
{
	const char *id;
	int pixmap = 0;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return 0;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Invalid handler\n");
		return 0;
	}

	if (handler->lb.type != _LB_TYPE_SCRIPT && handler->lb.type != _LB_TYPE_BUFFER) {
		ErrPrint("Invalid handler\n");
		return 0;
	}

	id = fb_id(handler->lb.data.fb);
	if (id && sscanf(id, SCHEMA_PIXMAP "%d", &pixmap) != 1) {
		ErrPrint("PIXMAP Id is not valid\n");
		return 0;
	}

	return pixmap;
}

EAPI int livebox_release_pd_pixmap(struct livebox *handler, int pixmap)
{
	struct packet *packet;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->pd.type != _PD_TYPE_SCRIPT && handler->pd.type != _PD_TYPE_BUFFER) {
		ErrPrint("Handler is not valid type\n");
		return LB_STATUS_ERROR_INVALID;
	}

	packet = packet_create_noack("pd_release_pixmap", "ssi", handler->pkgname, handler->id, pixmap);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	return master_rpc_request_only(handler, packet);
}

EAPI void *livebox_acquire_fb(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return NULL;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Invalid handle\n");
		return NULL;
	}

	if (handler->lb.type != _LB_TYPE_SCRIPT && handler->lb.type != _LB_TYPE_BUFFER) {
		ErrPrint("Handler is not valid type\n");
		return NULL;
	}

	return fb_acquire_buffer(handler->lb.data.fb);
}

EAPI int livebox_release_fb(void *buffer)
{
	return fb_release_buffer(buffer);
}

EAPI int livebox_fb_refcnt(void *buffer)
{
	return fb_refcnt(buffer);
}

EAPI void *livebox_acquire_pdfb(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return NULL;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Invalid handler\n");
		return NULL;
	}

	if (handler->pd.type != _PD_TYPE_SCRIPT && handler->pd.type != _PD_TYPE_BUFFER) {
		ErrPrint("Handler is not valid type\n");
		return NULL;
	}

	return fb_acquire_buffer(handler->pd.data.fb);
}

EAPI int livebox_release_pdfb(void *buffer)
{
	return fb_release_buffer(buffer);
}

EAPI int livebox_pdfb_refcnt(void *buffer)
{
	return fb_refcnt(buffer);
}

EAPI int livebox_pdfb_bufsz(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return fb_size(handler->pd.data.fb);
}

EAPI int livebox_lbfb_bufsz(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return fb_size(handler->lb.data.fb);
}

EAPI int livebox_is_user(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return handler->is_user;
}

EAPI int livebox_set_pinup(struct livebox *handler, int flag, ret_cb_t cb, void *data)
{
	struct packet *packet;
	int ret;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->pinup_cb) {
		ErrPrint("Previous pinup request is not finished\n");
		return LB_STATUS_ERROR_BUSY;
	}

	if (handler->is_pinned_up == flag) {
		DbgPrint("No changes\n");
		return LB_STATUS_ERROR_ALREADY;
	}

	packet = packet_create("pinup_changed", "ssi", handler->pkgname, handler->id, flag);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	if (!cb)
		cb = default_pinup_cb;

	handler->pinup_cb = cb;
	handler->pinup_cbdata = data;

	ret = master_rpc_async_request(handler, packet, 0, pinup_done_cb, NULL);
	if (ret < 0) {
		handler->pinup_cb = NULL;
		handler->pinup_cbdata = NULL;
	}

	return ret;
}

EAPI int livebox_is_pinned_up(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id)
		return LB_STATUS_ERROR_INVALID;

	return handler->is_pinned_up;
}

EAPI int livebox_has_pinup(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id)
		return LB_STATUS_ERROR_INVALID;

	return handler->lb.pinup_supported;
}

EAPI int livebox_set_data(struct livebox *handler, void *data)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE)
		return LB_STATUS_ERROR_INVALID;

	handler->data = data;
	return LB_STATUS_SUCCESS;
}

EAPI void *livebox_get_data(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return NULL;
	}

	if (handler->state != CREATE)
		return NULL;

	return handler->data;
}

EAPI int livebox_is_exists(const char *pkgname)
{
	char *lb;

	lb = lb_pkgname(pkgname);
	if (lb) {
		free(lb);
		return 1;
	}

	return 0;
}

EAPI const char *livebox_content(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return NULL;
	}

	if (handler->state != CREATE)
		return NULL;

	return handler->content;
}

EAPI const char *livebox_category_title(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return NULL;
	}

	if (handler->state != CREATE)
		return NULL;

	return handler->title;
}

EAPI int livebox_emit_text_signal(struct livebox *handler, const char *emission, const char *source, double sx, double sy, double ex, double ey, ret_cb_t cb, void *data)
{
	struct packet *packet;
	struct cb_info *cbinfo;
	int ret;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if ((handler->lb.type != _LB_TYPE_TEXT && handler->pd.type != _PD_TYPE_TEXT) || handler->state != CREATE || !handler->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!emission)
		emission = "";

	if (!source)
		source = "";

	packet = packet_create("text_signal", "ssssdddd",
				handler->pkgname, handler->id, emission, source, sx, sy, ex, ey);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	cbinfo = create_cb_info(cb, data);
	if (!cbinfo) {
		packet_destroy(packet);
		return LB_STATUS_ERROR_FAULT;
	}

	ret = master_rpc_async_request(handler, packet, 0, text_signal_cb, cbinfo);
	if (ret < 0)
		destroy_cb_info(cbinfo);

	return LB_STATUS_ERROR_FAULT;
}

EAPI int livebox_subscribe_group(const char *cluster, const char *category)
{
	struct packet *packet;

	/*!
	 * \TODO
	 * Validate the group info using DB
	 * If the group info is not valid, do not send this request
	 */

	packet = packet_create_noack("subscribe", "ss", cluster ? cluster : "", category ? category : "");
	if (!packet) {
		ErrPrint("Failed to create a packet\n");
		return LB_STATUS_ERROR_FAULT;
	}

	return master_rpc_request_only(NULL, packet);
}

EAPI int livebox_unsubscribe_group(const char *cluster, const char *category)
{
	struct packet *packet;

	/*!
	 * \TODO
	 * Validate the group info using DB
	 * If the group info is not valid, do not send this request
	 * AND Check the subscribed or not too
	 */

	packet = packet_create_noack("unsubscribe", "ss", cluster ? cluster : "", category ? category : "");
	if (!packet) {
		ErrPrint("Failed to create a packet\n");
		return LB_STATUS_ERROR_FAULT;
	}

	return master_rpc_request_only(NULL, packet);
}

EAPI int livebox_refresh(struct livebox *handler)
{
	struct packet *packet;

	if (!handler) {
		ErrPrint("Hnalder is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id)
		return LB_STATUS_ERROR_INVALID;

	packet = packet_create_noack("update", "ss", handler->pkgname, handler->id);
	if (!packet) {
		ErrPrint("Failed to create a packet\n");
		return LB_STATUS_ERROR_FAULT;
	}

	return master_rpc_request_only(handler, packet);
}

EAPI int livebox_refresh_group(const char *cluster, const char *category)
{
	struct packet *packet;

	if (!cluster || !category) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	packet = packet_create_noack("refresh_group", "ss", cluster, category);
	if (!packet) {
		ErrPrint("Failed to create a packet\n");
		return LB_STATUS_ERROR_FAULT;
	}

	return master_rpc_request_only(NULL, packet);
}

EAPI int livebox_set_visibility(struct livebox *handler, enum livebox_visible_state state)
{
	struct packet *packet;
	int ret;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE || !handler->id)
		return LB_STATUS_ERROR_INVALID;

	if (!handler->is_user) {
		/* System cluster livebox cannot be changed its visible states */
		if (state == LB_HIDE_WITH_PAUSE) {
			ErrPrint("CA Livebox is not able to change the visibility\n");
			return LB_STATUS_ERROR_PERMISSION;
		}
	}

	DbgPrint("Change the visibility %d <> %d, %s\n", handler->visible, state, handler->id);

	if (handler->visible == state)
		return LB_STATUS_ERROR_ALREADY;

	packet = packet_create_noack("change,visibility", "ssi", handler->pkgname, handler->id, (int)state);
	if (!packet) {
		ErrPrint("Failed to create a packet\n");
		return LB_STATUS_ERROR_FAULT;
	}

	ret = master_rpc_request_only(handler, packet);
	if (ret == 0)
		handler->visible = state;

	return ret;
}

EAPI enum livebox_visible_state livebox_visibility(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_VISIBLE_ERROR;
	}

	if (handler->state != CREATE || !handler->id)
		return LB_VISIBLE_ERROR;

	return handler->visible;
}

int lb_set_group(struct livebox *handler, const char *cluster, const char *category)
{
	void *pc = NULL;
	void *ps = NULL;

	if (cluster) {
		pc = strdup(cluster);
		if (!pc) {
			CRITICAL_LOG("Heap: %s (cluster: %s)\n", strerror(errno), cluster);
			return LB_STATUS_ERROR_MEMORY;
		}
	}

	if (category) {
		ps = strdup(category);
		if (!ps) {
			CRITICAL_LOG("Heap: %s (category: %s)\n", strerror(errno), category);
			free(pc);
			return LB_STATUS_ERROR_MEMORY;
		}
	}

	if (handler->cluster)
		free(handler->cluster);

	if (handler->category)
		free(handler->category);

	handler->cluster = pc;
	handler->category = ps;

	return LB_STATUS_SUCCESS;
}

void lb_set_size(struct livebox *handler, int w, int h)
{
	handler->lb.width = w;
	handler->lb.height = h;
}

void lb_set_update_mode(struct livebox *handle, int active_mode)
{
	handle->is_active_update = active_mode;
}

void lb_set_pdsize(struct livebox *handler, int w, int h)
{
	handler->pd.width = w;
	handler->pd.height = h;
}

void lb_set_default_pdsize(struct livebox *handler, int w, int h)
{
	handler->pd.default_width = w;
	handler->pd.default_height = h;
}

void lb_invoke_fault_handler(enum livebox_fault_type event, const char *pkgname, const char *file, const char *func)
{
	struct dlist *l;
	struct dlist *n;
	struct fault_info *info;

	dlist_foreach_safe(s_info.fault_list, l, n, info) {
		if (info->handler(event, pkgname, file, func, info->user_data) == EXIT_FAILURE)
			s_info.fault_list = dlist_remove(s_info.fault_list, l);
	}
}

void lb_invoke_event_handler(struct livebox *handler, enum livebox_event_type event)
{
	struct dlist *l;
	struct dlist *n;
	struct event_info *info;

	dlist_foreach_safe(s_info.event_list, l, n, info) {
		if (info->handler(handler, event, info->user_data) == EXIT_FAILURE)
			s_info.event_list = dlist_remove(s_info.event_list, l);
	}
}

struct livebox *lb_find_livebox(const char *pkgname, const char *id)
{
	struct dlist *l;
	struct livebox *handler;

	dlist_foreach(s_info.livebox_list, l, handler) {
		if (!handler->id)
			continue;

		if (!strcmp(handler->pkgname, pkgname) && !strcmp(handler->id, id))
			return handler;
	}

	return NULL;
}

struct livebox *lb_find_livebox_by_timestamp(double timestamp)
{
	struct dlist *l;
	struct livebox *handler;

	dlist_foreach(s_info.livebox_list, l, handler) {
		if (handler->timestamp == timestamp)
			return handler;
	}

	return NULL;
}

static inline char *get_file_kept_in_safe(const char *id)
{
	const char *path;
	char *new_path;
	int len;
	int base_idx;

	path = util_uri_to_path(id);
	if (!path) {
		ErrPrint("Invalid URI(%s)\n", id);
		return NULL;
	}

	/*!
	 * \TODO: REMOVE ME
	 */
	if (s_info.prevent_overwrite) {
		new_path = strdup(path);
		if (!new_path)
			ErrPrint("Heap: %s\n", strerror(errno));

		return new_path;
	}


	len = strlen(path);
	base_idx = len - 1;

	while (base_idx > 0 && path[base_idx] != '/') base_idx--;
	base_idx += (path[base_idx] == '/');

	new_path = malloc(len + 10);
	if (!new_path) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return NULL;
	}

	strncpy(new_path, path, base_idx);
	snprintf(new_path + base_idx, len + 10 - base_idx, "reader/%s", path + base_idx);
	return new_path;
}

struct livebox *lb_new_livebox(const char *pkgname, const char *id, double timestamp)
{
	struct livebox *handler;

	handler = calloc(1, sizeof(*handler));
	if (!handler) {
		ErrPrint("Failed to create a new livebox\n");
		return NULL;
	}

	handler->pkgname = strdup(pkgname);
	if (!handler->pkgname) {
		ErrPrint("%s\n", strerror(errno));
		free(handler);
		return NULL;
	}

	handler->id = strdup(id);
	if (!handler->id) {
		ErrPrint("%s\n", strerror(errno));
		free(handler->pkgname);
		free(handler);
		return NULL;
	}

	handler->filename = get_file_kept_in_safe(id);
	if (!handler->filename) {
		handler->filename = strdup(util_uri_to_path(id));
		if (!handler->filename)
			ErrPrint("Error: %s\n", strerror(errno));
	}

	handler->timestamp = timestamp;
	handler->lb.type = _LB_TYPE_FILE;
	handler->pd.type = _PD_TYPE_SCRIPT;
	handler->state = CREATE;
	handler->visible = LB_SHOW;

	s_info.livebox_list = dlist_append(s_info.livebox_list, handler);
	lb_ref(handler);
	return handler;
}

int lb_delete_all(void)
{
	struct dlist *l;
	struct dlist *n;
	struct livebox *handler;

	dlist_foreach_safe(s_info.livebox_list, l, n, handler) {
		lb_invoke_event_handler(handler, LB_EVENT_DELETED);
		lb_unref(handler);
	}

	return LB_STATUS_SUCCESS;
}

int lb_set_content(struct livebox *handler, const char *content)
{
	if (handler->content) {
		free(handler->content);
		handler->content = NULL;
	}

	if (content) {
		handler->content = strdup(content);
		if (!handler->content) {
			CRITICAL_LOG("Heap: %s (content: %s)\n", strerror(errno), content);
			return LB_STATUS_ERROR_MEMORY;
		}
	}

	return LB_STATUS_SUCCESS;
}

int lb_set_title(struct livebox *handler, const char *title)
{
	if (handler->title) {
		free(handler->title);
		handler->title = NULL;
	}

	if (title) {
		handler->title = strdup(title);
		if (!handler->title) {
			CRITICAL_LOG("Heap: %s (title: %s)\n", strerror(errno), title);
			return LB_STATUS_ERROR_MEMORY;
		}
	}

	return LB_STATUS_SUCCESS;
}

void lb_set_size_list(struct livebox *handler, int size_list)
{
	handler->lb.size_list = size_list;
}

void lb_set_auto_launch(struct livebox *handler, const char *auto_launch)
{
	if (!strlen(auto_launch))
		return;

	handler->lb.auto_launch = strdup(auto_launch);
	if (!handler->lb.auto_launch)
		ErrPrint("Heap: %s\n", strerror(errno));
}

void lb_set_priority(struct livebox *handler, double priority)
{
	handler->lb.priority = priority;
}

void lb_set_id(struct livebox *handler, const char *id)
{
	if (handler->id)
		free(handler->id);

	handler->id = strdup(id);
	if (!handler->id)
		ErrPrint("Error: %s\n", strerror(errno));

	if (handler->filename)
		free(handler->filename);

	handler->filename = get_file_kept_in_safe(id);
	if (!handler->filename) {
		handler->filename = strdup(util_uri_to_path(id));
		if (!handler->filename)
			ErrPrint("Error: %s\n", strerror(errno));
	}
}

int lb_set_lb_fb(struct livebox *handler, const char *filename)
{
	struct fb_info *fb;

	if (!handler)
		return LB_STATUS_ERROR_INVALID;

	fb = handler->lb.data.fb;
	if (fb && !strcmp(fb_id(fb), filename)) /*!< BUFFER is not changed, */
		return LB_STATUS_SUCCESS;

	handler->lb.data.fb = NULL;

	if (!filename || filename[0] == '\0') {
		if (fb)
			fb_destroy(fb);
		return LB_STATUS_SUCCESS;
	}

	handler->lb.data.fb = fb_create(filename, handler->lb.width, handler->lb.height);
	if (!handler->lb.data.fb) {
		ErrPrint("Faield to create a FB\n");
		if (fb)
			fb_destroy(fb);
		return LB_STATUS_ERROR_FAULT;
	}

	if (fb)
		fb_destroy(fb);

	return LB_STATUS_SUCCESS;
}

int lb_set_pd_fb(struct livebox *handler, const char *filename)
{
	struct fb_info *fb;

	if (!handler)
		return LB_STATUS_ERROR_INVALID;

	fb = handler->pd.data.fb;
	if (fb && !strcmp(fb_id(fb), filename)) {
		/* BUFFER is not changed, just update the content */
		return LB_STATUS_ERROR_EXIST;
	}
	handler->pd.data.fb = NULL;

	if (!filename || filename[0] == '\0') {
		if (fb)
			fb_destroy(fb);
		return LB_STATUS_SUCCESS;
	}

	handler->pd.data.fb = fb_create(filename, handler->pd.width, handler->pd.height);
	if (!handler->pd.data.fb) {
		ErrPrint("Failed to create a FB\n");
		if (fb)
			fb_destroy(fb);
		return LB_STATUS_ERROR_FAULT;
	}

	if (fb)
		fb_destroy(fb);
	return LB_STATUS_SUCCESS;
}

struct fb_info *lb_get_lb_fb(struct livebox *handler)
{
	return handler->lb.data.fb;
}

struct fb_info *lb_get_pd_fb(struct livebox *handler)
{
	return handler->pd.data.fb;
}

void lb_set_user(struct livebox *handler, int user)
{
	handler->is_user = user;
}

void lb_set_pinup(struct livebox *handler, int pinup_supported)
{
	handler->lb.pinup_supported = pinup_supported;
}

void lb_set_text_lb(struct livebox *handler)
{
	handler->lb.type = _LB_TYPE_TEXT;
}

void lb_set_text_pd(struct livebox *handler)
{
	handler->pd.type = _PD_TYPE_TEXT;
}

int lb_text_lb(struct livebox *handler)
{
	return handler->lb.type == _LB_TYPE_TEXT;
}

int lb_text_pd(struct livebox *handler)
{
	return handler->pd.type == _PD_TYPE_TEXT;
}

void lb_set_period(struct livebox *handler, double period)
{
	handler->lb.period = period;
}

struct livebox *lb_ref(struct livebox *handler)
{
	if (!handler)
		return NULL;

	handler->refcnt++;
	return handler;
}

struct livebox *lb_unref(struct livebox *handler)
{
	if (!handler)
		return NULL;

	handler->refcnt--;
	if (handler->refcnt > 0)
		return handler;

	if (handler->created_cb) {
		DbgPrint("Clear created event callback\n");
		handler->created_cb(handler, LB_STATUS_ERROR_FAULT, handler->created_cbdata);
		handler->created_cb = NULL;
		handler->created_cbdata = NULL;
	}

	if (handler->deleted_cb) {
		DbgPrint("Clear deleted event callback\n");
		handler->deleted_cb(handler, LB_STATUS_ERROR_FAULT, handler->deleted_cbdata);
		handler->deleted_cb = NULL;
		handler->deleted_cbdata = NULL;
	}

	if (handler->pinup_cb) {
		DbgPrint("Clear pinup event callback\n");
		handler->pinup_cb(handler, LB_STATUS_ERROR_FAULT, handler->pinup_cbdata);
		handler->pinup_cb = NULL;
		handler->pinup_cbdata = NULL;
	}

	if (handler->group_changed_cb) {
		DbgPrint("Clear group changed event callback\n");
		handler->group_changed_cb(handler, LB_STATUS_ERROR_FAULT, handler->group_cbdata);
		handler->group_changed_cb = NULL;
		handler->group_cbdata = NULL;
	}

	if (handler->period_changed_cb) {
		DbgPrint("Clear period changed event callback\n");
		handler->period_changed_cb(handler, LB_STATUS_ERROR_FAULT, handler->period_cbdata);
		handler->period_changed_cb = NULL;
		handler->period_cbdata = NULL;
	}

	if (handler->size_changed_cb) {
		DbgPrint("Clear size changed event callback\n");
		handler->size_changed_cb(handler, LB_STATUS_ERROR_FAULT, handler->size_cbdata);
		handler->size_changed_cb = NULL;
		handler->size_cbdata = NULL;
	}

	if (handler->pd_created_cb) {
		DbgPrint("Clear pd created event callback\n");
		handler->pd_created_cb(handler, LB_STATUS_ERROR_FAULT, handler->pd_created_cbdata);
		handler->pd_created_cb = NULL;
		handler->pd_created_cbdata = NULL;
	}

	if (handler->pd_destroyed_cb) {
		DbgPrint("Clear pd destroyed event callback\n");
		handler->pd_destroyed_cb(handler, LB_STATUS_ERROR_FAULT, handler->pd_destroyed_cbdata);
		handler->pd_destroyed_cb = NULL;
		handler->pd_destroyed_cbdata = NULL;
	}

	if (handler->update_mode_cb) {
		DbgPrint("Clear update mode callback\n");
		handler->update_mode_cb(handler, LB_STATUS_ERROR_FAULT, handler->update_mode_cbdata);
		handler->update_mode_cb = NULL;
		handler->update_mode_cbdata = NULL;
	}

	if (handler->access_event_cb) {
		DbgPrint("Clear access status callback\n");
		handler->access_event_cb(handler, LB_STATUS_ERROR_FAULT, handler->access_event_cbdata);
		handler->access_event_cb = NULL;
		handler->access_event_cbdata = NULL;
	}

	if (handler->filename)
		util_unlink(handler->filename);

	dlist_remove_data(s_info.livebox_list, handler);

	handler->state = DESTROYED;
	free(handler->cluster);
	free(handler->category);
	free(handler->id);
	free(handler->pkgname);
	free(handler->filename);
	free(handler->lb.auto_launch);

	if (handler->lb.data.fb) {
		fb_destroy(handler->lb.data.fb);
		handler->lb.data.fb = NULL;
	}

	if (handler->pd.data.fb) {
		fb_destroy(handler->pd.data.fb);
		handler->pd.data.fb = NULL;
	}

	free(handler);
	return NULL;
}

int lb_send_delete(struct livebox *handler, ret_cb_t cb, void *data)
{
	struct packet *packet;
	struct cb_info *cbinfo;
	int ret;

	if (!cb && !!data) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->deleted_cb) {
		ErrPrint("Already in-progress\n");
		return LB_STATUS_ERROR_BUSY;
	}

	packet = packet_create("delete", "ss", handler->pkgname, handler->id);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		if (cb)
			cb(handler, LB_STATUS_ERROR_FAULT, data);

		return LB_STATUS_ERROR_FAULT;
	}

	if (!cb)
		cb = default_delete_cb;

	cbinfo = create_cb_info(cb, data);
	if (!cbinfo) {
		packet_destroy(packet);
		return LB_STATUS_ERROR_FAULT;
	}

	ret = master_rpc_async_request(handler, packet, 0, del_ret_cb, cbinfo);
	if (ret < 0)
		destroy_cb_info(cbinfo);

	return ret;
}

EAPI int livebox_client_paused(void)
{
	struct packet *packet;

	packet = packet_create_noack("client_paused", "d", util_timestamp());
	if (!packet) {
		ErrPrint("Failed to create a pause packet\n");
		return LB_STATUS_ERROR_FAULT;
	}

	return master_rpc_request_only(NULL, packet);
}

EAPI int livebox_client_resumed(void)
{
	struct packet *packet;

	packet = packet_create_noack("client_resumed", "d", util_timestamp());
	if (!packet) {
		ErrPrint("Failed to create a resume packet\n");
		return LB_STATUS_ERROR_FAULT;
	}

	return master_rpc_request_only(NULL, packet);
}

/* End of a file */
