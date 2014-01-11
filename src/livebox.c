/*
 * Copyright 2013  Samsung Electronics Co., Ltd
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

#include <stdio.h>
#include <errno.h>
#include <stdlib.h> /* malloc */
#include <string.h> /* strdup */
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <gio/gio.h>
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
#include "conf.h"

#define EAPI __attribute__((visibility("default")))
#define MINIMUM_EVENT	s_info.event_filter

#if defined(FLOG)
FILE *__file_log_fp;
#endif

enum event_state {
	INFO_STATE_CALLBACK_IN_IDLE = 0x00,
	INFO_STATE_CALLBACK_IN_PROCESSING = 0x01,
};

static struct info {
	struct dlist *livebox_list;
	struct dlist *livebox_common_list;

	struct dlist *event_list;
	struct dlist *fault_list;

	int init_count;
	int prevent_overwrite;
	double event_filter;
	enum event_state event_state;
	enum event_state fault_state;
	guint job_timer;
	struct dlist *job_list;
} s_info = {
	.livebox_list = NULL,
	.event_list = NULL,
	.fault_list = NULL,
	.init_count = 0,
	.prevent_overwrite = 0,
	.event_filter = 0.01f,
	.event_state = INFO_STATE_CALLBACK_IN_IDLE,
	.fault_state = INFO_STATE_CALLBACK_IN_IDLE,
	.job_timer = 0,
	.job_list = NULL,
};

struct cb_info {
	ret_cb_t cb;
	void *data;
};

struct event_info {
	int is_deleted;
	int (*handler)(struct livebox *handler, enum livebox_event_type event, void *data);
	void *user_data;
};

struct fault_info {
	int is_deleted;
	int (*handler)(enum livebox_fault_type event, const char *pkgname, const char *filename, const char *func, void *data);
	void *user_data;
};

static void lb_pixmap_acquired_cb(struct livebox *handler, const struct packet *result, void *data);
static void pd_pixmap_acquired_cb(struct livebox *handler, const struct packet *result, void *data);

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

static inline void default_key_event_cb(struct livebox *handler, int ret, void *data)
{
	DbgPrint("Default key event handler: %d\n", ret);
}

static inline __attribute__((always_inline)) struct cb_info *create_cb_info(ret_cb_t cb, void *data)
{
	struct cb_info *info;

	info = malloc(sizeof(*info));
	if (!info) {
		ErrPrint("Heap: %s\n", strerror(errno));
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

static int do_fb_lock(int fd)
{
        struct flock flock;
	int ret;

	flock.l_type = F_RDLCK;
	flock.l_whence = SEEK_SET;
	flock.l_start = 0;
	flock.l_len = 0;
	flock.l_pid = getpid();

	do {
		ret = fcntl(fd, F_SETLKW, &flock);
		if (ret < 0) {
			ret = errno;
			ErrPrint("fcntl: %s\n", strerror(errno));
		}
	} while (ret == EINTR);

	return ret;
}

static int do_fb_unlock(int fd)
{
	struct flock flock;
	int ret;

	flock.l_type = F_UNLCK;
	flock.l_whence = SEEK_SET;
	flock.l_start = 0;
	flock.l_len = 0;
	flock.l_pid = getpid();

	do {
		ret = fcntl(fd, F_SETLKW, &flock);
		if (ret < 0) {
			ret = errno;
			ErrPrint("fcntl: %s\n", strerror(errno));
		}
	} while (ret == EINTR);

	return ret;
}

int lb_destroy_lock_file(struct livebox_common *common, int is_pd)
{
	if (is_pd) {
		if (!common->pd.lock) {
			return LB_STATUS_ERROR_INVALID;
		}

		if (close(common->pd.lock_fd) < 0) {
			ErrPrint("close: %s\n", strerror(errno));
		}
		common->pd.lock_fd = -1;

		if (unlink(common->pd.lock) < 0) {
			ErrPrint("unlink: %s\n", strerror(errno));
		}

		free(common->pd.lock);
		common->pd.lock = NULL;
	} else {
		if (!common->lb.lock) {
			return LB_STATUS_ERROR_INVALID;
		}

		if (close(common->lb.lock_fd) < 0) {
			ErrPrint("close: %s\n", strerror(errno));
		}
		common->lb.lock_fd = -1;

		if (unlink(common->lb.lock) < 0) {
			ErrPrint("unlink: %s\n", strerror(errno));
		}

		free(common->lb.lock);
		common->lb.lock = NULL;
	}

	return LB_STATUS_SUCCESS;
}

int lb_create_lock_file(struct livebox_common *common, int is_pd)
{
	int len;
	char *file;

	len = strlen(common->id);
	file = malloc(len + 20);
	if (!file) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return LB_STATUS_ERROR_MEMORY;
	}

	snprintf(file, len + 20, "%s.%s.lck", util_uri_to_path(common->id), is_pd ? "pd" : "lb");

	if (is_pd) {
		common->pd.lock_fd = open(file, O_RDONLY);
		if (common->pd.lock_fd < 0) {
			ErrPrint("open: %s\n", strerror(errno));
			free(file);
			return LB_STATUS_ERROR_IO;
		}

		common->pd.lock = file;
	} else {
		common->lb.lock_fd = open(file, O_RDONLY);
		if (common->lb.lock_fd < 0) {
			ErrPrint("open: %s\n", strerror(errno));
			free(file);
			return LB_STATUS_ERROR_IO;
		}

		common->lb.lock = file;
	}

	return LB_STATUS_SUCCESS;
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
		ErrPrint("Resize request is failed: %d\n", ret);
		goto errout;
	}

	return;

errout:
	handler->cbs.update_mode.cb(handler, ret, handler->cbs.update_mode.data);
	handler->cbs.update_mode.cb = NULL;
	handler->cbs.update_mode.data = NULL;
	return;
}

static void resize_cb(struct livebox *handler, const struct packet *result, void *data)
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
		ErrPrint("Resize request is failed: %d\n", ret);
		goto errout;
	}

	return;

errout:
	handler->cbs.size_changed.cb(handler, ret, handler->cbs.size_changed.data);
	handler->cbs.size_changed.cb = NULL;
	handler->cbs.size_changed.data = NULL;
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

	if (cb) {
		cb(handler, ret, cbdata);
	}
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

	if (ret < 0) {
		goto errout;
	}

	return;

errout:
	handler->cbs.group_changed.cb(handler, ret, handler->cbs.group_changed.data);
	handler->cbs.group_changed.cb = NULL;
	handler->cbs.group_changed.data = NULL;
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

	if (ret < 0) {
		goto errout;
	}

	return;

errout:
	handler->cbs.period_changed.cb(handler, ret, handler->cbs.period_changed.data);
	handler->cbs.period_changed.cb = NULL;
	handler->cbs.period_changed.data = NULL;
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
		handler->cbs.deleted.cb = cb;
		handler->cbs.deleted.data = cbdata;
	} else if (cb) {
		cb(handler, ret, cbdata);
	}

	/*!
	 * \note
	 * Do not call the deleted callback from here.
	 * master will send the "deleted" event.
	 * Then invoke this callback.
	 *
	 * if (handler->cbs.deleted.cb)
	 * 	handler->cbs.deleted.cb(handler, ret, handler->cbs.deleted.data);
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
		handler->cbs.created.cb = cb;
		handler->cbs.created.data = cbdata;

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

	lb_unref(handler, 1);
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
		ErrPrint("Failed to create a PD[%d]\n", ret);
		goto errout;
	}

	return;

errout:
	handler->cbs.pd_created.cb(handler, ret, handler->cbs.pd_created.data);
	handler->cbs.pd_created.cb = NULL;
	handler->cbs.pd_created.data = NULL;
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

	if (cb) {
		cb(handler, ret, cbdata);
	}
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
		ErrPrint("Result is NIL (may connection lost)\n");
		ret = LB_STATUS_ERROR_FAULT;
	} else if (packet_get(result, "i", &ret) != 1) {
		ErrPrint("Invalid parameter\n");
		ret = LB_STATUS_ERROR_INVALID;
	}

	if (ret == 0) {
		handler->cbs.pd_destroyed.cb = cb;
		handler->cbs.pd_destroyed.data = cbdata;
	} else if (cb) {
		handler->common->is_pd_created = 0;
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

	if (cb) {
		cb(handler, ret, cbdata);
	}
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

	if (!result) {
		ret = LB_STATUS_ERROR_FAULT;
	} else if (packet_get(result, "i", &ret) != 1) {
		ret = LB_STATUS_ERROR_INVALID;
	}

	if (cb) {
		cb(handler, ret, cbdata);
	}
}

static int lb_acquire_lb_pixmap(struct livebox *handler, ret_cb_t cb, void *data)
{
	struct packet *packet;
	struct cb_info *cbinfo;
	const char *id;
	int ret;

	id = fb_id(handler->common->lb.fb);
	if (!id || strncasecmp(id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP))) {
		return LB_STATUS_ERROR_INVALID;
	}

	packet = packet_create("lb_acquire_pixmap", "ss", handler->common->pkgname, handler->common->id);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	cbinfo = create_cb_info(cb, data);
	if (!cbinfo) {
		packet_destroy(packet);
		return LB_STATUS_ERROR_FAULT;
	}

	ret = master_rpc_async_request(handler, packet, 0, lb_pixmap_acquired_cb, cbinfo);
	if (ret < 0) {
		destroy_cb_info(cbinfo);
	}

	return ret;
}

static void lb_pixmap_acquired_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int pixmap;
	int ret = LB_STATUS_ERROR_INVALID;
	ret_cb_t cb;
	void *cbdata;
	struct cb_info *info = data;

	cb = info->cb;
	cbdata = info->data;
	destroy_cb_info(info);

	if (!result) {
		pixmap = 0; /* PIXMAP 0 means error */
	} else if (packet_get(result, "ii", &pixmap, &ret) != 2) {
		pixmap = 0;
	}

	if (ret == LB_STATUS_ERROR_BUSY) {
		ret = lb_acquire_lb_pixmap(handler, cb, cbdata);
		DbgPrint("Busy, Try again: %d\n", ret);
		/* Try again */
	} else {
		if (cb) {
			cb(handler, pixmap, cbdata);
		}
	}
}

static int lb_acquire_pd_pixmap(struct livebox *handler, ret_cb_t cb, void *data)
{
	struct packet *packet;
	struct cb_info *cbinfo;
	const char *id;
	int ret;

	id = fb_id(handler->common->pd.fb);
	if (!id || strncasecmp(id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP))) {
		return LB_STATUS_ERROR_INVALID;
	}

	packet = packet_create("pd_acquire_pixmap", "ss", handler->common->pkgname, handler->common->id);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	cbinfo = create_cb_info(cb, data);
	if (!cbinfo) {
		packet_destroy(packet);
		return LB_STATUS_ERROR_FAULT;
	}

	ret = master_rpc_async_request(handler, packet, 0, pd_pixmap_acquired_cb, cbinfo);
	if (ret < 0) {
		/*!
		 * \note
		 * Packet will be destroyed by master_rpc_async_request
		 */
		destroy_cb_info(cbinfo);
	}

	return ret;
}

static void pd_pixmap_acquired_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int pixmap;
	int ret;
	ret_cb_t cb;
	void *cbdata;
	struct cb_info *info = data;

	cb = info->cb;
	cbdata = info->data;
	destroy_cb_info(info);

	if (!result) {
		pixmap = 0; /* PIXMAP 0 means error */
		ret = LB_STATUS_ERROR_FAULT;
	} else if (packet_get(result, "ii", &pixmap, &ret) != 2) {
		pixmap = 0;
		ret = LB_STATUS_ERROR_INVALID;
	}

	if (ret == LB_STATUS_ERROR_BUSY) {
		ret = lb_acquire_pd_pixmap(handler, cb, cbdata);
		DbgPrint("Busy, Try again: %d\n", ret);
		/* Try again */
	} else {
		if (cb) {
			DbgPrint("ret: %d, pixmap: %d\n", ret, pixmap);
			cb(handler, pixmap, cbdata);
		}
	}
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

	if (ret < 0) {
		goto errout;
	}

	return;

errout:
	handler->cbs.pinup.cb(handler, ret, handler->cbs.pinup.data);
	handler->cbs.pinup.cb = NULL;
	handler->cbs.pinup.data = NULL;
}

static void key_ret_cb(struct livebox *handler, const struct packet *result, void *data)
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

	return;
errout:
	handler->cbs.key_event.cb(handler, ret, handler->cbs.key_event.data);
	handler->cbs.key_event.cb = NULL;
	handler->cbs.key_event.data = NULL;
	return;
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

	return;

errout:
	handler->cbs.access_event.cb(handler, ret, handler->cbs.access_event.data);
	handler->cbs.access_event.cb = NULL;
	handler->cbs.access_event.data = NULL;
	return;
}

static int send_access_event(struct livebox *handler, const char *event, int x, int y)
{
	struct packet *packet;
	double timestamp;

	timestamp = util_timestamp();

	packet = packet_create(event, "ssdii", handler->common->pkgname, handler->common->id, timestamp, x, y);
	if (!packet) {
		ErrPrint("Failed to build packet\n");
		return LB_STATUS_ERROR_FAULT;
	}

	return master_rpc_async_request(handler, packet, 0, access_ret_cb, NULL);
}

static int send_key_event(struct livebox *handler, const char *event, unsigned int keycode)
{
	struct packet *packet;
	double timestamp;

	timestamp = util_timestamp();
	packet = packet_create(event, "ssdi", handler->common->pkgname, handler->common->id, timestamp, keycode);
	if (!packet) {
		ErrPrint("Failed to build packet\n");
		return LB_STATUS_ERROR_FAULT;
	}

	return master_rpc_async_request(handler, packet, 0, key_ret_cb, NULL);
}

static int send_mouse_event(struct livebox *handler, const char *event, int x, int y)
{
	struct packet *packet;
	double timestamp;

	timestamp = util_timestamp();
	packet = packet_create_noack(event, "ssdii", handler->common->pkgname, handler->common->id, timestamp, x, y);
	if (!packet) {
		ErrPrint("Failed to build param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	return master_rpc_request_only(handler, packet);
}

static void initialize_livebox(void *disp, int use_thread)
{
#if defined(FLOG)
	char filename[BUFSIZ];
	snprintf(filename, sizeof(filename), "/tmp/%d.box.log", getpid());
	__file_log_fp = fopen(filename, "w+t");
	if (!__file_log_fp) {
		__file_log_fp = fdopen(1, "w+t");
	}
#endif
	livebox_service_init();
	fb_init(disp);

	client_init(use_thread);

	s_info.init_count++;
}

EAPI int livebox_init_with_options(void *disp, int prevent_overwrite, double event_filter, int use_thread)
{
	if (s_info.init_count > 0) {
		s_info.init_count++;
		return LB_STATUS_SUCCESS;
	}

	/*!
	 * \note
	 * Some application doesn't want to use the environment value.
	 * So set them using arguments.
	 */
	s_info.prevent_overwrite = prevent_overwrite;
	MINIMUM_EVENT = event_filter;

	initialize_livebox(disp, use_thread);
	return LB_STATUS_SUCCESS;
}

EAPI int livebox_init(void *disp)
{
	const char *env;

	if (s_info.init_count > 0) {
		s_info.init_count++;
		return LB_STATUS_SUCCESS;
	}

	env = getenv("PROVIDER_DISABLE_PREVENT_OVERWRITE");
	if (env && !strcasecmp(env, "true")) {
		s_info.prevent_overwrite = 1;
	}

	env = getenv("PROVIDER_EVENT_FILTER");
	if (env) {
		sscanf(env, "%lf", &MINIMUM_EVENT);
	}

	initialize_livebox(disp, 0);
	return LB_STATUS_SUCCESS;
}

EAPI int livebox_fini(void)
{
	if (s_info.init_count <= 0) {
		ErrPrint("Doesn't initialized\n");
		return LB_STATUS_ERROR_INVALID;
	}

	s_info.init_count--;
	if (s_info.init_count > 0) {
		ErrPrint("init count : %d\n", s_info.init_count);
		return LB_STATUS_SUCCESS;
	}

	client_fini();
	fb_fini();
	livebox_service_fini();
	return LB_STATUS_SUCCESS;
}

static inline char *lb_pkgname(const char *pkgname)
{
	char *lb;

	lb = livebox_service_pkgname(pkgname);
	if (!lb) {
		if (util_validate_livebox_package(pkgname) == 0) {
			return strdup(pkgname);
		}
	}

	return lb;
}

static struct livebox_common *find_sharable_common_handle(const char *pkgname, const char *content, int w, int h, const char *cluster, const char *category)
{
	struct dlist *l;
	struct livebox_common *common;

	if (!conf_shared_content()) {
		/*!
		 * Shared content option is turnned off.
		 */
		return NULL;
	}

	dlist_foreach(s_info.livebox_common_list, l, common) {
		if (common->state != CREATE) {
			continue;
		}

		if (strcmp(common->pkgname, pkgname)) {
			continue;
		}

		if (strcmp(common->cluster, cluster)) {
			DbgPrint("Cluster mismatched\n");
			continue;
		}

		if (strcmp(common->category, category)) {
			DbgPrint("Category mismatched\n");
			continue;
		}

		if (common->content && content) {
			if (strcmp(common->content, content)) {
				DbgPrint("%s Content ([%s] <> [%s])\n", common->pkgname, common->content, content);
				continue;	
			}
		} else {
			int c1_len;
			int c2_len;

			/*!
			 * \note
			 * We assumes "" (ZERO length string) to NULL
			 */
			c1_len = common->content ? strlen(common->content) : 0;
			c2_len = content ? strlen(content) : 0;
			if (c1_len != c2_len) {
				DbgPrint("%s Content %p <> %p\n", common->pkgname, common->content, content);
				continue;
			}
		}

		if (common->request.size_changed) {
			DbgPrint("Changing size\n");
			/*!
			 * \note
			 * Do not re-use resizing instance.
			 * We will not use predicted size.
			 */
			continue;
		}

		if (common->request.created) {
			DbgPrint("Creating now but re-use it (%s)\n", common->pkgname);
		}

		if (common->lb.width != w || common->lb.height != h) {
			DbgPrint("Size mismatched\n");
			continue;
		}

		DbgPrint("common handle is found: %p\n", common);
		return common;
	}

	return NULL;
}

/*!
 * Just wrapping the livebox_add_with_size function.
 */
EAPI struct livebox *livebox_add(const char *pkgname, const char *content, const char *cluster, const char *category, double period, ret_cb_t cb, void *data)
{
	return livebox_add_with_size(pkgname, content, cluster, category, period, LB_SIZE_TYPE_UNKNOWN, cb, data);
}

static gboolean job_execute_cb(void *data)
{
	struct job_item *item;
	struct dlist *l;

	l = dlist_nth(s_info.job_list, 0);
	if (!l) {
		s_info.job_timer = 0;
		return FALSE;
	}

	item = dlist_data(l);
	s_info.job_list = dlist_remove(s_info.job_list, l);

	if (item) {
		item->cb(item->handle, item->ret, item->data);
		lb_unref(item->handle, 1);
		free(item);
	}

	return TRUE;
}

static int job_add(struct livebox *handle, ret_cb_t job_cb, int ret, void *data)
{
	struct job_item *item;

	if (!job_cb) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	item = malloc(sizeof(*item));
	if (!item) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return LB_STATUS_ERROR_MEMORY;
	}

	item->handle = lb_ref(handle);
	item->cb = job_cb;
	item->data = data;
	item->ret = ret;

	s_info.job_list = dlist_append(s_info.job_list, item);

	if (!s_info.job_timer) {
		s_info.job_timer = g_timeout_add(1, job_execute_cb, NULL);
		if (!s_info.job_timer) {
			ErrPrint("Failed to create a job timer\n");
		}
	}

	return LB_STATUS_SUCCESS;
}

static int create_real_instance(struct livebox *handler, ret_cb_t cb, void *data)
{
	struct cb_info *cbinfo;
	struct packet *packet;
	struct livebox_common *common;
	int ret;

	common = handler->common;

	packet = packet_create("new", "dssssdii",
				common->timestamp, common->pkgname, common->content,
				common->cluster, common->category,
				common->lb.period, common->lb.width, common->lb.height);
	if (!packet) {
		ErrPrint("Failed to create a new packet\n");
		return LB_STATUS_ERROR_FAULT;
	}

	cbinfo = create_cb_info(cb, data);
	if (!cbinfo) {
		ErrPrint("Failed to create a cbinfo\n");
		packet_destroy(packet);
		return LB_STATUS_ERROR_MEMORY;
	}

	/*!
	 * \note
	 * master_rpc_async_request will destroy the packet (decrease the refcnt)
	 * So be aware the packet object after return from master_rpc_async_request.
	 */
	ret = master_rpc_async_request(handler, packet, 0, new_ret_cb, cbinfo);
	if (ret < 0) {
		ErrPrint("Failed to send a new packet\n");
		destroy_cb_info(cbinfo);
		return LB_STATUS_ERROR_FAULT;
	}
	handler->common->request.created = 1;
	return LB_STATUS_SUCCESS;
}

static void create_cb(struct livebox *handle, int ret, void *data)
{
	struct cb_info *cbinfo = data;

	if (cbinfo->cb) {
		cbinfo->cb(handle, ret, cbinfo->data);
	}

	destroy_cb_info(cbinfo);

	/*!
	 * \note
	 * Forcely generate "updated" event
	 */
	lb_invoke_event_handler(handle, LB_EVENT_LB_UPDATED);
}

static int create_fake_instance(struct livebox *handler, ret_cb_t cb, void *data)
{
	struct cb_info *cbinfo;

	cbinfo = create_cb_info(cb, data);
	if (!cbinfo) {
		ErrPrint("Failed to create a cbinfo\n");
		return LB_STATUS_ERROR_MEMORY;
	}

	if (job_add(handler, create_cb, LB_STATUS_SUCCESS, cbinfo) != LB_STATUS_SUCCESS) {
		destroy_cb_info(cbinfo);
	}

	return LB_STATUS_SUCCESS;
}

struct livebox_common *lb_create_common_handle(struct livebox *handle, const char *pkgname, const char *cluster, const char *category)
{
	struct livebox_common *common;

	common = calloc(1, sizeof(*common));
	if (!common) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return NULL;
	}

	common->pkgname = strdup(pkgname);
	if (!common->pkgname) {
		free(common);
		return NULL;
	}

	common->cluster = strdup(cluster);
	if (!common->cluster) {
		ErrPrint("Error: %s\n", strerror(errno));
		free(common->pkgname);
		free(common);
		return NULL;
	}

	common->category = strdup(category);
	if (!common->category) {
		ErrPrint("Error: %s\n", strerror(errno));
		free(common->cluster);
		free(common->pkgname);
		free(common);
		return NULL;
	}

	/* Data provider will set this */
	common->lb.type = _LB_TYPE_FILE;
	common->pd.type = _PD_TYPE_SCRIPT;

	/* Used for handling the mouse event on a box */
	common->lb.mouse_event = livebox_service_mouse_event(common->pkgname);

	/* Cluster infomration is not determined yet */
	common->nr_of_sizes = 0x01;

	common->timestamp = util_timestamp();
	common->is_user = 1;
	common->delete_type = LB_DELETE_PERMANENTLY;
	common->pd.lock = NULL;
	common->pd.lock_fd = -1;
	common->lb.lock = NULL;
	common->lb.lock_fd = -1;

	common->state = CREATE;
	common->visible = LB_SHOW;

	s_info.livebox_common_list = dlist_append(s_info.livebox_common_list, common);
	return common;
}

int lb_destroy_common_handle(struct livebox_common *common)
{
	dlist_remove_data(s_info.livebox_common_list, common);

	common->state = DESTROYED;
	free(common->cluster);
	free(common->category);
	free(common->id);
	free(common->pkgname);
	free(common->filename);
	free(common->lb.auto_launch);
	free(common->alt.icon);
	free(common->alt.name);

	if (common->lb.fb) {
		fb_destroy(common->lb.fb);
		common->lb.fb = NULL;
	}

	if (common->pd.fb) {
		fb_destroy(common->pd.fb);
		common->pd.fb = NULL;
	}

	return 0;
}

int lb_common_ref(struct livebox_common *common, struct livebox *handle)
{
	common->livebox_list = dlist_append(common->livebox_list, handle);
	common->refcnt++;

	return common->refcnt;
}

int lb_common_unref(struct livebox_common *common, struct livebox *handle)
{
	int refcnt;
	dlist_remove_data(common->livebox_list, handle);
	refcnt = --common->refcnt;

	return refcnt;
}

static void refresh_for_paused_updating_cb(struct livebox *handle, int ret, void *data)
{
	if (handle->paused_updating == 0) {
		DbgPrint("Paused updates are cleared\n");
		return;
	}

	DbgPrint("Pending updates are found\n");
	lb_invoke_event_handler(handle, LB_EVENT_LB_UPDATED);
}

static int lb_set_visibility(struct livebox *handler, enum livebox_visible_state state)
{
	struct packet *packet;
	int need_to_add_job = 0;
	int ret;

	if (handler->common->visible != LB_SHOW && state == LB_SHOW) {
		if (handler->paused_updating) {
			need_to_add_job = 1;
		}
	} else if (handler->common->visible == LB_SHOW && state != LB_SHOW) {
		struct dlist *l;
		struct livebox *item;

		dlist_foreach(handler->common->livebox_list, l, item) {
			if (item->visible == LB_SHOW) {
				DbgPrint("%s visibility is not changed\n", handler->common->pkgname);
				return LB_STATUS_SUCCESS;
			}
		}
	} else if (handler->common->visible == LB_SHOW && state == LB_SHOW && handler->paused_updating) {
		if (job_add(handler, refresh_for_paused_updating_cb, LB_STATUS_SUCCESS, NULL) < 0) {
			ErrPrint("Unable to add a new job for refreshing box\n");
		}

		return LB_STATUS_SUCCESS;
	} else {
		/*!
		 * \brief
		 * No need to send this to the master
		 */
		return LB_STATUS_SUCCESS;
	}

	packet = packet_create_noack("change,visibility", "ssi", handler->common->pkgname, handler->common->id, (int)state);
	if (!packet) {
		ErrPrint("Failed to create a packet\n");
		return LB_STATUS_ERROR_FAULT;
	}

	ret = master_rpc_request_only(handler, packet);
	if (ret == LB_STATUS_SUCCESS) {
		DbgPrint("[%s] visibility is changed 0x[%x]\n", handler->common->pkgname, state);
		handler->common->visible = state;

		if (need_to_add_job) {
			if (job_add(handler, refresh_for_paused_updating_cb, LB_STATUS_SUCCESS, NULL) < 0) {
				ErrPrint("Unable to add a new job for refreshing box\n");
			}
		}
	}

	return ret;
}

EAPI struct livebox *livebox_add_with_size(const char *pkgname, const char *content, const char *cluster, const char *category, double period, int type, ret_cb_t cb, void *data)
{
	char *lbid;
	struct livebox *handler;
	int w = 0;
	int h = 0;

	if (!pkgname || !cluster || !category) {
		ErrPrint("Invalid arguments: pkgname[%p], cluster[%p], category[%p]\n",
								pkgname, cluster, category);
		return NULL;
	}

	lbid = lb_pkgname(pkgname);
	if (!lbid) {
		ErrPrint("Invalid package: %s\n", pkgname);
		return NULL;
	}

	if (livebox_service_is_enabled(lbid) == 0) {
		DbgPrint("Livebox [%s](%s) is disabled package\n", lbid, pkgname);
		free(lbid);
		return NULL;
	}

	if (type != LB_SIZE_TYPE_UNKNOWN) {
		(void)livebox_service_get_size(type, &w, &h);
	}

	handler = calloc(1, sizeof(*handler));
	if (!handler) {
		ErrPrint("Error: %s\n", strerror(errno));
		free(lbid);
		return NULL;
	}

	if (!cb) {
		cb = default_create_cb;
	}

	handler->common = find_sharable_common_handle(lbid, content, w, h, cluster, category);
	if (!handler->common) {
		handler->common = lb_create_common_handle(handler, lbid, cluster, category);
		free(lbid);
		if (!handler->common) {
			ErrPrint("Failed to find common handle\n");
			free(handler);
			return NULL;
		}

		if (!content || !strlen(content)) {
			char *pc;
			/*!
			 * \note
			 * I know the content should not be modified. use it temporarly without "const"
			 */
			pc = livebox_service_content(handler->common->pkgname);
			lb_set_content(handler->common, pc);
			free(pc);
		} else {
			lb_set_content(handler->common, content);
		}

		lb_set_period(handler->common, period);
		lb_set_size(handler->common, w, h);
		lb_common_ref(handler->common, handler);

		if (create_real_instance(handler, cb, data) < 0) {
			if (lb_common_unref(handler->common, handler) == 0) {
				/*!
				 * Delete common
				 */
				lb_destroy_common_handle(handler->common);
				handler->common = NULL;
			}
			free(handler);
			return NULL;
		}
	} else {
		free(lbid);

		lb_common_ref(handler->common, handler);

		if (handler->common->request.created) {
			/*!
			 * If a box is in creating, wait its result too
			 */
			handler->cbs.created.cb = cb;
			handler->cbs.created.data = data;
		} else {
			/*!
			 * or fire the fake created_event
			 */
			if (create_fake_instance(handler, cb, data) < 0) {
				if (lb_common_unref(handler->common, handler) == 0) {
					/*!
					 * Delete common
					 */
					lb_destroy_common_handle(handler->common);
				}
				free(handler);
				return NULL;
			}
		}
	}

	handler->visible = LB_SHOW;
	handler->state = CREATE;
	handler = lb_ref(handler);

	if (handler->common->visible != LB_SHOW) {
		lb_set_visibility(handler, LB_SHOW);
	}

	return handler;
}

EAPI double livebox_period(struct livebox *handler)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is not valid\n");
		return 0.0f;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Invalid handle\n");
		return 0.0f;
	}

	if (!handler->common->id) {
		ErrPrint("Hnalder is not valid\n");
		return 0.0f;
	}

	return handler->common->lb.period;
}

EAPI int livebox_set_period(struct livebox *handler, double period, ret_cb_t cb, void *data)
{
	struct packet *packet;
	int ret;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->common->request.period_changed) {
		ErrPrint("Previous request for changing period is not finished\n");
		return LB_STATUS_ERROR_BUSY;
	}

	if (!handler->common->is_user) {
		ErrPrint("CA Livebox is not able to change the period\n");
		return LB_STATUS_ERROR_PERMISSION;
	}

	if (handler->common->lb.period == period) {
		DbgPrint("No changes\n");
		return LB_STATUS_ERROR_ALREADY;
	}

	packet = packet_create("set_period", "ssd", handler->common->pkgname, handler->common->id, period);
	if (!packet) {
		ErrPrint("Failed to build a packet %s\n", handler->common->pkgname);
		return LB_STATUS_ERROR_FAULT;
	}

	if (!cb) {
		cb = default_period_changed_cb;
	}

	ret = master_rpc_async_request(handler, packet, 0, period_ret_cb, NULL);
	if (ret == LB_STATUS_SUCCESS) {
		handler->cbs.period_changed.cb = cb;
		handler->cbs.period_changed.data = data;
		handler->common->request.period_changed = 1;
	}

	return ret;
}

static void lb_update_visibility(struct livebox_common *old_common)
{
	struct dlist *l;
	struct livebox *item;

	item = NULL;
	dlist_foreach(old_common->livebox_list, l, item) {
		if (item->visible == LB_SHOW) {
			break;
		}

		item = NULL;
	}

	if (!item) {
		l = dlist_nth(old_common->livebox_list, 0);
		item = dlist_data(l);

		if (item) {
			lb_set_visibility(item, LB_HIDE_WITH_PAUSE);
		} else {
			ErrPrint("Unable to get the valid handle from common handler\n");
		}
	}
}

/*!
 * \note
 * The second parameter should be the "return value",
 * But in this case, we will use it for "type of deleting instance".
 */
static void job_del_cb(struct livebox *handle, int type, void *data)
{
	struct cb_info *cbinfo = data;
	ret_cb_t cb;

	if (handle->visible == LB_SHOW) {
		lb_update_visibility(handle->common);
	}

	cb = cbinfo->cb;
	data = cbinfo->data;
	destroy_cb_info(cbinfo);

	if (handle->common->refcnt == 1) {
		handle->common->delete_type = type;
		handle->common->state = DELETE;

		if (!handle->common->id) {
			/*!
			 * \note
			 * The id is not determined yet.
			 * It means a user didn't receive created event yet.
			 * Then just stop to delete procedure from here.
			 * Because the "created" event handle will release this.
			 * By the way, if the user adds any callback for getting return status of this,
			 * call it at here.
			 */
			if (cb) {
				cb(handle, LB_STATUS_SUCCESS, data);
			}
		}

		DbgPrint("Send delete request\n");
		lb_send_delete(handle, type, cb, data);
	} else {
		if (cb) {
			cb(handle, LB_STATUS_SUCCESS, data);
		}

		DbgPrint("Before unref: %d\n", handle->common->refcnt);
		lb_unref(handle, 1);
	}
}

EAPI int livebox_del_NEW(struct livebox *handler, int type, ret_cb_t cb, void *data)
{
	struct cb_info *cbinfo;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE) {
		ErrPrint("Handler is already deleted\n");
		return LB_STATUS_ERROR_INVALID;
	}

	handler->state = DELETE;

	cbinfo = create_cb_info(cb, data);
	if (!cbinfo) {
		ErrPrint("Failed to create a cbinfo\n");
		return LB_STATUS_ERROR_MEMORY;
	}

	if (job_add(handler, job_del_cb, type, cbinfo) != LB_STATUS_SUCCESS) {
		ErrPrint("Failed to add a new job\n");
		destroy_cb_info(cbinfo);
		return LB_STATUS_ERROR_FAULT;
	}

	return LB_STATUS_SUCCESS;
}

EAPI int livebox_del(struct livebox *handler, ret_cb_t cb, void *data)
{
	return livebox_del_NEW(handler, LB_DELETE_PERMANENTLY, cb, data);
}

EAPI int livebox_set_fault_handler(int (*cb)(enum livebox_fault_type, const char *, const char *, const char *, void *), void *data)
{
	struct fault_info *info;

	if (!cb) {
		return LB_STATUS_ERROR_INVALID;
	}

	info = malloc(sizeof(*info));
	if (!info) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return LB_STATUS_ERROR_MEMORY;
	}

	info->handler = cb;
	info->user_data = data;
	info->is_deleted = 0;

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

			data = info->user_data;

			if (s_info.fault_state == INFO_STATE_CALLBACK_IN_PROCESSING) {
				info->is_deleted = 1;
			} else {
				s_info.fault_list = dlist_remove(s_info.fault_list, l);
				free(info);
			}

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
		ErrPrint("Heap: %s\n", strerror(errno));
		return LB_STATUS_ERROR_MEMORY;
	}

	info->handler = cb;
	info->user_data = data;
	info->is_deleted = 0;

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

			data = info->user_data;

			if (s_info.event_state == INFO_STATE_CALLBACK_IN_PROCESSING) {
				info->is_deleted = 1;
			} else {
				s_info.event_list = dlist_remove(s_info.event_list, l);
				free(info);
			}

			return data;
		}
	}

	return NULL;
}

EAPI int livebox_set_update_mode(struct livebox *handler, int active_update, ret_cb_t cb, void *data)
{
	struct packet *packet;
	int ret;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is Invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is Invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is Invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->common->request.update_mode) {
		ErrPrint("Previous update_mode cb is not finished yet\n");
		return LB_STATUS_ERROR_BUSY;
	}

	if (handler->common->is_active_update == active_update) {
		return LB_STATUS_ERROR_ALREADY;
	}

	if (!handler->common->is_user) {
		return LB_STATUS_ERROR_PERMISSION;
	}

	packet = packet_create("update_mode", "ssi", handler->common->pkgname, handler->common->id, active_update);
	if (!packet) {
		return LB_STATUS_ERROR_FAULT;
	}

	if (!cb) {
		cb = default_update_mode_cb;
	}

	ret = master_rpc_async_request(handler, packet, 0, update_mode_cb, NULL);
	if (ret == LB_STATUS_SUCCESS) {
		handler->cbs.update_mode.cb = cb;
		handler->cbs.update_mode.data = data;
		handler->common->request.update_mode = 1;
	}

	return ret;
}

EAPI int livebox_is_active_update(struct livebox *handler)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is Invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is Invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		return LB_STATUS_ERROR_INVALID;
	}

	return handler->common->is_active_update;
}

static void resize_job_cb(struct livebox *handler, int ret, void *data)
{
	struct cb_info *info = data;

	if (info->cb) {
		info->cb(handler, ret, info->data);
	}

	free(info);

	/*!
	 * \note
	 * Forcely update the box
	 */
	lb_invoke_event_handler(handler, LB_EVENT_LB_UPDATED);
}

EAPI int livebox_resize(struct livebox *handler, int type, ret_cb_t cb, void *data)
{
	struct livebox_common *common;
	int w;
	int h;
	int ret;

	/*!
	 * \TODO
	 * If this handle is host instance or link instance,
	 * Create a new instance or find another linkable instance.
	 */

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	/*!
	 * \note
	 * resize operation should be separated by each handler.
	 * If a handler is resizing, the other handler can request resize too.
	 * So we should not use the common->request.size_changed flag.
	 */
	if (handler->cbs.size_changed.cb) {
		ErrPrint("Previous resize request is not finished yet\n");
		return LB_STATUS_ERROR_BUSY;
	}

	if (livebox_service_get_size(type, &w, &h) != 0) {
		ErrPrint("Invalid size type\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->common->lb.width == w && handler->common->lb.height == h) {
		DbgPrint("No changes\n");
		return LB_STATUS_ERROR_ALREADY;
	}

	if (!handler->common->is_user) {
		ErrPrint("CA Livebox is not able to be resized\n");
		return LB_STATUS_ERROR_PERMISSION;
	}

	if (handler->common->refcnt <= 1) {
		struct packet *packet;

		/* Only 1 instance */
		packet = packet_create("resize", "ssii", handler->common->pkgname, handler->common->id, w, h);
		if (!packet) {
			ErrPrint("Failed to build param\n");
			return LB_STATUS_ERROR_FAULT;
		}

		if (!cb) {
			cb = default_lb_size_changed_cb;
		}

		ret = master_rpc_async_request(handler, packet, 0, resize_cb, NULL);
		if (ret == LB_STATUS_SUCCESS) {
			handler->cbs.size_changed.cb = cb;
			handler->cbs.size_changed.data = data;
			handler->common->request.size_changed = 1;
		}
	} else {
		common = find_sharable_common_handle(handler->common->pkgname, handler->common->content, w, h, handler->common->cluster, handler->common->category);
		if (!common) {
			struct livebox_common *old_common;
			/*!
			 * \note
			 * If the common handler is in resizing,
			 * if user tries to resize a hander, then simply create new one even if the requested size is same with this.

			if (handler->common->request.size_changed) {
			}

			 */

			old_common = handler->common;

			common = lb_create_common_handle(handler, old_common->pkgname, old_common->cluster, old_common->category);
			if (!common) {
				ErrPrint("Failed to create common handle\n");
				return LB_STATUS_ERROR_FAULT;
			}

			lb_set_size(common, w, h);
			lb_set_content(common, old_common->content);
			lb_set_period(common, old_common->lb.period);

			/*!
			 * \note
			 * Disconnecting from old one.
			 */
			if (lb_common_unref(old_common, handler) == 0) {
				/*!
				 * \note
				 * Impossible
				 */
				ErrPrint("Common has no associated handler\n");
			}

			lb_common_ref(common, handler);

			/*!
			 * Connect to a new one
			 */
			handler->common = common;

			/*!
			 * \TODO
			 * Need to care, if it fails to create a common handle,
			 * the resize operation will be failed.
			 * in that case, we should reuse the old common handle
			 */
			ret = create_real_instance(handler, cb, data);
			if (ret < 0) {
				lb_common_unref(common, handler);
				lb_destroy_common_handle(common);

				lb_common_ref(old_common, handler);
				handler->common = old_common;
			} else {
				/*!
				 * In this case, we should update visibility of old_common's liveboxes
				 */
				if (handler->visible == LB_SHOW) {
					lb_update_visibility(old_common);
				}
			}
		} else {
			struct cb_info *cbinfo;

			cbinfo = create_cb_info(cb, data);
			if (!cbinfo) {
				ErrPrint("Failed to create a cbinfo\n");
				ret = LB_STATUS_ERROR_MEMORY;
			} else {
				ret = job_add(handler, resize_job_cb, LB_STATUS_SUCCESS, cbinfo);
				if (ret == LB_STATUS_SUCCESS) {
					struct livebox_common *old_common;

					old_common = handler->common;

					if (lb_common_unref(handler->common, handler) == 0) {
						ErrPrint("Old common has no associated handler\n");
					}

					lb_common_ref(common, handler);
					handler->common = common;

					if (handler->visible == LB_SHOW) {
						lb_update_visibility(old_common);
					}
				} else {
					destroy_cb_info(cbinfo);
				}
			}
		}
	}

	return ret;
}

EAPI int livebox_click(struct livebox *handler, double x, double y)
{
	struct packet *packet;
	double timestamp;
	int ret;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->common->lb.auto_launch) {
/*
		service_h service;

		DbgPrint("AUTO_LAUNCH [%s]\n", handler->common->lb.auto_launch);

		ret = service_create(&service);
		if (ret == SERVICE_ERROR_NONE) {
			service_set_package(service, handler->common->lb.auto_launch);
			service_send_launch_request(service, NULL, NULL);
			service_destroy(service);
		} else {
			ErrPrint("Failed to launch an app %s (%d)\n", handler->common->lb.auto_launch, ret);
		}
*/
		ret = aul_launch_app(handler->common->lb.auto_launch, NULL);
		if (ret <= 0) {
			ErrPrint("Failed to launch an app %s (%d)\n", handler->common->lb.auto_launch, ret);
		}
	}

	timestamp = util_timestamp();
	DbgPrint("CLICKED: %lf\n", timestamp);

	packet = packet_create_noack("clicked", "sssddd", handler->common->pkgname, handler->common->id, "clicked", timestamp, x, y);
	if (!packet) {
		ErrPrint("Failed to build param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	ret = master_rpc_request_only(handler, packet);

	if (!handler->common->lb.mouse_event && (handler->common->lb.type == _LB_TYPE_BUFFER || handler->common->lb.type == _LB_TYPE_SCRIPT)) {
		int ret; /* Shadow variable */
		ret = send_mouse_event(handler, "lb_mouse_down", x * handler->common->lb.width, y * handler->common->lb.height);
		if (ret < 0) {
			ErrPrint("Failed to send Down: %d\n", ret);
		}

		ret = send_mouse_event(handler, "lb_mouse_move", x * handler->common->lb.width, y * handler->common->lb.height);
		if (ret < 0) {
			ErrPrint("Failed to send Move: %d\n", ret);
		}

		ret = send_mouse_event(handler, "lb_mouse_up", x * handler->common->lb.width, y * handler->common->lb.height);
		if (ret < 0) {
			ErrPrint("Failed to send Up: %d\n", ret);
		}
	}

	return ret;
}

EAPI int livebox_has_pd(struct livebox *handler)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return !!handler->common->pd.fb;
}

EAPI int livebox_pd_is_created(struct livebox *handler)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->pd.fb || !handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return handler->common->is_pd_created;
}

EAPI int livebox_create_pd(struct livebox *handler, ret_cb_t cb, void *data)
{
	return livebox_create_pd_with_position(handler, -1.0, -1.0, cb, data);
}

EAPI int livebox_create_pd_with_position(struct livebox *handler, double x, double y, ret_cb_t cb, void *data)
{
	struct packet *packet;
	int ret;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->pd.fb || !handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	/*!
	 * \note
	 * Only one handler can have a PD
	 */
	if (handler->common->is_pd_created == 1) {
		DbgPrint("PD already created\n");
		return LB_STATUS_SUCCESS;
	}

	if (handler->common->request.pd_created) {
		ErrPrint("Previous request is not completed yet\n");
		return LB_STATUS_ERROR_BUSY;
	}

	packet = packet_create("create_pd", "ssdd", handler->common->pkgname, handler->common->id, x, y);
	if (!packet) {
		ErrPrint("Failed to build param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	if (!cb) {
		cb = default_pd_created_cb;
	}

	DbgPrint("PERF_DBOX\n");
	ret = master_rpc_async_request(handler, packet, 0, pd_create_cb, NULL);
	if (ret == LB_STATUS_SUCCESS) {
		handler->cbs.pd_created.cb = cb;
		handler->cbs.pd_created.data = data;
		handler->common->request.pd_created = 1;
	}

	return ret;
}

EAPI int livebox_move_pd(struct livebox *handler, double x, double y)
{
	struct packet *packet;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->pd.fb || !handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->is_pd_created) {
		ErrPrint("PD is not created\n");
		return LB_STATUS_ERROR_INVALID;
	}

	packet = packet_create_noack("pd_move", "ssdd", handler->common->pkgname, handler->common->id, x, y);
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

	if (!pkgname) {
		return LB_STATUS_ERROR_INVALID;
	}

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
	if (ret < 0) {
		destroy_cb_info(cbinfo);
	}

	return ret;
}

EAPI int livebox_destroy_pd(struct livebox *handler, ret_cb_t cb, void *data)
{
	struct packet *packet;
	struct cb_info *cbinfo;
	int ret;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->pd.fb || !handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	/*!
	 * \FIXME
	 * Replace the callback check code.
	 * Use the flag instead of callback.
	 * the flag should be in the ADT "common"
	 */
	if (!handler->common->is_pd_created && !handler->common->request.pd_created) {
		ErrPrint("PD is not created\n");
		return LB_STATUS_ERROR_INVALID;
	}

	packet = packet_create("destroy_pd", "ss", handler->common->pkgname, handler->common->id);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	if (!cb) {
		cb = default_pd_destroyed_cb;
	}

	cbinfo = create_cb_info(cb, data);
	if (!cbinfo) {
		packet_destroy(packet);
		return LB_STATUS_ERROR_FAULT;
	}

	ret = master_rpc_async_request(handler, packet, 0, pd_destroy_cb, cbinfo);
	if (ret < 0) {
		destroy_cb_info(cbinfo);
	}

	return ret;
}

EAPI int livebox_access_event(struct livebox *handler, enum access_event_type type, double x, double y, ret_cb_t cb, void *data)
{
	int w = 1;
	int h = 1;
	char cmd[32] = { '\0', };
	char *ptr = cmd;
	int ret;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->common->request.access_event) {
		ErrPrint("Previous access event is not yet done\n");
		return LB_STATUS_ERROR_BUSY;
	}

	if (type & ACCESS_EVENT_PD_MASK) {
		if (!handler->common->is_pd_created) {
			ErrPrint("PD is not created\n");
			return LB_STATUS_ERROR_INVALID;
		}
		*ptr++ = 'p';
		*ptr++ = 'd';
		w = handler->common->pd.width;
		h = handler->common->pd.height;
	} else if (type & ACCESS_EVENT_LB_MASK) {
		*ptr++ = 'l';
		*ptr++ = 'b';
		w = handler->common->lb.width;
		h = handler->common->lb.height;
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
	case ACCESS_EVENT_ACTION_DOWN:
		strcpy(ptr, "_access_action_down");
		break;
	case ACCESS_EVENT_ACTION_UP:
		strcpy(ptr, "_access_action_up");
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

	if (!cb) {
		cb = default_access_event_cb;
	}

	ret = send_access_event(handler, cmd, x * w, y * h);
	if (ret == LB_STATUS_SUCCESS) {
		handler->cbs.access_event.cb = cb;
		handler->cbs.access_event.data = data;
		handler->common->request.access_event = 1;
	}

	return ret;
}

EAPI int livebox_content_event(struct livebox *handler, enum content_event_type type, double x, double y)
{
	return livebox_mouse_event(handler, type, x, y);
}

EAPI int livebox_mouse_event(struct livebox *handler, enum content_event_type type, double x, double y)
{
	int w = 1;
	int h = 1;
	char cmd[32] = { '\0', };
	char *ptr = cmd;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!(type & CONTENT_EVENT_MOUSE_MASK)) {
		ErrPrint("Invalid content event is used\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (type & CONTENT_EVENT_PD_MASK) {
		int flag = 1;

		if (!handler->common->is_pd_created) {
			ErrPrint("PD is not created\n");
			return LB_STATUS_ERROR_INVALID;
		}

		if (!handler->common->pd.fb) {
			ErrPrint("Handler is not valid\n");
			return LB_STATUS_ERROR_INVALID;
		}

		if (type & CONTENT_EVENT_MOUSE_MOVE) {
			if (fabs(x - handler->common->pd.x) < MINIMUM_EVENT && fabs(y - handler->common->pd.y) < MINIMUM_EVENT) {
				return LB_STATUS_ERROR_BUSY;
			}
		} else if (type & CONTENT_EVENT_MOUSE_SET) {
			flag = 0;
		}

		if (flag) {
			w = handler->common->pd.width;
			h = handler->common->pd.height;
			handler->common->pd.x = x;
			handler->common->pd.y = y;
		}
		*ptr++ = 'p';
		*ptr++ = 'd';
	} else if (type & CONTENT_EVENT_LB_MASK) {
		int flag = 1;

		if (!handler->common->lb.mouse_event) {
			return LB_STATUS_ERROR_INVALID;
		}

		if (!handler->common->lb.fb) {
			ErrPrint("Handler is not valid\n");
			return LB_STATUS_ERROR_INVALID;
		}

		if (type & CONTENT_EVENT_MOUSE_MOVE) {
			if (fabs(x - handler->common->lb.x) < MINIMUM_EVENT && fabs(y - handler->common->lb.y) < MINIMUM_EVENT) {
				return LB_STATUS_ERROR_BUSY;
			}
		} else if (type & CONTENT_EVENT_MOUSE_SET) {
			flag = 0;
		}

		if (flag) {
			w = handler->common->lb.width;
			h = handler->common->lb.height;
			handler->common->lb.x = x;
			handler->common->lb.y = y;
		}
		*ptr++ = 'l';
		*ptr++ = 'b';
	} else {
		ErrPrint("Invalid event type\n");
		return LB_STATUS_ERROR_INVALID;
	}

	/*!
	 * Must be short than 29 bytes.
	 */
	switch ((type & ~(CONTENT_EVENT_PD_MASK | CONTENT_EVENT_LB_MASK))) {
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
	default:
		ErrPrint("Invalid event type\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return send_mouse_event(handler, cmd, x * w, y * h);
}

EAPI int livebox_key_event(struct livebox *handler, enum content_event_type type, unsigned int keycode, ret_cb_t cb, void *data)
{
	char cmd[32] = { '\0', };
	char *ptr = cmd;
	int ret;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!(type & CONTENT_EVENT_KEY_MASK)) {
		ErrPrint("Invalid key event is used\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->common->request.key_event) {
		ErrPrint("Previous key event is not completed yet\n");
		return LB_STATUS_ERROR_BUSY;
	}

	if (type & CONTENT_EVENT_PD_MASK) {
		if (!handler->common->is_pd_created) {
			ErrPrint("PD is not created\n");
			return LB_STATUS_ERROR_INVALID;
		}

		if (!handler->common->pd.fb) {
			ErrPrint("Handler is not valid\n");
			return LB_STATUS_ERROR_INVALID;
		}

		if (type & CONTENT_EVENT_KEY_DOWN) {
			/*!
			 * \TODO
			 * filtering the reproduced events if it is too fast
			 */
		} else if (type & CONTENT_EVENT_KEY_SET) {
			/*!
			 * \TODO
			 * What can I do for this case?
			 */
		}

		*ptr++ = 'p';
		*ptr++ = 'd';
	} else if (type & CONTENT_EVENT_LB_MASK) {
		if (!handler->common->lb.mouse_event) {
			return LB_STATUS_ERROR_INVALID;
		}

		if (!handler->common->lb.fb) {
			ErrPrint("Handler is not valid\n");
			return LB_STATUS_ERROR_INVALID;
		}

		if (type & CONTENT_EVENT_KEY_DOWN) {
			/*!
			 * \TODO
			 * filtering the reproduced events if it is too fast
			 */
		} else if (type & CONTENT_EVENT_KEY_SET) {
			/*!
			 * What can I do for this case?
			 */
		}

		*ptr++ = 'l';
		*ptr++ = 'b';
	} else {
		ErrPrint("Invalid event type\n");
		return LB_STATUS_ERROR_INVALID;
	}

	/*!
	 * Must be short than 29 bytes.
	 */
	switch ((type & ~(CONTENT_EVENT_PD_MASK | CONTENT_EVENT_LB_MASK))) {
	case CONTENT_EVENT_KEY_FOCUS_IN | CONTENT_EVENT_KEY_MASK:
		strcpy(ptr, "_key_focus_in");
		break;
	case CONTENT_EVENT_KEY_FOCUS_OUT | CONTENT_EVENT_KEY_MASK:
		strcpy(ptr, "_key_focus_out");
		break;
	case CONTENT_EVENT_KEY_UP | CONTENT_EVENT_KEY_MASK:
		strcpy(ptr, "_key_up");
		break;
	case CONTENT_EVENT_KEY_DOWN | CONTENT_EVENT_KEY_MASK:
		strcpy(ptr, "_key_down");
		break;
	case CONTENT_EVENT_KEY_SET | CONTENT_EVENT_KEY_MASK:
		strcpy(ptr, "_key_set");
		break;
	case CONTENT_EVENT_KEY_UNSET | CONTENT_EVENT_KEY_MASK:
		strcpy(ptr, "_key_unset");
		break;
	default:
		ErrPrint("Invalid event type\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!cb) {
		cb = default_key_event_cb;
	}

	ret = send_key_event(handler, cmd, keycode);
	if (ret == LB_STATUS_SUCCESS) {
		handler->cbs.key_event.cb = cb;
		handler->cbs.key_event.data = data;
		handler->common->request.key_event = 1;
	}

	return ret;
}

EAPI const char *livebox_filename(struct livebox *handler)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return NULL;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return NULL;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return NULL;
	}

	if (handler->common->filename) {
		return handler->common->filename;
	}

	/* Oooops */
	return util_uri_to_path(handler->common->id);
}

EAPI int livebox_get_pdsize(struct livebox *handler, int *w, int *h)
{
	int _w;
	int _h;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!w) {
		w = &_w;
	}
	if (!h) {
		h = &_h;
	}

	if (!handler->common->is_pd_created) {
		*w = handler->common->pd.default_width;
		*h = handler->common->pd.default_height;
	} else {
		*w = handler->common->pd.width;
		*h = handler->common->pd.height;
	}

	return LB_STATUS_SUCCESS;
}

EAPI int livebox_size(struct livebox *handler)
{
	int w;
	int h;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	w = handler->common->lb.width;
	h = handler->common->lb.height;

	switch (handler->common->lb.type) {
	case _LB_TYPE_BUFFER:
	case _LB_TYPE_SCRIPT:
		if (!fb_is_created(handler->common->lb.fb)) {
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

	if (!cluster || !category || handler->state != CREATE) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->common->request.group_changed) {
		ErrPrint("Previous group changing request is not finished yet\n");
		return LB_STATUS_ERROR_BUSY;
	}

	if (!handler->common->is_user) {
		ErrPrint("CA Livebox is not able to change the group\n");
		return LB_STATUS_ERROR_PERMISSION;
	}

	if (!strcmp(handler->common->cluster, cluster) && !strcmp(handler->common->category, category)) {
		DbgPrint("No changes\n");
		return LB_STATUS_ERROR_ALREADY;
	}

	packet = packet_create("change_group", "ssss", handler->common->pkgname, handler->common->id, cluster, category);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	if (!cb) {
		cb = default_group_changed_cb;
	}

	ret = master_rpc_async_request(handler, packet, 0, set_group_ret_cb, NULL);
	if (ret == LB_STATUS_SUCCESS) {
		handler->cbs.group_changed.cb = cb;
		handler->cbs.group_changed.data = data; 
		handler->common->request.group_changed = 1;
	}

	return ret;
}

EAPI int livebox_get_group(struct livebox *handler, const char **cluster, const char **category)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!cluster || !category || handler->state != CREATE) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	*cluster = handler->common->cluster;
	*category = handler->common->category;
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

	if (!cnt || handler->state != CREATE) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	for (j = i = 0; i < NR_OF_SIZE_LIST; i++) {
		if (handler->common->lb.size_list & (0x01 << i)) {
			if (j == *cnt) {
				break;
			}

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

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is not valid\n");
		return NULL;
	}

	return handler->common->pkgname;
}

EAPI double livebox_priority(struct livebox *handler)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return -1.0f;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return -1.0f;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid (%p)\n", handler);
		return -1.0f;
	}

	return handler->common->lb.priority;
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
	if (ret < 0) {
		destroy_cb_info(cbinfo);
	}

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
	if (ret < 0) {
		destroy_cb_info(cbinfo);
	}

	return ret;
}

EAPI enum livebox_lb_type livebox_lb_type(struct livebox *handler)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_TYPE_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_TYPE_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_TYPE_INVALID;
	}

	switch (handler->common->lb.type) {
	case _LB_TYPE_FILE:
		return LB_TYPE_IMAGE;
	case _LB_TYPE_BUFFER:
	case _LB_TYPE_SCRIPT:
		{
			const char *id;
			id = fb_id(handler->common->lb.fb);
			if (id && !strncasecmp(id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP))) {
				return LB_TYPE_PIXMAP;
			}
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
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return PD_TYPE_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return PD_TYPE_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return PD_TYPE_INVALID;
	}

	switch (handler->common->pd.type) {
	case _PD_TYPE_TEXT:
		return PD_TYPE_TEXT;
	case _PD_TYPE_BUFFER:
	case _PD_TYPE_SCRIPT:
		{
			const char *id;
			id = fb_id(handler->common->pd.fb);
			if (id && !strncasecmp(id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP))) {
				return PD_TYPE_PIXMAP;
			}
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

	memcpy(&handler->cbs.pd_ops, ops, sizeof(*ops));
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

	memcpy(&handler->cbs.lb_ops, ops, sizeof(*ops));
	return LB_STATUS_SUCCESS;
}

EAPI int livebox_acquire_lb_pixmap(struct livebox *handler, ret_cb_t cb, void *data)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->common->lb.type != _LB_TYPE_SCRIPT && handler->common->lb.type != _LB_TYPE_BUFFER) {
		ErrPrint("Handler is not valid type\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return lb_acquire_lb_pixmap(handler, cb, data);
}

EAPI int livebox_release_lb_pixmap(struct livebox *handler, int pixmap)
{
	struct packet *packet;

	if (!handler || pixmap == 0 || handler->state != CREATE) {
		ErrPrint("Handler is invalid [%d]\n", pixmap);
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->common->lb.type != _LB_TYPE_SCRIPT && handler->common->lb.type != _LB_TYPE_BUFFER) {
		ErrPrint("Handler is not valid type\n");
		return LB_STATUS_ERROR_INVALID;
	}

	packet = packet_create_noack("lb_release_pixmap", "ssi", handler->common->pkgname, handler->common->id, pixmap);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return master_rpc_request_only(handler, packet);
}

EAPI int livebox_acquire_pd_pixmap(struct livebox *handler, ret_cb_t cb, void *data)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->common->pd.type != _PD_TYPE_SCRIPT && handler->common->pd.type != _PD_TYPE_BUFFER) {
		ErrPrint("Handler is not valid type\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return lb_acquire_pd_pixmap(handler, cb, data);
}

EAPI int livebox_pd_pixmap(const struct livebox *handler)
{
	const char *id;
	int pixmap = 0;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return 0;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return 0;
	}

	if (!handler->common->id) {
		ErrPrint("Invalid handler\n");
		return 0;
	}

	if (handler->common->pd.type != _PD_TYPE_SCRIPT && handler->common->pd.type != _PD_TYPE_BUFFER) {
		ErrPrint("Invalid handler\n");
		return 0;
	}

	id = fb_id(handler->common->pd.fb);
	if (id && sscanf(id, SCHEMA_PIXMAP "%u", (unsigned int *)&pixmap) != 1) {
		ErrPrint("PIXMAP Id is not valid\n");
		return 0;
	}

	return pixmap;
}

EAPI int livebox_lb_pixmap(const struct livebox *handler)
{
	const char *id;
	int pixmap = 0;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return 0;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return 0;
	}

	if (!handler->common->id) {
		ErrPrint("Invalid handler\n");
		return 0;
	}

	if (handler->common->lb.type != _LB_TYPE_SCRIPT && handler->common->lb.type != _LB_TYPE_BUFFER) {
		ErrPrint("Invalid handler\n");
		return 0;
	}

	id = fb_id(handler->common->lb.fb);
	if (id && sscanf(id, SCHEMA_PIXMAP "%u", (unsigned int *)&pixmap) != 1) {
		ErrPrint("PIXMAP Id is not valid\n");
		return 0;
	}

	return pixmap;
}

EAPI int livebox_release_pd_pixmap(struct livebox *handler, int pixmap)
{
	struct packet *packet;

	if (!handler || pixmap == 0 || handler->state != CREATE) {
		ErrPrint("Handler is invalid [%d]\n", pixmap);
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->common->pd.type != _PD_TYPE_SCRIPT && handler->common->pd.type != _PD_TYPE_BUFFER) {
		ErrPrint("Handler is not valid type\n");
		return LB_STATUS_ERROR_INVALID;
	}

	packet = packet_create_noack("pd_release_pixmap", "ssi", handler->common->pkgname, handler->common->id, pixmap);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	return master_rpc_request_only(handler, packet);
}

EAPI void *livebox_acquire_fb(struct livebox *handler)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return NULL;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return NULL;
	}

	if (!handler->common->id) {
		ErrPrint("Invalid handle\n");
		return NULL;
	}

	if (handler->common->lb.type != _LB_TYPE_SCRIPT && handler->common->lb.type != _LB_TYPE_BUFFER) {
		ErrPrint("Handler is not valid type\n");
		return NULL;
	}

	return fb_acquire_buffer(handler->common->lb.fb);
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
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return NULL;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return NULL;
	}

	if (!handler->common->id) {
		ErrPrint("Invalid handler\n");
		return NULL;
	}

	if (handler->common->pd.type != _PD_TYPE_SCRIPT && handler->common->pd.type != _PD_TYPE_BUFFER) {
		ErrPrint("Handler is not valid type\n");
		return NULL;
	}

	return fb_acquire_buffer(handler->common->pd.fb);
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
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Invalid handler\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return fb_size(handler->common->pd.fb);
}

EAPI int livebox_lbfb_bufsz(struct livebox *handler)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Invalid handler\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return fb_size(handler->common->lb.fb);
}

EAPI int livebox_is_user(struct livebox *handler)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Invalid handler\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return handler->common->is_user;
}

EAPI int livebox_set_pinup(struct livebox *handler, int flag, ret_cb_t cb, void *data)
{
	struct packet *packet;
	int ret;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Invalid handler\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->common->request.pinup) {
		ErrPrint("Previous pinup request is not finished\n");
		return LB_STATUS_ERROR_BUSY;
	}

	if (handler->common->is_pinned_up == flag) {
		DbgPrint("No changes\n");
		return LB_STATUS_ERROR_ALREADY;
	}

	packet = packet_create("pinup_changed", "ssi", handler->common->pkgname, handler->common->id, flag);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return LB_STATUS_ERROR_FAULT;
	}

	if (!cb) {
		cb = default_pinup_cb;
	}

	ret = master_rpc_async_request(handler, packet, 0, pinup_done_cb, NULL);
	if (ret == LB_STATUS_SUCCESS) {
		handler->cbs.pinup.cb = cb;
		handler->cbs.pinup.data = data;
		handler->common->request.pinup = 1;
	}

	return ret;
}

EAPI int livebox_is_pinned_up(struct livebox *handler)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Invalid handler\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return handler->common->is_pinned_up;
}

EAPI int livebox_has_pinup(struct livebox *handler)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Invalid handler\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return handler->common->lb.pinup_supported;
}

EAPI int livebox_set_data(struct livebox *handler, void *data)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	handler->data = data;
	return LB_STATUS_SUCCESS;
}

EAPI void *livebox_get_data(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return NULL;
	}

	if (handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return NULL;
	}

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
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return NULL;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Invalid handle\n");
		return NULL;
	}

	return handler->common->content;
}

EAPI const char *livebox_category_title(struct livebox *handler)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return NULL;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Invalid handle\n");
		return NULL;
	}

	return handler->common->title;
}

EAPI int livebox_emit_text_signal(struct livebox *handler, const char *emission, const char *source, double sx, double sy, double ex, double ey, ret_cb_t cb, void *data)
{
	struct packet *packet;
	struct cb_info *cbinfo;
	int ret;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if ((handler->common->lb.type != _LB_TYPE_TEXT && handler->common->pd.type != _PD_TYPE_TEXT) || !handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!emission) {
		emission = "";
	}

	if (!source) {
		source = "";
	}

	packet = packet_create("text_signal", "ssssdddd",
				handler->common->pkgname, handler->common->id, emission, source, sx, sy, ex, ey);
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
	if (ret < 0) {
		destroy_cb_info(cbinfo);
	}

	return ret;
}

EAPI int livebox_subscribe_group(const char *cluster, const char *category)
{
	struct packet *packet;

	/*!
	 * \todo
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
	 * \todo
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

EAPI int livebox_refresh(struct livebox *handler, int force)
{
	struct packet *packet;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	packet = packet_create_noack("update", "ssi", handler->common->pkgname, handler->common->id, force);
	if (!packet) {
		ErrPrint("Failed to create a packet\n");
		return LB_STATUS_ERROR_FAULT;
	}

	return master_rpc_request_only(handler, packet);
}

EAPI int livebox_refresh_group(const char *cluster, const char *category, int force)
{
	struct packet *packet;

	if (!cluster || !category) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	packet = packet_create_noack("refresh_group", "ssi", cluster, category, force);
	if (!packet) {
		ErrPrint("Failed to create a packet\n");
		return LB_STATUS_ERROR_FAULT;
	}

	return master_rpc_request_only(NULL, packet);
}

EAPI int livebox_set_visibility(struct livebox *handler, enum livebox_visible_state state)
{
	int old_state;
	int ret;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->is_user) {
		/* System cluster livebox cannot be changed its visible states */
		if (state == LB_HIDE_WITH_PAUSE) {
			ErrPrint("CA Livebox is not able to change the visibility\n");
			return LB_STATUS_ERROR_PERMISSION;
		}
	}

	DbgPrint("[%s] Change visiblity to 0x%x\n", handler->common->pkgname, state);

	if (handler->visible == state) {
		DbgPrint("%s has no changes\n", handler->common->pkgname);
		return LB_STATUS_ERROR_ALREADY;
	}

	old_state = handler->visible;
	handler->visible = state;

	ret = lb_set_visibility(handler, state);
	if (ret < 0) {
		handler->visible = old_state;
	}

	return ret;
}

EAPI enum livebox_visible_state livebox_visibility(struct livebox *handler)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is invalid\n");
		return LB_VISIBLE_ERROR;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is not valid\n");
		return LB_VISIBLE_ERROR;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid\n");
		return LB_VISIBLE_ERROR;
	}

	return handler->visible;
}

int lb_set_group(struct livebox_common *common, const char *cluster, const char *category)
{
	void *pc = NULL;
	void *ps = NULL;

	if (cluster) {
		pc = strdup(cluster);
		if (!pc) {
			ErrPrint("Heap: %s (cluster: %s)\n", strerror(errno), cluster);
			return LB_STATUS_ERROR_MEMORY;
		}
	}

	if (category) {
		ps = strdup(category);
		if (!ps) {
			ErrPrint("Heap: %s (category: %s)\n", strerror(errno), category);
			free(pc);
			return LB_STATUS_ERROR_MEMORY;
		}
	}

	if (common->cluster) {
		free(common->cluster);
	}

	if (common->category) {
		free(common->category);
	}

	common->cluster = pc;
	common->category = ps;

	return LB_STATUS_SUCCESS;
}

void lb_set_size(struct livebox_common *common, int w, int h)
{
	common->lb.width = w;
	common->lb.height = h;
}

void lb_set_update_mode(struct livebox_common *common, int active_mode)
{
	common->is_active_update = active_mode;
}

void lb_set_pdsize(struct livebox_common *common, int w, int h)
{
	common->pd.width = w;
	common->pd.height = h;
}

void lb_set_default_pdsize(struct livebox_common *common, int w, int h)
{
	common->pd.default_width = w;
	common->pd.default_height = h;
}

void lb_invoke_fault_handler(enum livebox_fault_type event, const char *pkgname, const char *file, const char *func)
{
	struct dlist *l;
	struct dlist *n;
	struct fault_info *info;

	s_info.fault_state = INFO_STATE_CALLBACK_IN_PROCESSING;

	dlist_foreach_safe(s_info.fault_list, l, n, info) {
		if (!info->is_deleted && info->handler(event, pkgname, file, func, info->user_data) == EXIT_FAILURE) {
			info->is_deleted = 1;
		}

		if (info->is_deleted) {
			s_info.fault_list = dlist_remove(s_info.fault_list, l);
			free(info);
		}
	}

	s_info.fault_state &= ~INFO_STATE_CALLBACK_IN_PROCESSING;
}

void lb_invoke_event_handler(struct livebox *handler, enum livebox_event_type event)
{
	struct dlist *l;
	struct dlist *n;
	struct event_info *info;

	if (event == LB_EVENT_LB_UPDATED && handler->common->refcnt > 1) {
		if (handler->visible != LB_SHOW) {
			DbgPrint("Update requested(pending) - %s\n", handler->common->pkgname);
			handler->paused_updating++;
			return;
		} else {
			handler->paused_updating = 0;
		}
	}

	s_info.event_state = INFO_STATE_CALLBACK_IN_PROCESSING;

	dlist_foreach_safe(s_info.event_list, l, n, info) {
		if (!info->is_deleted && info->handler(handler, event, info->user_data) == EXIT_FAILURE) {
			DbgPrint("Event handler returns EXIT_FAILURE\n");
			info->is_deleted = 1;
		}

		if (info->is_deleted) {
			s_info.event_list = dlist_remove(s_info.event_list, l);
			free(info);
		}
	}

	s_info.event_state &= ~INFO_STATE_CALLBACK_IN_PROCESSING;
}

struct livebox_common *lb_find_common_handle(const char *pkgname, const char *id)
{
	struct dlist *l;
	struct livebox_common *common;

	dlist_foreach(s_info.livebox_common_list, l, common) {
		if (!common->id) {
			continue;
		}

		if (!strcmp(common->pkgname, pkgname) && !strcmp(common->id, id)) {
			return common;
		}
	}

	return NULL;
}

struct livebox_common *lb_find_common_handle_by_timestamp(double timestamp)
{
	struct dlist *l;
	struct livebox_common *common;

	dlist_foreach(s_info.livebox_common_list, l, common) {
		if (common->timestamp == timestamp) {
			return common;
		}
	}

	return NULL;
}

struct livebox *lb_new_livebox(const char *pkgname, const char *id, double timestamp, const char *cluster, const char *category)
{
	struct livebox *handler;

	handler = calloc(1, sizeof(*handler));
	if (!handler) {
		ErrPrint("Failed to create a new livebox\n");
		return NULL;
	}

	handler->common = lb_create_common_handle(handler, pkgname, cluster, category);
	if (!handler->common) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(handler);
		return NULL;
	}

	lb_common_ref(handler->common, handler);
	lb_set_id(handler->common, id);
	handler->common->timestamp = timestamp;
	handler->common->state = CREATE;
	handler->visible = LB_SHOW;
	s_info.livebox_list = dlist_append(s_info.livebox_list, handler);

	return lb_ref(handler);
}

int lb_delete_all(void)
{
	struct dlist *l;
	struct dlist *n;
	struct livebox *handler;

	dlist_foreach_safe(s_info.livebox_list, l, n, handler) {
		lb_invoke_event_handler(handler, LB_EVENT_DELETED);
		lb_unref(handler, 1);
	}

	return LB_STATUS_SUCCESS;
}

int lb_set_content(struct livebox_common *common, const char *content)
{
	char *pc = NULL;

	if (content) {
		pc = strdup(content);
		if (!pc) {
			ErrPrint("heap: %s [%s]\n", strerror(errno), content);
			return LB_STATUS_ERROR_MEMORY;
		}
	}

	free(common->content);
	common->content = pc;
	return LB_STATUS_SUCCESS;
}

int lb_set_title(struct livebox_common *common, const char *title)
{
	char *pt = NULL;

	if (title) {
		pt = strdup(title);
		if (!pt) {
			ErrPrint("heap: %s [%s]\n", strerror(errno), title);
			return LB_STATUS_ERROR_MEMORY;
		}
	}

	free(common->title);
	common->title = pt;
	return LB_STATUS_SUCCESS;
}

void lb_set_size_list(struct livebox_common *common, int size_list)
{
	common->lb.size_list = size_list;
}

void lb_set_auto_launch(struct livebox_common *common, const char *auto_launch)
{
	char *pa = NULL;

	if (!auto_launch || !strlen(auto_launch)) {
		return;
	}

	pa = strdup(auto_launch);
	if (!pa) {
		ErrPrint("heap: %s, [%s]\n", strerror(errno), auto_launch);
		return;
	}

	free(common->lb.auto_launch);
	common->lb.auto_launch = pa;
}

void lb_set_priority(struct livebox_common *common, double priority)
{
	common->lb.priority = priority;
}

void lb_set_id(struct livebox_common *common, const char *id)
{
	char *pi = NULL;

	if (id) {
		pi = strdup(id);
		if (!pi) {
			ErrPrint("heap: %s [%s]\n", strerror(errno), pi);
			return;
		}
	}

	free(common->id);
	common->id = pi;
}

void lb_set_filename(struct livebox_common *common, const char *filename)
{
	if (common->filename) {
		if (common->lb.type == _LB_TYPE_FILE || common->lb.type == _LB_TYPE_TEXT) {
			if (common->filename[0] && unlink(common->filename) < 0) {
				ErrPrint("unlink: %s (%s)\n", strerror(errno), common->filename);
			}
		}

		free(common->filename);
	}

	common->filename = strdup(filename);
	if (!common->filename) {
		ErrPrint("Heap: %s\n", strerror(errno));
	}
}

void lb_set_alt_info(struct livebox_common *common, const char *icon, const char *name)
{
	char *_icon = NULL;
	char *_name = NULL;

	if (icon && strlen(icon)) {
		_icon = strdup(icon);
		if (!_icon) {
			ErrPrint("Heap: %s\n", strerror(errno));
		}
	}

	if (name && strlen(name)) {
		_name = strdup(name);
		if (!_name) {
			ErrPrint("Heap: %s\n", strerror(errno));
		}
	}

	free(common->alt.icon);
	common->alt.icon = _icon;

	free(common->alt.name);
	common->alt.name = _name;
}

int lb_set_lb_fb(struct livebox_common *common, const char *filename)
{
	struct fb_info *fb;

	if (!common) {
		return LB_STATUS_ERROR_INVALID;
	}

	fb = common->lb.fb;
	if (fb && !strcmp(fb_id(fb), filename)) { /*!< BUFFER is not changed, */
		return LB_STATUS_SUCCESS;
	}

	common->lb.fb = NULL;

	if (!filename || filename[0] == '\0') {
		if (fb) {
			fb_destroy(fb);
		}
		return LB_STATUS_SUCCESS;
	}

	common->lb.fb = fb_create(filename, common->lb.width, common->lb.height);
	if (!common->lb.fb) {
		ErrPrint("Faield to create a FB\n");
		if (fb) {
			fb_destroy(fb);
		}
		return LB_STATUS_ERROR_FAULT;
	}

	if (fb) {
		fb_destroy(fb);
	}

	return LB_STATUS_SUCCESS;
}

int lb_set_pd_fb(struct livebox_common *common, const char *filename)
{
	struct fb_info *fb;

	if (!common || common->state != CREATE) {
		return LB_STATUS_ERROR_INVALID;
	}

	fb = common->pd.fb;
	if (fb && !strcmp(fb_id(fb), filename)) {
		/* BUFFER is not changed, just update the content */
		return LB_STATUS_ERROR_EXIST;
	}
	common->pd.fb = NULL;

	if (!filename || filename[0] == '\0') {
		if (fb) {
			fb_destroy(fb);
		}
		return LB_STATUS_SUCCESS;
	}

	common->pd.fb = fb_create(filename, common->pd.width, common->pd.height);
	if (!common->pd.fb) {
		ErrPrint("Failed to create a FB\n");
		if (fb) {
			fb_destroy(fb);
		}
		return LB_STATUS_ERROR_FAULT;
	}

	if (fb) {
		fb_destroy(fb);
	}
	return LB_STATUS_SUCCESS;
}

struct fb_info *lb_get_lb_fb(struct livebox_common *common)
{
	return common->lb.fb;
}

struct fb_info *lb_get_pd_fb(struct livebox_common *common)
{
	return common->pd.fb;
}

void lb_set_user(struct livebox_common *common, int user)
{
	common->is_user = user;
}

void lb_set_pinup(struct livebox_common *common, int pinup_supported)
{
	common->lb.pinup_supported = pinup_supported;
}

void lb_set_text_lb(struct livebox_common *common)
{
	common->lb.type = _LB_TYPE_TEXT;
}

void lb_set_text_pd(struct livebox_common *common)
{
	common->pd.type = _PD_TYPE_TEXT;
}

int lb_text_lb(struct livebox_common *common)
{
	return common->lb.type == _LB_TYPE_TEXT;
}

int lb_text_pd(struct livebox_common *common)
{
	return common->pd.type == _PD_TYPE_TEXT;
}

void lb_set_period(struct livebox_common *common, double period)
{
	common->lb.period = period;
}

struct livebox *lb_ref(struct livebox *handler)
{
	if (!handler) {
		return NULL;
	}

	handler->refcnt++;
	return handler;
}

struct livebox *lb_unref(struct livebox *handler, int destroy_common)
{
	if (!handler) {
		return NULL;
	}

	handler->refcnt--;
	if (handler->refcnt > 0) {
		return handler;
	}

	if (handler->cbs.created.cb) {
		handler->cbs.created.cb(handler, LB_STATUS_ERROR_FAULT, handler->cbs.created.data);
		handler->cbs.created.cb = NULL;
		handler->cbs.created.data = NULL;
	}

	if (handler->cbs.deleted.cb) {
		handler->cbs.deleted.cb(handler, LB_STATUS_ERROR_FAULT, handler->cbs.deleted.data);
		handler->cbs.deleted.cb = NULL;
		handler->cbs.deleted.data = NULL;
	}

	if (handler->cbs.pinup.cb) {
		handler->cbs.pinup.cb(handler, LB_STATUS_ERROR_FAULT, handler->cbs.pinup.data);
		handler->cbs.pinup.cb = NULL;
		handler->cbs.pinup.data = NULL;
	}

	if (handler->cbs.group_changed.cb) {
		handler->cbs.group_changed.cb(handler, LB_STATUS_ERROR_FAULT, handler->cbs.group_changed.data);
		handler->cbs.group_changed.cb = NULL;
		handler->cbs.group_changed.data = NULL;
	}

	if (handler->cbs.period_changed.cb) {
		handler->cbs.period_changed.cb(handler, LB_STATUS_ERROR_FAULT, handler->cbs.period_changed.data);
		handler->cbs.period_changed.cb = NULL;
		handler->cbs.period_changed.data = NULL;
	}

	if (handler->cbs.size_changed.cb) {
		handler->cbs.size_changed.cb(handler, LB_STATUS_ERROR_FAULT, handler->cbs.size_changed.data);
		handler->cbs.size_changed.cb = NULL;
		handler->cbs.size_changed.data = NULL;
	}

	if (handler->cbs.pd_created.cb) {
		handler->cbs.pd_created.cb(handler, LB_STATUS_ERROR_FAULT, handler->cbs.pd_created.data);
		handler->cbs.pd_created.cb = NULL;
		handler->cbs.pd_created.data = NULL;
	}

	if (handler->cbs.pd_destroyed.cb) {
		handler->cbs.pd_destroyed.cb(handler, LB_STATUS_ERROR_FAULT, handler->cbs.pd_destroyed.data);
		handler->cbs.pd_destroyed.cb = NULL;
		handler->cbs.pd_destroyed.data = NULL;
	}

	if (handler->cbs.update_mode.cb) {
		handler->cbs.update_mode.cb(handler, LB_STATUS_ERROR_FAULT, handler->cbs.update_mode.data);
		handler->cbs.update_mode.cb = NULL;
		handler->cbs.update_mode.data = NULL;
	}

	if (handler->cbs.access_event.cb) {
		handler->cbs.access_event.cb(handler, LB_ACCESS_STATUS_ERROR, handler->cbs.access_event.data);
		handler->cbs.access_event.cb = NULL;
		handler->cbs.access_event.data = NULL;
	}

	if (handler->cbs.key_event.cb) {
		handler->cbs.key_event.cb(handler, LB_KEY_STATUS_ERROR, handler->cbs.key_event.data);
		handler->cbs.key_event.cb = NULL;
		handler->cbs.key_event.data = NULL;
	}

	if (handler->common->filename) {
		(void)util_unlink(handler->common->filename);
	}

	dlist_remove_data(s_info.livebox_list, handler);

	handler->state = DESTROYED;
	if (lb_common_unref(handler->common, handler) == 0) {
		if (destroy_common) {
			lb_destroy_common_handle(handler->common);
		}
	}
	free(handler);
	return NULL;
}

int lb_send_delete(struct livebox *handler, int type, ret_cb_t cb, void *data)
{
	struct packet *packet;
	struct cb_info *cbinfo;
	int ret;

	if (handler->common->request.deleted) {
		ErrPrint("Already in-progress\n");
		if (cb) {
			cb(handler, LB_STATUS_SUCCESS, data);
		}
		return LB_STATUS_ERROR_BUSY;
	}

	if (!cb) {
		cb = default_delete_cb;
	}

	packet = packet_create("delete", "ssi", handler->common->pkgname, handler->common->id, type);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		if (cb) {
			cb(handler, LB_STATUS_ERROR_FAULT, data);
		}

		return LB_STATUS_ERROR_FAULT;
	}

	cbinfo = create_cb_info(cb, data);
	if (!cbinfo) {
		packet_destroy(packet);
		ErrPrint("Failed to create cbinfo\n");
		if (cb) {
			cb(handler, LB_STATUS_ERROR_FAULT, data);
		}

		return LB_STATUS_ERROR_FAULT;
	}

	ret = master_rpc_async_request(handler, packet, 0, del_ret_cb, cbinfo);
	if (ret < 0) {
		/*!
		 * Packet is destroyed by master_rpc_async_request.
		 */
		destroy_cb_info(cbinfo);

		if (cb) {
			cb(handler, LB_STATUS_ERROR_FAULT, data);
		}
	} else {
		handler->common->request.deleted = 1;
	}

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

EAPI int livebox_sync_lb_fb(struct livebox *handler)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		return LB_STATUS_ERROR_INVALID;
	}

	return lb_sync_lb_fb(handler->common);
}

int lb_sync_lb_fb(struct livebox_common *common)
{
	int ret;

	if (fb_type(lb_get_lb_fb(common)) == BUFFER_TYPE_FILE && common->lb.lock_fd >= 0) {
		(void)do_fb_lock(common->lb.lock_fd);
		ret = fb_sync(lb_get_lb_fb(common));
		(void)do_fb_unlock(common->lb.lock_fd);
	} else {
		ret = fb_sync(lb_get_lb_fb(common));
	}

	return ret;
}

int lb_sync_pd_fb(struct livebox_common *common)
{
	int ret;

	if (fb_type(lb_get_pd_fb(common)) == BUFFER_TYPE_FILE && common->pd.lock_fd >= 0) {
		(void)do_fb_lock(common->pd.lock_fd);
		ret = fb_sync(lb_get_pd_fb(common));
		(void)do_fb_unlock(common->pd.lock_fd);
	} else {
		ret = fb_sync(lb_get_pd_fb(common));
	}

	return ret;
}

EAPI int livebox_sync_pd_fb(struct livebox *handler)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	return lb_sync_pd_fb(handler->common);
}

EAPI const char *livebox_alt_icon(struct livebox *handler)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is not valid[%p]\n", handler);
		return NULL;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is not valid\n");
		return NULL;
	}

	return handler->common->alt.icon;
}

EAPI const char *livebox_alt_name(struct livebox *handler)
{
	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is not valid[%p]\n", handler);
		return NULL;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is not valid\n");
		return NULL;
	}

	return handler->common->alt.name;
}

EAPI int livebox_acquire_fb_lock(struct livebox *handler, int is_pd)
{
	int ret = LB_STATUS_SUCCESS;
	int fd;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Handler is not valid[%p]\n", handler);
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Handler is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid[%p]\n", handler);
		return LB_STATUS_ERROR_INVALID;
	}

	if (is_pd) {
		if (!handler->common->pd.lock || handler->common->pd.lock_fd < 0) {
			DbgPrint("Lock: %s (%d)\n", handler->common->pd.lock, handler->common->pd.lock_fd);
			return LB_STATUS_ERROR_INVALID;
		}

		if (fb_type(lb_get_pd_fb(handler->common)) == BUFFER_TYPE_FILE) {
			return LB_STATUS_SUCCESS;
		}

		fd = handler->common->pd.lock_fd;
	} else {
		if (!handler->common->lb.lock || handler->common->lb.lock_fd < 0) {
			DbgPrint("Lock: %s (%d)\n", handler->common->lb.lock, handler->common->lb.lock_fd);
			return LB_STATUS_ERROR_INVALID;
		}

		if (fb_type(lb_get_lb_fb(handler->common)) == BUFFER_TYPE_FILE) {
			return LB_STATUS_SUCCESS;
		}

		fd = handler->common->lb.lock_fd;
	}

	ret = do_fb_lock(fd);

	return ret == 0 ? LB_STATUS_SUCCESS : LB_STATUS_ERROR_FAULT;
}

EAPI int livebox_release_fb_lock(struct livebox *handler, int is_pd)
{
	int ret = LB_STATUS_SUCCESS;
	int fd;

	if (!handler || handler->state != CREATE) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common || handler->common->state != CREATE) {
		ErrPrint("Invalid handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (!handler->common->id) {
		ErrPrint("Handler is not valid[%p]\n", handler);
		return LB_STATUS_ERROR_INVALID;
	}

	if (is_pd) {
		if (!handler->common->pd.lock || handler->common->pd.lock_fd < 0) {
			DbgPrint("Unlock: %s (%d)\n", handler->common->pd.lock, handler->common->pd.lock_fd);
			return LB_STATUS_ERROR_INVALID;
		}

		if (fb_type(lb_get_pd_fb(handler->common)) == BUFFER_TYPE_FILE) {
			return LB_STATUS_SUCCESS;
		}

		fd = handler->common->pd.lock_fd;
	} else {
		if (!handler->common->lb.lock || handler->common->lb.lock_fd < 0) {
			DbgPrint("Unlock: %s (%d)\n", handler->common->lb.lock, handler->common->lb.lock_fd);
			return LB_STATUS_ERROR_INVALID;
		}

		if (fb_type(lb_get_lb_fb(handler->common)) == BUFFER_TYPE_FILE) {
			return LB_STATUS_SUCCESS;
		}

		fd = handler->common->lb.lock_fd;
	}

	ret = do_fb_unlock(fd);

	return ret == 0 ? LB_STATUS_SUCCESS : LB_STATUS_ERROR_FAULT;
}

EAPI int livebox_set_option(enum livebox_option_type option, int state)
{
	int ret = LB_STATUS_SUCCESS;

	switch (option) {
	case LB_OPTION_MANUAL_SYNC:
		conf_set_manual_sync(state);
		break;
	case LB_OPTION_FRAME_DROP_FOR_RESIZE:
		conf_set_frame_drop_for_resizing(state);
		break;
	case LB_OPTION_SHARED_CONTENT:
		conf_set_shared_content(state);
		break;
	default:
		ret = LB_STATUS_ERROR_INVALID;
		break;
	}

	return ret;
}

EAPI int livebox_option(enum livebox_option_type option)
{
	int ret;

	switch (option) {
	case LB_OPTION_MANUAL_SYNC:
		ret = conf_manual_sync();
		break;
	case LB_OPTION_FRAME_DROP_FOR_RESIZE:
		ret = conf_frame_drop_for_resizing();
		break;
	case LB_OPTION_SHARED_CONTENT:
		ret = conf_shared_content();
		break;
	default:
		ret = LB_STATUS_ERROR_INVALID;
		break;
	}

	return ret;
}

/* End of a file */
