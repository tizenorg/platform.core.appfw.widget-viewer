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
#include <dynamicbox_service.h>
#include <dynamicbox_errno.h>
#include <dynamicbox_cmd_list.h>
#include <dynamicbox_buffer.h>

#include "debug.h"
#include "fb.h"
#include "dynamicbox.h"
#include "dynamicbox_internal.h"
#include "dlist.h"
#include "util.h"
#include "master_rpc.h"
#include "client.h"
#include "conf.h"

#define EAPI __attribute__((visibility("default")))

#if defined(FLOG)
FILE *__file_log_fp;
#endif

static int default_launch_handler(dynamicbox_h handler, const char *appid, void *data);

static struct info {
    int init_count;
    int prevent_overwrite;
    guint job_timer;
    struct dlist *job_list;

    struct launch {
	int (*handler)(dynamicbox_h handler, const char *appid, void *data);
	void *data;
    } launch;
} s_info = {
    .init_count = 0,
    .prevent_overwrite = 0,
    .job_timer = 0,
    .job_list = NULL,
    .launch = {
	.handler = default_launch_handler,
	.data = NULL,
    },
};

static void dbox_pixmap_acquired_cb(dynamicbox_h handler, const struct packet *result, void *data);
static void gbar_pixmap_acquired_cb(dynamicbox_h handler, const struct packet *result, void *data);
static void gbar_xpixmap_acquired_cb(dynamicbox_h handler, const struct packet *result, void *data);
static void dbox_xpixmap_acquired_cb(dynamicbox_h handler, const struct packet *result, void *data);

static int default_launch_handler(dynamicbox_h handler, const char *appid, void *data)
{
    int ret;

    ret = aul_launch_app(appid, NULL);
    if (ret <= 0) {
	ErrPrint("Failed to launch an app %s (%d)\n", appid, ret);
    }

    /*
       app_control_h service;

       DbgPrint("AUTO_LAUNCH [%s]\n", handler->common->dbox.auto_launch);

       ret = app_control_create(&service);
       if (ret == APP_CONTROL_ERROR_NONE) {
       app_control_set_package(service, handler->common->dbox.auto_launch);
       app_control_send_launch_request(service, NULL, NULL);
       app_control_destroy(service);
       } else {
       ErrPrint("Failed to launch an app %s (%d)\n", handler->common->dbox.auto_launch, ret);
       }
     */

    return ret > 0 ? DBOX_STATUS_ERROR_NONE : DBOX_STATUS_ERROR_FAULT;
}

static inline void default_create_cb(dynamicbox_h handler, int ret, void *data)
{
    DbgPrint("Default created event handler: %d\n", ret);
}

static inline void default_pinup_cb(dynamicbox_h handler, int ret, void *data)
{
    DbgPrint("Default pinup event handler: %d\n", ret);
}

static inline void default_group_changed_cb(dynamicbox_h handler, int ret, void *data)
{
    DbgPrint("Default group changed event handler: %d\n", ret);
}

static inline void default_period_changed_cb(dynamicbox_h handler, int ret, void *data)
{
    DbgPrint("Default period changed event handler: %d\n", ret);
}

static inline void default_gbar_created_cb(dynamicbox_h handler, int ret, void *data)
{
    DbgPrint("Default GBAR created event handler: %d\n", ret);
}

static inline void default_gbar_destroyed_cb(dynamicbox_h handler, int ret, void *data)
{
    DbgPrint("Default GBAR destroyed event handler: %d\n", ret);
}

static inline void default_dbox_size_changed_cb(dynamicbox_h handler, int ret, void *data)
{
    DbgPrint("Default DBOX size changed event handler: %d\n", ret);
}

static inline void default_update_mode_cb(dynamicbox_h handler, int ret, void *data)
{
    DbgPrint("Default update mode set event handler: %d\n", ret);
}

static inline void default_access_event_cb(dynamicbox_h handler, int ret, void *data)
{
    DbgPrint("Default access event handler: %d\n", ret);
}

static inline void default_key_event_cb(dynamicbox_h handler, int ret, void *data)
{
    DbgPrint("Default key event handler: %d\n", ret);
}

static void update_mode_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    int ret;

    if (!result) {
	ret = DBOX_STATUS_ERROR_FAULT;
	goto errout;
    } else if (packet_get(result, "i", &ret) != 1) {
	ErrPrint("Invalid argument\n");
	ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
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
    handler->common->request.update_mode = 0;

    if (handler->common->state != DBOX_STATE_DELETE) {
	if (ret == (int)DBOX_STATUS_ERROR_NOT_EXIST && handler->refcnt == 2) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_DELETED);
	    dbox_unref(handler, 1);
	}
    }
}

static void resize_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    int ret;

    if (!result) {
	ret = DBOX_STATUS_ERROR_FAULT;
	goto errout;
    } else if (packet_get(result, "i", &ret) != 1) {
	ErrPrint("Invalid argument\n");
	ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
	goto errout;
    }

    /*!
     * \note
     * In case of resize request,
     * The dynamicbox handler will not have resized value right after this callback,
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
    handler->common->request.size_changed = 0;

    if (handler->common->state != DBOX_STATE_DELETE) {
	if (ret == (int)DBOX_STATUS_ERROR_NOT_EXIST && handler->refcnt == 2) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_DELETED);
	    dbox_unref(handler, 1);
	}
    }
}

static void text_signal_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    int ret;
    void *cbdata;
    struct cb_info *info = data;
    dynamicbox_ret_cb cb;

    cbdata = info->data;
    cb = info->cb;
    dbox_destroy_cb_info(info);

    if (!result) {
	ret = DBOX_STATUS_ERROR_FAULT;
    } else if (packet_get(result, "i", &ret) != 1) {
	ErrPrint("Invalid argument\n");
	ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (cb) {
	cb(handler, ret, cbdata);
    }
    return;
}

static void set_group_ret_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    int ret;

    if (!result) {
	ret = DBOX_STATUS_ERROR_FAULT;
	goto errout;
    } else if (packet_get(result, "i", &ret) != 1) {
	ErrPrint("Invalid argument\n");
	ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
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
    handler->common->request.group_changed = 0;

    if (handler->common->state != DBOX_STATE_DELETE) {
	if (ret == (int)DBOX_STATUS_ERROR_NOT_EXIST && handler->refcnt == 2) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_DELETED);
	    dbox_unref(handler, 1);
	}
    }
}

static void period_ret_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    int ret;

    if (!result) {
	ret = DBOX_STATUS_ERROR_FAULT;
	goto errout;
    } else if (packet_get(result, "i", &ret) != 1) {
	ErrPrint("Invalid argument\n");
	ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
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
    handler->common->request.period_changed = 0;

    if (handler->common->state != DBOX_STATE_DELETE) {
	if (ret == (int)DBOX_STATUS_ERROR_NOT_EXIST && handler->refcnt == 2) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_DELETED);
	    dbox_unref(handler, 1);
	}
    }
}

static void gbar_create_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    int ret;

    if (!result) {
	ret = DBOX_STATUS_ERROR_FAULT;
	goto errout;
    } else if (packet_get(result, "i", &ret) != 1) {
	ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
	goto errout;
    }

    if (ret < 0) {
	ErrPrint("Failed to create a GBAR[%d]\n", ret);
	goto errout;
    }

    return;

errout:
    handler->cbs.gbar_created.cb(handler, ret, handler->cbs.gbar_created.data);
    handler->cbs.gbar_created.cb = NULL;
    handler->cbs.gbar_created.data = NULL;
    handler->common->request.gbar_created = 0;

    if (handler->common->state != DBOX_STATE_DELETE) {
	if (ret == (int)DBOX_STATUS_ERROR_NOT_EXIST && handler->refcnt == 2) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_DELETED);
	    dbox_unref(handler, 1);
	}
    }
}

static void activated_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    int ret;
    struct cb_info *info = data;
    void *cbdata;
    dynamicbox_ret_cb cb;
    const char *pkgname = "";

    cbdata = info->data;
    cb = info->cb;
    dbox_destroy_cb_info(info);

    if (!result) {
	ret = DBOX_STATUS_ERROR_FAULT;
    } else if (packet_get(result, "is", &ret, &pkgname) != 2) {
	ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (cb) {
	cb(handler, ret, cbdata);
    }
}

static void gbar_destroy_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    int ret;
    dynamicbox_ret_cb cb;
    void *cbdata;
    struct cb_info *info = data;

    cbdata = info->data;
    cb = info->cb;
    dbox_destroy_cb_info(info);

    if (!result) {
	ErrPrint("Result is NIL (may connection lost)\n");
	ret = DBOX_STATUS_ERROR_FAULT;
    } else if (packet_get(result, "i", &ret) != 1) {
	ErrPrint("Invalid parameter\n");
	ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (ret == (int)DBOX_STATUS_ERROR_NONE) {
	handler->cbs.gbar_destroyed.cb = cb;
	handler->cbs.gbar_destroyed.data = cbdata;
    } else {
	handler->common->is_gbar_created = 0;
	handler->common->request.gbar_destroyed = 0;

	if (cb) {
	    cb(handler, ret, cbdata);
	}
    }
}

static void delete_cluster_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    struct cb_info *info = data;
    int ret;
    dynamicbox_ret_cb cb;
    void *cbdata;

    cb = info->cb;
    cbdata = info->data;
    dbox_destroy_cb_info(info);

    if (!result) {
	ret = DBOX_STATUS_ERROR_FAULT;
    } else if (packet_get(result, "i", &ret) != 1) {
	ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (cb) {
	cb(handler, ret, cbdata);
    }
}

static void delete_category_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    struct cb_info *info = data;
    int ret;
    dynamicbox_ret_cb cb;
    void *cbdata;

    cb = info->cb;
    cbdata = info->data;
    dbox_destroy_cb_info(info);

    if (!result) {
	ret = DBOX_STATUS_ERROR_FAULT;
    } else if (packet_get(result, "i", &ret) != 1) {
	ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (cb) {
	cb(handler, ret, cbdata);
    }
}

static int dbox_acquire_dbox_pixmap(dynamicbox_h handler, dynamicbox_ret_cb cb, void *data)
{
    struct packet *packet;
    struct cb_info *cbinfo;
    const char *id;
    unsigned int cmd = CMD_DBOX_ACQUIRE_PIXMAP;
    int ret;

    id = fb_id(handler->common->dbox.fb);
    if (!id || strncasecmp(id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP))) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    packet = packet_create((const char *)&cmd, "ss", handler->common->pkgname, handler->common->id);
    if (!packet) {
	ErrPrint("Failed to build a param\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    cbinfo = dbox_create_cb_info(cb, data);
    if (!cbinfo) {
	packet_destroy(packet);
	return DBOX_STATUS_ERROR_FAULT;
    }

    ret = master_rpc_async_request(handler, packet, 0, dbox_pixmap_acquired_cb, cbinfo);
    if (ret < 0) {
	dbox_destroy_cb_info(cbinfo);
    }

    return ret;
}

static void dbox_pixmap_acquired_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    int pixmap;
    int ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
    dynamicbox_ret_cb cb;
    void *cbdata;
    struct cb_info *info = data;

    cb = info->cb;
    cbdata = info->data;
    dbox_destroy_cb_info(info);

    if (!result) {
	pixmap = 0; /* PIXMAP 0 means error */
    } else if (packet_get(result, "ii", &pixmap, &ret) != 2) {
	pixmap = 0;
    }

    if (ret == (int)DBOX_STATUS_ERROR_BUSY) {
	ret = dbox_acquire_dbox_pixmap(handler, cb, cbdata);
	DbgPrint("Busy, Try again: %d\n", ret);
	/* Try again */
    } else if (ret == (int)DBOX_STATUS_ERROR_NOT_EXIST && handler->refcnt == 2) {
	if (cb) {
	    cb(handler, pixmap, cbdata);
	}

	if (handler->common->state != DBOX_STATE_DELETE) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_DELETED);
	    dbox_unref(handler, 1);
	}
    } else {
	if (cb) {
	    cb(handler, pixmap, cbdata);
	}
    }
}

static void dbox_xpixmap_acquired_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    int pixmap;
    int ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
    dynamicbox_ret_cb cb;
    void *cbdata;
    struct cb_info *info = data;

    cb = info->cb;
    cbdata = info->data;
    dbox_destroy_cb_info(info);

    if (!result) {
	pixmap = 0;
    } else if (packet_get(result, "ii", &pixmap, &ret) != 2) {
	pixmap = 0;
    }

    if (cb) {
	cb(handler, pixmap, cbdata);
    }

    if (handler->common->state != DBOX_STATE_DELETE) {
	if (ret == (int)DBOX_STATUS_ERROR_NOT_EXIST && handler->refcnt == 2) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_DELETED);
	    dbox_unref(handler, 1);
	}
    }
}

static int dbox_acquire_gbar_extra_pixmap(dynamicbox_h handler, int idx, dynamicbox_ret_cb cb, void *data)
{
    struct packet *packet;
    struct cb_info *cbinfo;
    unsigned int cmd = CMD_GBAR_ACQUIRE_XPIXMAP;
    int ret;

    packet = packet_create((const char *)&cmd, "ssi", handler->common->pkgname, handler->common->id, idx);
    if (!packet) {
	ErrPrint("Failed to build a param\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    cbinfo = dbox_create_cb_info(cb, data);
    if (!cbinfo) {
	packet_destroy(packet);
	return DBOX_STATUS_ERROR_FAULT;
    }

    ret = master_rpc_async_request(handler, packet, 0, gbar_xpixmap_acquired_cb, cbinfo);
    if (ret < 0) {
	/*!
	 * \note
	 * Packet will be destroyed by master_rpc_async_request
	 */
	dbox_destroy_cb_info(cbinfo);
    }

    return ret;
}

static int dbox_acquire_dbox_extra_pixmap(dynamicbox_h handler, int idx, dynamicbox_ret_cb cb, void *data)
{
    struct packet *packet;
    struct cb_info *cbinfo;
    unsigned int cmd = CMD_DBOX_ACQUIRE_XPIXMAP;
    int ret;

    packet = packet_create((const char *)&cmd, "ssi", handler->common->pkgname, handler->common->id, idx);
    if (!packet) {
	ErrPrint("Failed to build a param\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    cbinfo = dbox_create_cb_info(cb, data);
    if (!cbinfo) {
	packet_destroy(packet);
	return DBOX_STATUS_ERROR_FAULT;
    }

    ret = master_rpc_async_request(handler, packet, 0, dbox_xpixmap_acquired_cb, cbinfo);
    if (ret < 0) {
	/*!
	 * \note
	 * Packet will be destroyed by master_rpc_async_request
	 */
	dbox_destroy_cb_info(cbinfo);
    }

    return ret;
}

static int dbox_acquire_gbar_pixmap(dynamicbox_h handler, dynamicbox_ret_cb cb, void *data)
{
    struct packet *packet;
    struct cb_info *cbinfo;
    unsigned int cmd = CMD_GBAR_ACQUIRE_PIXMAP;
    const char *id;
    int ret;

    id = fb_id(handler->common->gbar.fb);
    if (!id || strncasecmp(id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP))) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    packet = packet_create((const char *)&cmd, "ss", handler->common->pkgname, handler->common->id);
    if (!packet) {
	ErrPrint("Failed to build a param\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    cbinfo = dbox_create_cb_info(cb, data);
    if (!cbinfo) {
	packet_destroy(packet);
	return DBOX_STATUS_ERROR_FAULT;
    }

    ret = master_rpc_async_request(handler, packet, 0, gbar_pixmap_acquired_cb, cbinfo);
    if (ret < 0) {
	/*!
	 * \note
	 * Packet will be destroyed by master_rpc_async_request
	 */
	dbox_destroy_cb_info(cbinfo);
    }

    return ret;
}

static void gbar_xpixmap_acquired_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    int pixmap;
    int ret;
    dynamicbox_ret_cb cb;
    void *cbdata;
    struct cb_info *info = data;

    cb = info->cb;
    cbdata = info->data;
    dbox_destroy_cb_info(info);

    if (!result) {
	pixmap = 0; /* PIXMAP 0 means error */
	ret = DBOX_STATUS_ERROR_FAULT;
    } else if (packet_get(result, "ii", &pixmap, &ret) != 2) {
	pixmap = 0;
	ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (cb) {
	DbgPrint("ret: %x, pixmap: %d\n", ret, pixmap);
	cb(handler, pixmap, cbdata);
    }

    if (handler->common->state != DBOX_STATE_DELETE) {
	if (ret == (int)DBOX_STATUS_ERROR_NOT_EXIST && handler->refcnt == 2) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_DELETED);
	    dbox_unref(handler, 1);
	}
    }
}

static void gbar_pixmap_acquired_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    int pixmap;
    int ret;
    dynamicbox_ret_cb cb;
    void *cbdata;
    struct cb_info *info = data;

    cb = info->cb;
    cbdata = info->data;
    dbox_destroy_cb_info(info);

    if (!result) {
	pixmap = 0; /* PIXMAP 0 means error */
	ret = DBOX_STATUS_ERROR_FAULT;
    } else if (packet_get(result, "ii", &pixmap, &ret) != 2) {
	pixmap = 0;
	ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (ret == (int)DBOX_STATUS_ERROR_BUSY) {
	ret = dbox_acquire_gbar_pixmap(handler, cb, cbdata);
	DbgPrint("Busy, Try again: %d\n", ret);
	/* Try again */
    } else if (ret == (int)DBOX_STATUS_ERROR_NOT_EXIST && handler->refcnt == 2) {
	if (cb) {
	    cb(handler, pixmap, cbdata);
	}

	if (handler->common->state != DBOX_STATE_DELETE) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_DELETED);
	    dbox_unref(handler, 1);
	}
    } else {
	if (cb) {
	    DbgPrint("ret: %d, pixmap: %d\n", ret, pixmap);
	    cb(handler, pixmap, cbdata);
	}
    }
}

static void pinup_done_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    int ret;

    if (!result) {
	ret = DBOX_STATUS_ERROR_FAULT;
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
    handler->common->request.pinup = 0;

    if (handler->common->state != DBOX_STATE_DELETE) {
	if (ret == (int)DBOX_STATUS_ERROR_NOT_EXIST && handler->refcnt == 2) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_DELETED);
	    dbox_unref(handler, 1);
	}
    }
}

static void key_ret_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    int ret;

    if (!result) {
	ret = DBOX_STATUS_ERROR_FAULT;
	return;
    }

    if (packet_get(result, "i", &ret) != 1) {
	ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
	return;
    }

    if (ret != DBOX_STATUS_ERROR_NONE) {
	goto errout;
    }

    return;
errout:
    handler->cbs.key_event.cb(handler, ret, handler->cbs.key_event.data);
    handler->cbs.key_event.cb = NULL;
    handler->cbs.key_event.data = NULL;
    handler->common->request.key_event = 0;

    if (handler->common->state != DBOX_STATE_DELETE) {
	if (ret == (int)DBOX_STATUS_ERROR_NOT_EXIST && handler->refcnt == 2) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_DELETED);
	    dbox_unref(handler, 1);
	}
    }
}

static void access_ret_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    int ret;

    if (!result) {
	ret = DBOX_STATUS_ERROR_FAULT;
	return;
    }

    if (packet_get(result, "i", &ret) != 1) {
	ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
	return;
    }

    if (ret != DBOX_STATUS_ERROR_NONE) {
	goto errout;
    }

    return;

errout:
    handler->cbs.access_event.cb(handler, ret, handler->cbs.access_event.data);
    handler->cbs.access_event.cb = NULL;
    handler->cbs.access_event.data = NULL;
    handler->common->request.access_event = 0;

    if (handler->common->state != DBOX_STATE_DELETE) {
	if (ret == (int)DBOX_STATUS_ERROR_NOT_EXIST && handler->refcnt == 2) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_DELETED);
	    dbox_unref(handler, 1);
	}
    }
}

static int send_access_event(dynamicbox_h handler, const char *event, int x, int y, int type)
{
    struct packet *packet;
    double timestamp;

    timestamp = util_timestamp();

    packet = packet_create(event, "ssdiii", handler->common->pkgname, handler->common->id, timestamp, x, y, type);
    if (!packet) {
	ErrPrint("Failed to build packet\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    return master_rpc_async_request(handler, packet, 0, access_ret_cb, NULL);
}

static int send_key_event(dynamicbox_h handler, const char *event, unsigned int keycode)
{
    struct packet *packet;
    double timestamp;

    timestamp = util_timestamp();
    packet = packet_create(event, "ssdi", handler->common->pkgname, handler->common->id, timestamp, keycode);
    if (!packet) {
	ErrPrint("Failed to build packet\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    return master_rpc_async_request(handler, packet, 0, key_ret_cb, NULL);
}

static int send_mouse_event(dynamicbox_h handler, const char *event, int x, int y)
{
    struct packet *packet;
    double timestamp;

    timestamp = util_timestamp();
    packet = packet_create_noack(event, "ssdii", handler->common->pkgname, handler->common->id, timestamp, x, y);
    if (!packet) {
	ErrPrint("Failed to build param\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    return master_rpc_request_only(handler, packet);
}

static int initialize_dynamicbox(void *disp, int use_thread)
{
    int ret;
#if defined(FLOG)
    char filename[BUFSIZ];
    snprintf(filename, sizeof(filename), "/tmp/%d.box.log", getpid());
    __file_log_fp = fopen(filename, "w+t");
    if (!__file_log_fp) {
	__file_log_fp = fdopen(1, "w+t");
    }
#endif
    ret = dynamicbox_service_init();
    if (ret != DBOX_STATUS_ERROR_NONE) {
	return ret;
    }

    ret = fb_init(disp);
    if (ret != DBOX_STATUS_ERROR_NONE) {
	dynamicbox_service_fini();
	return ret;
    }

    ret = client_init(use_thread);
    if (ret != DBOX_STATUS_ERROR_NONE) {
	fb_fini();
	dynamicbox_service_fini();
	return ret;
    }

    s_info.init_count++;
    return ret;
}

static inline char *dbox_pkgname(const char *pkgname)
{
    char *dbox;

    dbox = dynamicbox_service_dbox_id(pkgname);
    if (!dbox) {
	dbox = strdup(pkgname);
    }

    return dbox;
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
	dbox_unref(item->handle, 1);
	free(item);
    }

    return TRUE;
}

static int job_add(dynamicbox_h handle, dynamicbox_ret_cb job_cb, int ret, void *data)
{
    struct job_item *item;

    if (!job_cb) {
	ErrPrint("Invalid argument\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    item = malloc(sizeof(*item));
    if (!item) {
	ErrPrint("Heap: %s\n", strerror(errno));
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    item->handle = dbox_ref(handle);
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

    return DBOX_STATUS_ERROR_NONE;
}

static void new_ret_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    int ret;
    struct cb_info *info = data;
    dynamicbox_ret_cb cb;
    void *cbdata;

    cb = info->cb;
    cbdata = info->data;
    dbox_destroy_cb_info(info);

    if (!result) {
	ret = DBOX_STATUS_ERROR_FAULT;
    } else if (packet_get(result, "i", &ret) != 1) {
	ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
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

    dbox_unref(handler, 1);
}

static int create_real_instance(dynamicbox_h handler, dynamicbox_ret_cb cb, void *data)
{
    struct cb_info *cbinfo;
    struct packet *packet;
    struct dynamicbox_common *common;
    unsigned int cmd = CMD_NEW;
    int ret;

    common = handler->common;

    packet = packet_create((const char *)&cmd, "dssssdii",
	    common->timestamp, common->pkgname, common->content,
	    common->cluster, common->category,
	    common->dbox.period, common->dbox.width, common->dbox.height);
    if (!packet) {
	ErrPrint("Failed to create a new packet\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_FAULT);
	return DBOX_STATUS_ERROR_FAULT;
    }

    cbinfo = dbox_create_cb_info(cb, data);
    if (!cbinfo) {
	ErrPrint("Failed to create a cbinfo\n");
	packet_destroy(packet);
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_OUT_OF_MEMORY);
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    /*!
     * \note
     * master_rpc_async_request will destroy the packet (decrease the refcnt)
     * So be aware the packet object after return from master_rpc_async_request.
     */
    ret = master_rpc_async_request(handler, packet, 0, new_ret_cb, cbinfo);
    if (ret < 0) {
	ErrPrint("Failed to send a new packet\n");
	dbox_destroy_cb_info(cbinfo);
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_FAULT);
	return DBOX_STATUS_ERROR_FAULT;
    }
    handler->common->request.created = 1;
    return DBOX_STATUS_ERROR_NONE;
}

static void create_cb(dynamicbox_h handle, int ret, void *data)
{
    struct cb_info *cbinfo = data;

    if (cbinfo->cb) {
	cbinfo->cb(handle, ret, cbinfo->data);
    }

    dbox_destroy_cb_info(cbinfo);

    /*!
     * \note
     * Forcely generate "updated" event
     */
    dbox_invoke_event_handler(handle, DBOX_EVENT_DBOX_UPDATED);
}

static int create_fake_instance(dynamicbox_h handler, dynamicbox_ret_cb cb, void *data)
{
    struct cb_info *cbinfo;

    cbinfo = dbox_create_cb_info(cb, data);
    if (!cbinfo) {
	ErrPrint("Failed to create a cbinfo\n");
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    if (job_add(handler, create_cb, DBOX_STATUS_ERROR_NONE, cbinfo) != DBOX_STATUS_ERROR_NONE) {
	dbox_destroy_cb_info(cbinfo);
    }

    return DBOX_STATUS_ERROR_NONE;
}

static void refresh_for_paused_updating_cb(dynamicbox_h handle, int ret, void *data)
{
    if (handle->paused_updating == 0) {
	DbgPrint("Paused updates are cleared\n");
	return;
    }

    DbgPrint("Pending updates are found\n");
    dbox_invoke_event_handler(handle, DBOX_EVENT_DBOX_UPDATED);
}

static int dbox_set_visibility(dynamicbox_h handler, dynamicbox_visible_state_e state)
{
    struct packet *packet;
    int need_to_add_job = 0;
    unsigned int cmd = CMD_CHANGE_VISIBILITY;
    int ret;

    if (handler->common->visible != DBOX_SHOW && state == DBOX_SHOW) {
	need_to_add_job = !!handler->paused_updating;
    } else if (handler->common->visible == DBOX_SHOW && state != DBOX_SHOW) {
	if (!!dbox_find_dbox_in_show(handler->common)) {
	    return DBOX_STATUS_ERROR_NONE;
	}
    } else if (handler->common->visible == DBOX_SHOW && state == DBOX_SHOW && handler->paused_updating) {
	if (job_add(handler, refresh_for_paused_updating_cb, DBOX_STATUS_ERROR_NONE, NULL) < 0) {
	    ErrPrint("Unable to add a new job for refreshing box\n");
	}

	return DBOX_STATUS_ERROR_NONE;
    } else {
	/*!
	 * \brief
	 * No need to send this to the master
	 */
	return DBOX_STATUS_ERROR_NONE;
    }

    packet = packet_create_noack((const char *)&cmd, "ssi", handler->common->pkgname, handler->common->id, (int)state);
    if (!packet) {
	ErrPrint("Failed to create a packet\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    ret = master_rpc_request_only(handler, packet);
    if (ret == (int)DBOX_STATUS_ERROR_NONE) {
	DbgPrint("[%s] visibility is changed 0x[%x]\n", handler->common->pkgname, state);
	handler->common->visible = state;

	if (need_to_add_job) {
	    if (job_add(handler, refresh_for_paused_updating_cb, DBOX_STATUS_ERROR_NONE, NULL) < 0) {
		ErrPrint("Unable to add a new job for refreshing box\n");
	    }
	}
    }

    return ret;
}

static void dbox_update_visibility(struct dynamicbox_common *old_common)
{
    dynamicbox_h item;

    item = dbox_find_dbox_in_show(old_common);
    if (!item) {
	item = dbox_get_dbox_nth(old_common, 0);
	if (item) {
	    dbox_set_visibility(item, DBOX_HIDE_WITH_PAUSE);
	} else {
	    ErrPrint("Unable to get the valid handle from common handler\n");
	}
    } else {
	dbox_set_visibility(item, DBOX_SHOW);
    }
}

/*!
 * \note
 * The second parameter should be the "return value",
 * But in this case, we will use it for "type of deleting instance".
 */
static void job_del_cb(dynamicbox_h handle, int type, void *data)
{
    struct cb_info *cbinfo = data;
    dynamicbox_ret_cb cb;

    if (handle->visible == DBOX_SHOW) {
	dbox_update_visibility(handle->common);
    }

    cb = cbinfo->cb;
    data = cbinfo->data;
    dbox_destroy_cb_info(cbinfo);

    if (handle->common->state != DBOX_STATE_CREATE) {
	DbgPrint("[%s] %d\n", handle->common->pkgname, handle->refcnt);
	if (cb) {
	    cb(handle, DBOX_STATUS_ERROR_NONE, data);
	}

	return;
    }

    if (handle->common->refcnt == 1) {
	handle->common->delete_type = type;
	handle->common->state = DBOX_STATE_DELETE;

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
		cb(handle, DBOX_STATUS_ERROR_NONE, data);
	    }
	}

	DbgPrint("Send delete request\n");
	dbox_send_delete(handle, type, cb, data);
    } else {
	if (cb) {
	    cb(handle, DBOX_STATUS_ERROR_NONE, data);
	}

	DbgPrint("Before unref: %d\n", handle->common->refcnt);
	dbox_unref(handle, 1);
    }
}

static void resize_job_cb(dynamicbox_h handler, int ret, void *data)
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
    dbox_invoke_event_handler(handler, DBOX_EVENT_DBOX_UPDATED);
}

static void turn_off_gbar_destroyed_flag_cb(dynamicbox_h handler, int ret, void *data)
{
    if (handler->common->request.gbar_destroyed) {
	dynamicbox_ret_cb cb;
	void *data;

	DbgPrint("gbar_destroyed request is canceled\n");
	handler->common->request.gbar_destroyed = 0;
	cb = handler->cbs.gbar_destroyed.cb;
	data = handler->cbs.gbar_destroyed.data;
	handler->cbs.gbar_destroyed.cb = NULL;
	handler->cbs.gbar_destroyed.data = NULL;

	if (cb) {
	    cb(handler, ret, data);
	}
    }
}

static void turn_off_gbar_created_flag_cb(dynamicbox_h handler, int ret, void *data)
{
    if (handler->common->request.gbar_created) {
	dynamicbox_ret_cb cb;
	void *data;

	DbgPrint("gbar_created request is canceled\n");
	handler->common->request.gbar_created = 0;
	cb = handler->cbs.gbar_created.cb;
	data = handler->cbs.gbar_created.data;
	handler->cbs.gbar_created.cb = NULL;
	handler->cbs.gbar_created.data = NULL;

	if (cb) {
	    cb(handler, ret, data);
	}
    }
}

EAPI int dynamicbox_init(void *disp, int prevent_overwrite, double event_filter, int use_thread)
{
    if (s_info.init_count > 0) {
	s_info.init_count++;
	return DBOX_STATUS_ERROR_NONE;
    }

    /*!
     * \note
     * Some application doesn't want to use the environment value.
     * So set them using arguments.
     */
    s_info.prevent_overwrite = prevent_overwrite;
    conf_set_event_filter(event_filter);

    return initialize_dynamicbox(disp, use_thread);
}

EAPI int dynamicbox_fini(void)
{
    if (s_info.init_count <= 0) {
	ErrPrint("Doesn't initialized\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    s_info.init_count--;
    if (s_info.init_count > 0) {
	ErrPrint("init count : %d\n", s_info.init_count);
	return DBOX_STATUS_ERROR_NONE;
    }

    client_fini();
    fb_fini();
    dynamicbox_service_fini();
    return DBOX_STATUS_ERROR_NONE;
}

EAPI dynamicbox_h dynamicbox_add(const char *pkgname, const char *content, const char *cluster, const char *category, double period, dynamicbox_size_type_e type, dynamicbox_ret_cb cb, void *data)
{
    char *dboxid;
    dynamicbox_h handler;
    int w = 0;
    int h = 0;

    if (!pkgname || !cluster || !category) {
	ErrPrint("Invalid arguments: pkgname[%p], cluster[%p], category[%p]\n",
		pkgname, cluster, category);
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return NULL;
    }

    dboxid = dbox_pkgname(pkgname);
    if (!dboxid) {
	ErrPrint("Invalid package: %s\n", pkgname);
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return NULL;
    }

    if (dynamicbox_service_is_enabled(dboxid) == 0) {
	DbgPrint("Livebox [%s](%s) is disabled package\n", dboxid, pkgname);
	free(dboxid);
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_DISABLED);
	return NULL;
    }

    if (type != DBOX_SIZE_TYPE_UNKNOWN) {
	(void)dynamicbox_service_get_size(type, &w, &h);
    }

    handler = calloc(1, sizeof(*handler));
    if (!handler) {
	ErrPrint("Error: %s\n", strerror(errno));
	free(dboxid);
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_OUT_OF_MEMORY);
	return NULL;
    }

    if (!cb) {
	cb = default_create_cb;
    }

    handler->common = dbox_find_sharable_common_handle(dboxid, content, w, h, cluster, category);
    if (!handler->common) {
	handler->common = dbox_create_common_handle(handler, dboxid, cluster, category);
	free(dboxid);
	if (!handler->common) {
	    ErrPrint("Failed to find common handle\n");
	    free(handler);
	    return NULL;
	}

	if (!content || !strlen(content)) {
	    char *pc;
	    /**
	     * @note
	     * I know the content should not be modified. use it temporarly without "const"
	     */
	    pc = dynamicbox_service_content(handler->common->pkgname);
	    dbox_set_content(handler->common, pc);
	    free(pc);
	} else {
	    dbox_set_content(handler->common, content);
	}

	dbox_set_period(handler->common, period);
	dbox_set_size(handler->common, w, h);
	dbox_common_ref(handler->common, handler);

	if (create_real_instance(handler, cb, data) < 0) {
	    if (dbox_common_unref(handler->common, handler) == 0) {
		/*!
		 * Delete common
		 */
		dbox_destroy_common_handle(handler->common);
		handler->common = NULL;
	    }
	    free(handler);
	    return NULL;
	}
    } else {
	free(dboxid);

	dbox_common_ref(handler->common, handler);

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
		if (dbox_common_unref(handler->common, handler) == 0) {
		    /*!
		     * Delete common
		     */
		    dbox_destroy_common_handle(handler->common);
		}
		free(handler);
		return NULL;
	    }
	}
    }

    handler->visible = DBOX_SHOW;
    handler->state = DBOX_STATE_CREATE;
    handler = dbox_ref(handler);

    if (handler->common->visible != DBOX_SHOW) {
	dbox_set_visibility(handler, DBOX_SHOW);
    }

    return handler;
}

EAPI double dynamicbox_period(dynamicbox_h handler)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is not valid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return -1.0f;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Invalid handle\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return -1.0f;
    }

    if (!handler->common->id) {
	ErrPrint("Hnalder is not valid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return -1.0f;
    }

    return handler->common->dbox.period;
}

EAPI int dynamicbox_set_period(dynamicbox_h handler, double period, dynamicbox_ret_cb cb, void *data)
{
    struct packet *packet;
    unsigned int cmd = CMD_SET_PERIOD;
    int ret;

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Invalid handle\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (handler->common->request.period_changed) {
	ErrPrint("Previous request for changing period is not finished\n");
	return DBOX_STATUS_ERROR_BUSY;
    }

    if (!handler->common->is_user) {
	ErrPrint("CA Livebox is not able to change the period\n");
	return DBOX_STATUS_ERROR_PERMISSION_DENIED;
    }

    if (handler->common->dbox.period == period) {
	DbgPrint("No changes\n");
	return DBOX_STATUS_ERROR_ALREADY;
    }

    packet = packet_create((const char *)&cmd, "ssd", handler->common->pkgname, handler->common->id, period);
    if (!packet) {
	ErrPrint("Failed to build a packet %s\n", handler->common->pkgname);
	return DBOX_STATUS_ERROR_FAULT;
    }

    if (!cb) {
	cb = default_period_changed_cb;
    }

    ret = master_rpc_async_request(handler, packet, 0, period_ret_cb, NULL);
    if (ret == (int)DBOX_STATUS_ERROR_NONE) {
	handler->cbs.period_changed.cb = cb;
	handler->cbs.period_changed.data = data;
	handler->common->request.period_changed = 1;
    }

    return ret;
}

EAPI int dynamicbox_del(dynamicbox_h handler, dynamicbox_delete_type_e type, dynamicbox_ret_cb cb, void *data)
{
    struct cb_info *cbinfo;

    if (!handler) {
	ErrPrint("Handler is NIL\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is already deleted\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    handler->state = DBOX_STATE_DELETE;

    cbinfo = dbox_create_cb_info(cb, data);
    if (!cbinfo) {
	ErrPrint("Failed to create a cbinfo\n");
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    if (job_add(handler, job_del_cb, type, cbinfo) != DBOX_STATUS_ERROR_NONE) {
	ErrPrint("Failed to add a new job\n");
	dbox_destroy_cb_info(cbinfo);
	return DBOX_STATUS_ERROR_FAULT;
    }

    return DBOX_STATUS_ERROR_NONE;
}

EAPI int dynamicbox_add_fault_handler(dynamicbox_fault_handler_cb dbox_cb, void *data)
{
    if (!dbox_cb) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    return dbox_add_fault_handler(dbox_cb, data);
}

EAPI void *dynamicbox_remove_fault_handler(dynamicbox_fault_handler_cb dbox_cb)
{
    if (!dbox_cb) {
	return NULL;
    }

    return dbox_remove_fault_handler(dbox_cb);
}

EAPI int dynamicbox_add_event_handler(dynamicbox_event_handler_cb dbox_cb, void *data)
{
    if (!dbox_cb) {
	ErrPrint("Invalid argument dbox_cb is nil\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    return dbox_add_event_handler(dbox_cb, data);
}

EAPI void *dynamicbox_remove_event_handler(dynamicbox_event_handler_cb dbox_cb)
{
    if (!dbox_cb) {
	return NULL;
    }

    return dbox_remove_event_handler(dbox_cb);
}

EAPI int dynamicbox_set_update_mode(dynamicbox_h handler, int active_update, dynamicbox_ret_cb cb, void *data)
{
    struct packet *packet;
    unsigned int cmd = CMD_UPDATE_MODE;
    int ret;

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is Invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is Invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is Invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (handler->common->request.update_mode) {
	ErrPrint("Previous update_mode cb is not finished yet\n");
	return DBOX_STATUS_ERROR_BUSY;
    }

    if (handler->common->is_active_update == active_update) {
	return DBOX_STATUS_ERROR_ALREADY;
    }

    if (!handler->common->is_user) {
	return DBOX_STATUS_ERROR_PERMISSION_DENIED;
    }

    packet = packet_create((const char *)&cmd, "ssi", handler->common->pkgname, handler->common->id, active_update);
    if (!packet) {
	return DBOX_STATUS_ERROR_FAULT;
    }

    if (!cb) {
	cb = default_update_mode_cb;
    }

    ret = master_rpc_async_request(handler, packet, 0, update_mode_cb, NULL);
    if (ret == (int)DBOX_STATUS_ERROR_NONE) {
	handler->cbs.update_mode.cb = cb;
	handler->cbs.update_mode.data = data;
	handler->common->request.update_mode = 1;
    }

    return ret;
}

EAPI int dynamicbox_is_active_update(dynamicbox_h handler)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is Invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is Invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    return handler->common->is_active_update;
}

EAPI int dynamicbox_resize(dynamicbox_h handler, dynamicbox_size_type_e type, dynamicbox_ret_cb cb, void *data)
{
    struct dynamicbox_common *common;
    int w;
    int h;
    int ret;

    /*!
     * \TODO
     * If this handle is host instance or link instance,
     * Create a new instance or find another linkable instance.
     */

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Invalid handle\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    /*!
     * \note
     * resize operation should be separated by each handler.
     * If a handler is resizing, the other handler can request resize too.
     * So we should not use the common->request.size_changed flag.
     */
    if (handler->cbs.size_changed.cb) {
	ErrPrint("Previous resize request is not finished yet\n");
	return DBOX_STATUS_ERROR_BUSY;
    }

    if (dynamicbox_service_get_size(type, &w, &h) != 0) {
	ErrPrint("Invalid size type\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (handler->common->dbox.width == w && handler->common->dbox.height == h) {
	DbgPrint("No changes\n");
	return DBOX_STATUS_ERROR_ALREADY;
    }

    if (!handler->common->is_user) {
	ErrPrint("CA Livebox is not able to be resized\n");
	return DBOX_STATUS_ERROR_PERMISSION_DENIED;
    }

    if (handler->common->refcnt <= 1) {
	struct packet *packet;
	unsigned int cmd = CMD_RESIZE;

	/* Only 1 instance */
	packet = packet_create((const char *)&cmd, "ssii", handler->common->pkgname, handler->common->id, w, h);
	if (!packet) {
	    ErrPrint("Failed to build param\n");
	    return DBOX_STATUS_ERROR_FAULT;
	}

	if (!cb) {
	    cb = default_dbox_size_changed_cb;
	}

	ret = master_rpc_async_request(handler, packet, 0, resize_cb, NULL);
	if (ret == (int)DBOX_STATUS_ERROR_NONE) {
	    handler->cbs.size_changed.cb = cb;
	    handler->cbs.size_changed.data = data;
	    handler->common->request.size_changed = 1;
	}
    } else {
	common = dbox_find_sharable_common_handle(handler->common->pkgname, handler->common->content, w, h, handler->common->cluster, handler->common->category);
	if (!common) {
	    struct dynamicbox_common *old_common;
	    /*!
	     * \note
	     * If the common handler is in resizing,
	     * if user tries to resize a hander, then simply create new one even if the requested size is same with this.

	     if (handler->common->request.size_changed) {
	     }

	     */

	    old_common = handler->common;

	    common = dbox_create_common_handle(handler, old_common->pkgname, old_common->cluster, old_common->category);
	    if (!common) {
		ErrPrint("Failed to create common handle\n");
		return DBOX_STATUS_ERROR_FAULT;
	    }

	    dbox_set_size(common, w, h);
	    dbox_set_content(common, old_common->content);
	    dbox_set_period(common, old_common->dbox.period);

	    /*!
	     * \note
	     * Disconnecting from old one.
	     */
	    if (dbox_common_unref(old_common, handler) == 0) {
		/*!
		 * \note
		 * Impossible
		 */
		ErrPrint("Common has no associated handler\n");
	    }

	    dbox_common_ref(common, handler);

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
		dbox_common_unref(common, handler);
		dbox_destroy_common_handle(common);

		dbox_common_ref(old_common, handler);
		handler->common = old_common;
	    } else {
		/*!
		 * In this case, we should update visibility of old_common's dynamicboxes
		 */
		if (handler->visible == DBOX_SHOW) {
		    dbox_update_visibility(old_common);
		}
	    }
	} else {
	    struct cb_info *cbinfo;

	    cbinfo = dbox_create_cb_info(cb, data);
	    if (!cbinfo) {
		ErrPrint("Failed to create a cbinfo\n");
		ret = DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	    } else {
		ret = job_add(handler, resize_job_cb, DBOX_STATUS_ERROR_NONE, cbinfo);
		if (ret == (int)DBOX_STATUS_ERROR_NONE) {
		    struct dynamicbox_common *old_common;

		    old_common = handler->common;

		    if (dbox_common_unref(handler->common, handler) == 0) {
			ErrPrint("Old common has no associated handler\n");
		    }

		    dbox_common_ref(common, handler);
		    handler->common = common;

		    if (handler->visible == DBOX_SHOW) {
			dbox_update_visibility(old_common); /* To update visibility: Show --> Paused */
			dbox_update_visibility(common);    /* To update visibility: Paused --> Show */
		    }
		} else {
		    dbox_destroy_cb_info(cbinfo);
		}
	    }
	}
    }

    return ret;
}

EAPI int dynamicbox_click(dynamicbox_h handler, double x, double y)
{
    struct packet *packet;
    double timestamp;
    unsigned int cmd = CMD_CLICKED;
    int ret;

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (handler->common->dbox.auto_launch) {
	if (s_info.launch.handler) {
	    ret = s_info.launch.handler(handler, handler->common->dbox.auto_launch, s_info.launch.data);
	    if (ret < 0) {
		ErrPrint("launch handler app %s (%d)\n", handler->common->dbox.auto_launch, ret);
	    }
	}
    }

    timestamp = util_timestamp();
    DbgPrint("CLICKED: %lf\n", timestamp);

    packet = packet_create_noack((const char *)&cmd, "sssddd", handler->common->pkgname, handler->common->id, "clicked", timestamp, x, y);
    if (!packet) {
	ErrPrint("Failed to build param\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    ret = master_rpc_request_only(handler, packet);
    return ret;
}

EAPI int dynamicbox_has_glance_bar(dynamicbox_h handler)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    return !!handler->common->gbar.fb;
}

EAPI int dynamicbox_glance_bar_is_created(dynamicbox_h handler)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->gbar.fb || !handler->common->id) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    return handler->common->is_gbar_created;
}

EAPI int dynamicbox_create_glance_bar(dynamicbox_h handler, double x, double y, dynamicbox_ret_cb cb, void *data)
{
    struct packet *packet;
    unsigned int cmd = CMD_CREATE_GBAR;
    int ret;

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->gbar.fb || !handler->common->id) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    /*!
     * \note
     * Only one handler can have a GBAR
     */
    if (handler->common->is_gbar_created) {
	DbgPrint("GBAR is already created\n");
	return DBOX_STATUS_ERROR_NONE;
    }

    if (handler->common->request.gbar_created) {
	ErrPrint("Previous request is not completed yet\n");
	return DBOX_STATUS_ERROR_BUSY;
    }

    /*!
     * \note
     * Turn off the gbar_destroyed request flag
     */
    if (handler->common->request.gbar_destroyed) {
	if (job_add(handler, turn_off_gbar_destroyed_flag_cb, DBOX_STATUS_ERROR_CANCEL, NULL) < 0) {
	    ErrPrint("Failed to add gbar_destroyed job\n");
	}
    }

    packet = packet_create((const char *)&cmd, "ssdd", handler->common->pkgname, handler->common->id, x, y);
    if (!packet) {
	ErrPrint("Failed to build param\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    if (!cb) {
	cb = default_gbar_created_cb;
    }

    DbgPrint("PERF_DBOX\n");
    ret = master_rpc_async_request(handler, packet, 0, gbar_create_cb, NULL);
    if (ret == (int)DBOX_STATUS_ERROR_NONE) {
	handler->cbs.gbar_created.cb = cb;
	handler->cbs.gbar_created.data = data;
	handler->common->request.gbar_created = 1;
    }

    return ret;
}

EAPI int dynamicbox_move_glance_bar(dynamicbox_h handler, double x, double y)
{
    struct packet *packet;
    unsigned int cmd = CMD_GBAR_MOVE;

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->gbar.fb || !handler->common->id) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->is_gbar_created) {
	ErrPrint("GBAR is not created\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    packet = packet_create_noack((const char *)&cmd, "ssdd", handler->common->pkgname, handler->common->id, x, y);
    if (!packet) {
	ErrPrint("Failed to build param\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    return master_rpc_request_only(handler, packet);
}

EAPI int dynamicbox_activate(const char *pkgname, dynamicbox_ret_cb cb, void *data)
{
    struct packet *packet;
    struct cb_info *cbinfo;
    unsigned int cmd = CMD_ACTIVATE_PACKAGE;
    int ret;

    if (!pkgname) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    packet = packet_create((const char *)&cmd, "s", pkgname);
    if (!packet) {
	ErrPrint("Failed to build a param\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    cbinfo = dbox_create_cb_info(cb, data);
    if (!cbinfo) {
	ErrPrint("Unable to create cbinfo\n");
	packet_destroy(packet);
	return DBOX_STATUS_ERROR_FAULT;
    }

    ret = master_rpc_async_request(NULL, packet, 0, activated_cb, cbinfo);
    if (ret < 0) {
	dbox_destroy_cb_info(cbinfo);
    }

    return ret;
}

EAPI int dynamicbox_destroy_glance_bar(dynamicbox_h handler, dynamicbox_ret_cb cb, void *data)
{
    struct packet *packet;
    struct cb_info *cbinfo;
    unsigned int cmd = CMD_DESTROY_GBAR;
    int ret;

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->gbar.fb || !handler->common->id) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    /*!
     * \FIXME
     * Replace the callback check code.
     * Use the flag instead of callback.
     * the flag should be in the ADT "common"
     */
    if (!handler->common->is_gbar_created && !handler->common->request.gbar_created) {
	ErrPrint("GBAR is not created\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (handler->common->request.gbar_destroyed) {
	ErrPrint("GBAR destroy request is already sent\n");
	return DBOX_STATUS_ERROR_ALREADY;
    }

    /*!
     * \note
     * Disable the gbar_created request flag
     */
    if (handler->common->request.gbar_created) {
	if (job_add(handler, turn_off_gbar_created_flag_cb, DBOX_STATUS_ERROR_CANCEL, NULL) < 0) {
	    ErrPrint("Failed to add a new job\n");
	}
    }

    DbgPrint("[%s]\n", handler->common->pkgname);

    packet = packet_create((const char *)&cmd, "ss", handler->common->pkgname, handler->common->id);
    if (!packet) {
	ErrPrint("Failed to build a param\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    if (!cb) {
	cb = default_gbar_destroyed_cb;
    }

    cbinfo = dbox_create_cb_info(cb, data);
    if (!cbinfo) {
	packet_destroy(packet);
	return DBOX_STATUS_ERROR_FAULT;
    }

    ret = master_rpc_async_request(handler, packet, 0, gbar_destroy_cb, cbinfo);
    if (ret < 0) {
	dbox_destroy_cb_info(cbinfo);
    } else {
	handler->common->request.gbar_destroyed = 1;
    }

    return ret;
}

EAPI int dynamicbox_feed_access_event(dynamicbox_h handler, dynamicbox_access_event_type_e type, dynamicbox_access_event_info_t info, dynamicbox_ret_cb cb, void *data)
{
    int w = 1;
    int h = 1;
    unsigned int cmd;
    int ret = 0;    /* re-used for sending event type */

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (handler->common->request.access_event) {
	ErrPrint("Previous access event is not yet done\n");
	return DBOX_STATUS_ERROR_BUSY;
    }

    if (type & DBOX_ACCESS_EVENT_GBAR_MASK) {
	if (!handler->common->is_gbar_created) {
	    ErrPrint("GBAR is not created\n");
	    return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	w = handler->common->gbar.width;
	h = handler->common->gbar.height;

	switch (type & ~(DBOX_ACCESS_EVENT_GBAR_MASK | DBOX_ACCESS_EVENT_DBOX_MASK)) {
	    case DBOX_ACCESS_EVENT_HIGHLIGHT:
		cmd = CMD_GBAR_ACCESS_HL;
		ret = (int)info->type;
		break;
	    case DBOX_ACCESS_EVENT_ACTIVATE:
		cmd = CMD_GBAR_ACCESS_ACTIVATE;
		break;
	    case DBOX_ACCESS_EVENT_ACTION:
		cmd = CMD_GBAR_ACCESS_ACTION;
		ret = (int)info->type;
		break;
	    case DBOX_ACCESS_EVENT_SCROLL:
		cmd = CMD_GBAR_ACCESS_SCROLL;
		ret = (int)info->type;
		break;
	    case DBOX_ACCESS_EVENT_VALUE_CHANGE:
		cmd = CMD_GBAR_ACCESS_VALUE_CHANGE;
		break;
	    case DBOX_ACCESS_EVENT_MOUSE:
		cmd = CMD_GBAR_ACCESS_MOUSE;
		ret = (int)info->type;
		break;
	    case DBOX_ACCESS_EVENT_BACK:
		cmd = CMD_GBAR_ACCESS_BACK;
		break;
	    case DBOX_ACCESS_EVENT_OVER:
		cmd = CMD_GBAR_ACCESS_OVER;
		break;
	    case DBOX_ACCESS_EVENT_READ:
		cmd = CMD_GBAR_ACCESS_READ;
		break;
	    case DBOX_ACCESS_EVENT_ENABLE:
		cmd = CMD_GBAR_ACCESS_ENABLE;
		ret = info->type;
		break;
	    default:
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

    } else if (type & DBOX_ACCESS_EVENT_DBOX_MASK) {
	w = handler->common->dbox.width;
	h = handler->common->dbox.height;
	switch (type & ~(DBOX_ACCESS_EVENT_GBAR_MASK | DBOX_ACCESS_EVENT_DBOX_MASK)) {
	    case DBOX_ACCESS_EVENT_HIGHLIGHT:
		cmd = CMD_DBOX_ACCESS_HL;
		ret = (int)info->type;
		break;
	    case DBOX_ACCESS_EVENT_ACTIVATE:
		cmd = CMD_DBOX_ACCESS_ACTIVATE;
		break;
	    case DBOX_ACCESS_EVENT_ACTION:
		cmd = CMD_DBOX_ACCESS_ACTION;
		ret = (int)info->type;
		break;
	    case DBOX_ACCESS_EVENT_SCROLL:
		cmd = CMD_DBOX_ACCESS_SCROLL;
		ret = (int)info->type;
		break;
	    case DBOX_ACCESS_EVENT_VALUE_CHANGE:
		cmd = CMD_DBOX_ACCESS_VALUE_CHANGE;
		break;
	    case DBOX_ACCESS_EVENT_MOUSE:
		cmd = CMD_DBOX_ACCESS_MOUSE;
		ret = (int)info->type;
		break;
	    case DBOX_ACCESS_EVENT_BACK:
		cmd = CMD_DBOX_ACCESS_BACK;
		break;
	    case DBOX_ACCESS_EVENT_OVER:
		cmd = CMD_DBOX_ACCESS_OVER;
		break;
	    case DBOX_ACCESS_EVENT_READ:
		cmd = CMD_DBOX_ACCESS_READ;
		break;
	    case DBOX_ACCESS_EVENT_ENABLE:
		cmd = CMD_DBOX_ACCESS_ENABLE;
		ret = info->type;
		break;
	    default:
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}
    } else {
	ErrPrint("Invalid event type\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!cb) {
	cb = default_access_event_cb;
    }

    ret = send_access_event(handler, (const char *)&cmd, info->x * w, info->y * h, ret);
    if (ret == (int)DBOX_STATUS_ERROR_NONE) {
	handler->cbs.access_event.cb = cb;
	handler->cbs.access_event.data = data;
	handler->common->request.access_event = 1;
    }

    return ret;
}

EAPI int dynamicbox_feed_mouse_event(dynamicbox_h handler, dynamicbox_mouse_event_type_e type, dynamicbox_mouse_event_info_t info)
{
    int w = 1;
    int h = 1;
    unsigned int cmd;

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!(type & DBOX_MOUSE_EVENT_MASK)) {
	ErrPrint("Invalid content event is used\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (type & DBOX_MOUSE_EVENT_GBAR_MASK) {
	int flag = 1;

	if (!handler->common->is_gbar_created) {
	    ErrPrint("GBAR is not created\n");
	    return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	if (!handler->common->gbar.fb) {
	    ErrPrint("Handler is not valid\n");
	    return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	if (type & DBOX_MOUSE_EVENT_MOVE) {
	    if (fabs(info->x - handler->common->gbar.x) < conf_event_filter() && fabs(info->y - handler->common->gbar.y) < conf_event_filter()) {
		return DBOX_STATUS_ERROR_BUSY;
	    }
	} else if (type & DBOX_MOUSE_EVENT_SET) {
	    flag = 0;
	}

	if (flag) {
	    handler->common->gbar.x = info->x;
	    handler->common->gbar.y = info->y;
	    w = handler->common->gbar.width;
	    h = handler->common->gbar.height;
	}

	switch ((type & ~(DBOX_MOUSE_EVENT_GBAR_MASK | DBOX_MOUSE_EVENT_DBOX_MASK))) {
	    case DBOX_MOUSE_EVENT_ENTER | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_GBAR_MOUSE_ENTER;
		break;
	    case DBOX_MOUSE_EVENT_LEAVE | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_GBAR_MOUSE_LEAVE;
		break;
	    case DBOX_MOUSE_EVENT_UP | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_GBAR_MOUSE_UP;
		break;
	    case DBOX_MOUSE_EVENT_DOWN | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_GBAR_MOUSE_DOWN;
		break;
	    case DBOX_MOUSE_EVENT_MOVE | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_GBAR_MOUSE_MOVE;
		break;
	    case DBOX_MOUSE_EVENT_SET | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_GBAR_MOUSE_SET;
		break;
	    case DBOX_MOUSE_EVENT_UNSET | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_GBAR_MOUSE_UNSET;
		break;
	    case DBOX_MOUSE_EVENT_ON_SCROLL | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_GBAR_MOUSE_ON_SCROLL;
		break;
	    case DBOX_MOUSE_EVENT_ON_HOLD | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_GBAR_MOUSE_ON_HOLD;
		break;
	    case DBOX_MOUSE_EVENT_OFF_SCROLL | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_GBAR_MOUSE_OFF_SCROLL;
		break;
	    case DBOX_MOUSE_EVENT_OFF_HOLD | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_GBAR_MOUSE_OFF_HOLD;
		break;
	    default:
		ErrPrint("Invalid event type\n");
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

    } else if (type & DBOX_MOUSE_EVENT_DBOX_MASK) {
	int flag = 1;

	if (!handler->common->dbox.fb) {
	    ErrPrint("Handler is not valid\n");
	    return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	if (type & DBOX_MOUSE_EVENT_MOVE) {
	    if (fabs(info->x - handler->common->dbox.x) < conf_event_filter() && fabs(info->y - handler->common->dbox.y) < conf_event_filter()) {
		return DBOX_STATUS_ERROR_BUSY;
	    }
	} else if (type & DBOX_MOUSE_EVENT_SET) {
	    flag = 0;
	}

	if (flag) {
	    handler->common->dbox.x = info->x;
	    handler->common->dbox.y = info->y;
	    w = handler->common->dbox.width;
	    h = handler->common->dbox.height;
	}

	switch ((type & ~(DBOX_MOUSE_EVENT_GBAR_MASK | DBOX_MOUSE_EVENT_DBOX_MASK))) {
	    case DBOX_MOUSE_EVENT_ENTER | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_DBOX_MOUSE_ENTER;
		break;
	    case DBOX_MOUSE_EVENT_LEAVE | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_DBOX_MOUSE_LEAVE;
		break;
	    case DBOX_MOUSE_EVENT_UP | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_DBOX_MOUSE_UP;
		break;
	    case DBOX_MOUSE_EVENT_DOWN | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_DBOX_MOUSE_DOWN;
		break;
	    case DBOX_MOUSE_EVENT_MOVE | DBOX_MOUSE_EVENT_MASK:
		if (!handler->common->dbox.mouse_event) {
		    return DBOX_STATUS_ERROR_INVALID_PARAMETER;
		}
		cmd = CMD_DBOX_MOUSE_MOVE;
		break;
	    case DBOX_MOUSE_EVENT_SET | DBOX_MOUSE_EVENT_MASK:
		if (!handler->common->dbox.mouse_event) {
		    return DBOX_STATUS_ERROR_INVALID_PARAMETER;
		}
		cmd = CMD_DBOX_MOUSE_SET;
		break;
	    case DBOX_MOUSE_EVENT_UNSET | DBOX_MOUSE_EVENT_MASK:
		if (!handler->common->dbox.mouse_event) {
		    return DBOX_STATUS_ERROR_INVALID_PARAMETER;
		}
		cmd = CMD_DBOX_MOUSE_UNSET;
		break;
	    case DBOX_MOUSE_EVENT_ON_SCROLL | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_DBOX_MOUSE_ON_SCROLL;
		break;
	    case DBOX_MOUSE_EVENT_ON_HOLD | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_DBOX_MOUSE_ON_HOLD;
		break;
	    case DBOX_MOUSE_EVENT_OFF_SCROLL | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_DBOX_MOUSE_OFF_SCROLL;
		break;
	    case DBOX_MOUSE_EVENT_OFF_HOLD | DBOX_MOUSE_EVENT_MASK:
		cmd = CMD_DBOX_MOUSE_OFF_HOLD;
		break;
	    default:
		ErrPrint("Invalid event type\n");
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}
    } else {
	ErrPrint("Invalid event type\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    return send_mouse_event(handler, (const char *)&cmd, info->x * w, info->y * h);
}

EAPI int dynamicbox_feed_key_event(dynamicbox_h handler, dynamicbox_key_event_type_e type, dynamicbox_key_event_info_t info, dynamicbox_ret_cb cb, void *data)
{
    int ret;
    unsigned int cmd;

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!(type & DBOX_KEY_EVENT_MASK)) {
	ErrPrint("Invalid key event is used\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (handler->common->request.key_event) {
	ErrPrint("Previous key event is not completed yet\n");
	return DBOX_STATUS_ERROR_BUSY;
    }

    if (type & DBOX_MOUSE_EVENT_GBAR_MASK) {
	if (!handler->common->is_gbar_created) {
	    ErrPrint("GBAR is not created\n");
	    return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	if (!handler->common->gbar.fb) {
	    ErrPrint("Handler is not valid\n");
	    return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	if (type & DBOX_KEY_EVENT_DOWN) {
	    /*!
	     * \TODO
	     * filtering the reproduced events if it is too fast
	     */
	} else if (type & DBOX_KEY_EVENT_SET) {
	    /*!
	     * \TODO
	     * What can I do for this case?
	     */
	}

	/*!
	 * Must be short than 29 bytes.
	 */
	switch ((type & ~(DBOX_MOUSE_EVENT_GBAR_MASK | DBOX_MOUSE_EVENT_DBOX_MASK))) {
	    case DBOX_KEY_EVENT_FOCUS_IN | DBOX_KEY_EVENT_MASK:
		cmd = CMD_GBAR_KEY_FOCUS_IN;
		break;
	    case DBOX_KEY_EVENT_FOCUS_OUT | DBOX_KEY_EVENT_MASK:
		cmd = CMD_GBAR_KEY_FOCUS_OUT;
		break;
	    case DBOX_KEY_EVENT_UP | DBOX_KEY_EVENT_MASK:
		cmd = CMD_GBAR_KEY_UP;
		break;
	    case DBOX_KEY_EVENT_DOWN | DBOX_KEY_EVENT_MASK:
		cmd = CMD_GBAR_KEY_DOWN;
		break;
	    case DBOX_KEY_EVENT_SET | DBOX_KEY_EVENT_MASK:
		cmd = CMD_GBAR_KEY_SET;
		break;
	    case DBOX_KEY_EVENT_UNSET | DBOX_KEY_EVENT_MASK:
		cmd = CMD_GBAR_KEY_UNSET;
		break;
	    default:
		ErrPrint("Invalid event type\n");
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

    } else if (type & DBOX_MOUSE_EVENT_DBOX_MASK) {
	if (!handler->common->dbox.fb) {
	    ErrPrint("Handler is not valid\n");
	    return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	if (type & DBOX_KEY_EVENT_DOWN) {
	    /*!
	     * \TODO
	     * filtering the reproduced events if it is too fast
	     */
	} else if (type & DBOX_KEY_EVENT_SET) {
	    /*!
	     * What can I do for this case?
	     */
	}

	switch ((type & ~(DBOX_MOUSE_EVENT_GBAR_MASK | DBOX_MOUSE_EVENT_DBOX_MASK))) {
	    case DBOX_KEY_EVENT_FOCUS_IN | DBOX_KEY_EVENT_MASK:
		cmd = CMD_DBOX_KEY_FOCUS_IN;
		break;
	    case DBOX_KEY_EVENT_FOCUS_OUT | DBOX_KEY_EVENT_MASK:
		cmd = CMD_DBOX_KEY_FOCUS_OUT;
		break;
	    case DBOX_KEY_EVENT_UP | DBOX_KEY_EVENT_MASK:
		cmd = CMD_DBOX_KEY_UP;
		break;
	    case DBOX_KEY_EVENT_DOWN | DBOX_KEY_EVENT_MASK:
		cmd = CMD_DBOX_KEY_DOWN;
		break;
	    case DBOX_KEY_EVENT_SET | DBOX_KEY_EVENT_MASK:
		cmd = CMD_DBOX_KEY_SET;
		break;
	    case DBOX_KEY_EVENT_UNSET | DBOX_KEY_EVENT_MASK:
		cmd = CMD_DBOX_KEY_UNSET;
		break;
	    default:
		ErrPrint("Invalid event type\n");
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}
    } else {
	ErrPrint("Invalid event type\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!cb) {
	cb = default_key_event_cb;
    }

    ret = send_key_event(handler, (const char *)&cmd, info->keycode);
    if (ret == (int)DBOX_STATUS_ERROR_NONE) {
	handler->cbs.key_event.cb = cb;
	handler->cbs.key_event.data = data;
	handler->common->request.key_event = 1;
    }

    return ret;
}

EAPI const char *dynamicbox_filename(dynamicbox_h handler)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return NULL;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
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
    dynamicbox_set_last_status(DBOX_STATUS_ERROR_NONE);
    return util_uri_to_path(handler->common->id);
}

EAPI int dynamicbox_get_glance_bar_size(dynamicbox_h handler, int *w, int *h)
{
    int _w;
    int _h;

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!w) {
	w = &_w;
    }
    if (!h) {
	h = &_h;
    }

    if (!handler->common->is_gbar_created) {
	*w = handler->common->gbar.default_width;
	*h = handler->common->gbar.default_height;
    } else {
	*w = handler->common->gbar.width;
	*h = handler->common->gbar.height;
    }

    return DBOX_STATUS_ERROR_NONE;
}

EAPI dynamicbox_size_type_e dynamicbox_size(dynamicbox_h handler)
{
    int w;
    int h;

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_SIZE_TYPE_UNKNOWN;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_SIZE_TYPE_UNKNOWN;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_SIZE_TYPE_UNKNOWN;
    }

    w = handler->common->dbox.width;
    h = handler->common->dbox.height;

    switch (handler->common->dbox.type) {
	case DBOX_TYPE_BUFFER:
	case DBOX_TYPE_SCRIPT:
	    if (!fb_is_created(handler->common->dbox.fb)) {
		w = 0;
		h = 0;
		dynamicbox_set_last_status(DBOX_STATUS_ERROR_NOT_EXIST);
	    }
	    break;
	default:
	    break;
    }

    return dynamicbox_service_size_type(w, h);
}

EAPI int dynamicbox_set_group(dynamicbox_h handler, const char *cluster, const char *category, dynamicbox_ret_cb cb, void *data)
{
    struct packet *packet;
    unsigned int cmd = CMD_CHANGE_GROUP;
    int ret;

    if (!handler) {
	ErrPrint("Handler is NIL\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!cluster || !category || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Invalid argument\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Invalid argument\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Invalid argument\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (handler->common->request.group_changed) {
	ErrPrint("Previous group changing request is not finished yet\n");
	return DBOX_STATUS_ERROR_BUSY;
    }

    if (!handler->common->is_user) {
	ErrPrint("CA Livebox is not able to change the group\n");
	return DBOX_STATUS_ERROR_PERMISSION_DENIED;
    }

    if (!strcmp(handler->common->cluster, cluster) && !strcmp(handler->common->category, category)) {
	DbgPrint("No changes\n");
	return DBOX_STATUS_ERROR_ALREADY;
    }

    packet = packet_create((const char *)&cmd, "ssss", handler->common->pkgname, handler->common->id, cluster, category);
    if (!packet) {
	ErrPrint("Failed to build a param\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    if (!cb) {
	cb = default_group_changed_cb;
    }

    ret = master_rpc_async_request(handler, packet, 0, set_group_ret_cb, NULL);
    if (ret == (int)DBOX_STATUS_ERROR_NONE) {
	handler->cbs.group_changed.cb = cb;
	handler->cbs.group_changed.data = data; 
	handler->common->request.group_changed = 1;
    }

    return ret;
}

EAPI int dynamicbox_get_group(dynamicbox_h handler, const char **cluster, const char **category)
{
    if (!handler) {
	ErrPrint("Handler is NIL\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!cluster || !category || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Invalid argument\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Invalid argument\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Invalid argument\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    *cluster = handler->common->cluster;
    *category = handler->common->category;
    return DBOX_STATUS_ERROR_NONE;
}

EAPI int dynamicbox_get_supported_sizes(dynamicbox_h handler, int *cnt, dynamicbox_size_type_e *size_list)
{
    register int i;
    register int j;

    if (!handler || !size_list) {
	ErrPrint("Invalid argument, handler(%p), size_list(%p)\n", handler, size_list);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!cnt || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    for (j = i = 0; i < DBOX_NR_OF_SIZE_LIST; i++) {
	if (handler->common->dbox.size_list & (0x01 << i)) {
	    if (j == *cnt) {
		break;
	    }

	    size_list[j++] = (dynamicbox_size_type_e)(0x01 << i);
	}
    }

    *cnt = j;
    return DBOX_STATUS_ERROR_NONE;
}

EAPI const char *dynamicbox_pkgname(dynamicbox_h handler)
{
    if (!handler) {
	ErrPrint("Handler is NIL\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return NULL;
    }

    if (handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is not valid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return NULL;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is not valid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return NULL;
    }

    dynamicbox_set_last_status(DBOX_STATUS_ERROR_NONE);
    return handler->common->pkgname;
}

EAPI double dynamicbox_priority(dynamicbox_h handler)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return -1.0f;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return -1.0f;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid (%p)\n", handler);
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return -1.0f;
    }

    return handler->common->dbox.priority;
}

EAPI int dynamicbox_delete_cluster(const char *cluster, dynamicbox_ret_cb cb, void *data)
{
    struct packet *packet;
    struct cb_info *cbinfo;
    unsigned int cmd = CMD_DELETE_CLUSTER;
    int ret;

    packet = packet_create((const char *)&cmd, "s", cluster);
    if (!packet) {
	ErrPrint("Failed to build a param\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    cbinfo = dbox_create_cb_info(cb, data);
    if (!cbinfo) {
	packet_destroy(packet);
	return DBOX_STATUS_ERROR_FAULT;
    }

    ret = master_rpc_async_request(NULL, packet, 0, delete_cluster_cb, cbinfo);
    if (ret < 0) {
	dbox_destroy_cb_info(cbinfo);
    }

    return ret;
}

EAPI int dynamicbox_delete_category(const char *cluster, const char *category, dynamicbox_ret_cb cb, void *data)
{
    struct packet *packet;
    struct cb_info *cbinfo;
    unsigned int cmd = CMD_DELETE_CATEGORY;
    int ret;

    packet = packet_create((const char *)&cmd, "ss", cluster, category);
    if (!packet) {
	ErrPrint("Failed to build a param\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    cbinfo = dbox_create_cb_info(cb, data);
    if (!cbinfo) {
	packet_destroy(packet);
	return DBOX_STATUS_ERROR_FAULT;
    }

    ret = master_rpc_async_request(NULL, packet, 0, delete_category_cb, cbinfo);
    if (ret < 0) {
	dbox_destroy_cb_info(cbinfo);
    }

    return ret;
}

EAPI dynamicbox_type_e dynamicbox_type(dynamicbox_h handler, int gbar)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_CONTENT_TYPE_INVALID;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_CONTENT_TYPE_INVALID;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_CONTENT_TYPE_INVALID;
    }

    if (gbar) {
	switch (handler->common->gbar.type) {
	    case GBAR_TYPE_TEXT:
		return DBOX_CONTENT_TYPE_TEXT;
	    case GBAR_TYPE_BUFFER:
	    case GBAR_TYPE_SCRIPT:
		{
		    const char *id;
		    id = fb_id(handler->common->gbar.fb);
		    if (id && !strncasecmp(id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP))) {
			return DBOX_CONTENT_TYPE_RESOURCE_ID;
		    }
		}
		return DBOX_CONTENT_TYPE_BUFFER;
	    case GBAR_TYPE_UIFW:
		return DBOX_CONTENT_TYPE_UIFW;
	    default:
		break;
	}
    } else {
	switch (handler->common->dbox.type) {
	    case DBOX_TYPE_FILE:
		return DBOX_CONTENT_TYPE_IMAGE;
	    case DBOX_TYPE_BUFFER:
	    case DBOX_TYPE_SCRIPT:
		{
		    const char *id;
		    id = fb_id(handler->common->dbox.fb);
		    if (id && !strncasecmp(id, SCHEMA_PIXMAP, strlen(SCHEMA_PIXMAP))) {
			return DBOX_CONTENT_TYPE_RESOURCE_ID;
		    }
		}
		return DBOX_CONTENT_TYPE_BUFFER;
	    case DBOX_TYPE_TEXT:
		return DBOX_CONTENT_TYPE_TEXT;
	    case DBOX_TYPE_UIFW:
		return DBOX_CONTENT_TYPE_UIFW;
	    default:
		break;
	}
    }

    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
    return DBOX_CONTENT_TYPE_INVALID;
}

EAPI int dynamicbox_set_text_handler(dynamicbox_h handler, int gbar, struct dynamicbox_script_operators *ops)
{
    if (!handler) {
	ErrPrint("Handler is NIL\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (gbar) {
	memcpy(&handler->cbs.gbar_ops, ops, sizeof(*ops));
    } else {
	memcpy(&handler->cbs.dbox_ops, ops, sizeof(*ops));
    }

    return DBOX_STATUS_ERROR_NONE;
}

EAPI int dynamicbox_acquire_extra_resource_id(dynamicbox_h handler, int gbar, int idx, dynamicbox_ret_cb cb, void *data)
{
    if (idx < 0) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Invalid handle\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (gbar) {
	/**
	 * This can be called from extra_resource_created event.
	 * and it can be called before get the created event.
	 * then we didn't know this handler's buffer type yet
	 * so we cannot use its type to validate handle 
	 *
	 * handler->common.gbar.type == unknown
	 */
	if (!handler->common->gbar.extra_buffer) {
	    return DBOX_STATUS_ERROR_NOT_EXIST;
	}

	if (idx >= conf_extra_buffer_count()) {
	    return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	return dbox_acquire_gbar_extra_pixmap(handler, idx, cb, data);
    } else {
	/**
	 * This can be called from extra_resource_created event.
	 * and it can be called before get the created event.
	 * then we didn't know this handler's buffer type yet
	 * so we cannot use its type to validate handle 
	 *
	 * handler->common.dbox.type == unknown
	 */
	if (!handler->common->dbox.extra_buffer) {
	    ErrPrint("Extra buffer is not prepared\n");
	    return DBOX_STATUS_ERROR_NOT_EXIST;
	}

	if (idx >= conf_extra_buffer_count()) {
	    ErrPrint("Invalid parameter: %d / %d\n", idx, conf_extra_buffer_count());
	    return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	return dbox_acquire_dbox_extra_pixmap(handler, idx, cb, data);
    }
}

EAPI int dynamicbox_acquire_resource_id(dynamicbox_h handler, int gbar, dynamicbox_ret_cb cb, void *data)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Invalid handle\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (gbar) {
	if (handler->common->gbar.type != GBAR_TYPE_SCRIPT && handler->common->gbar.type != GBAR_TYPE_BUFFER) {
	    ErrPrint("Handler is not valid type\n");
	    return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	return dbox_acquire_gbar_pixmap(handler, cb, data);
    } else {
	if (handler->common->dbox.type != DBOX_TYPE_SCRIPT && handler->common->dbox.type != DBOX_TYPE_BUFFER) {
	    ErrPrint("Handler is not valid type\n");
	    return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	return dbox_acquire_dbox_pixmap(handler, cb, data);
    }
}

/*!
 * \note
 * Do not check the state of handler and common-handler.
 * If this function is used in the deleted callback,
 * the handler and common-handler's state would be DELETE
 * if this function check the state of handles,
 * user cannot release the pixmap.
 */
EAPI int dynamicbox_release_resource_id(dynamicbox_h handler, int gbar, unsigned int resource_id)
{
    struct packet *packet;
    const char *pkgname;
    const char *id;
    unsigned int cmd;

    if (resource_id == 0 /* || handler->state != DBOX_STATE_CREATE */) {
	ErrPrint("Pixmap is invalid [%d]\n", resource_id);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (gbar) {
	if (!handler) {
	    /*!
	     * \note
	     * Even though the handler is NULL, we should send the release request to the master.
	     * Because the resource_id resource can be released after the handler is destroyed.
	     * Pixmap resource is used by client. and it cannot be guaranteed to release resource_id.
	     * In some cases, the resource_id can be released after the handler is deleted.
	     *
	     * Its implementation is up to the viewer app.
	     * But we cannot force it to use only with valid handler.
	     */
	    DbgPrint("Using NULL handler\n");
	    pkgname = NULL;
	    id = NULL;
	    /*!
	     * \note
	     * Master will try to find the buffer handler using given resource_id. if the pkgname and id is not valid.
	     */
	} else {
	    if (!handler->common /* || handler-common->state != DBOX_STATE_CREATE */) {
		ErrPrint("Handler is invalid\n");
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	    }

	    if (!handler->common->id) {
		ErrPrint("Invalid handle\n");
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	    }

	    /**
	     * This can be called from extra_resource_created event.
	     * and it can be called before get the created event.
	     * then we didn't know this handler's buffer type yet
	     * so we cannot use its type to validate handle 
	     *
	     * handler->common.gbar.type == unknown
	     */

	    pkgname = handler->common->pkgname;
	    id = handler->common->id;
	}

	cmd = CMD_GBAR_RELEASE_PIXMAP;
    } else {
	if (!handler) {
	    /*!
	     * \note
	     * Even though the handler is NULL, we should send the release request to the master.
	     * Because the resource_id resource can be released after the handler is destroyed.
	     * Pixmap resource is used by client. and it cannot be guaranteed to release resource_id.
	     * In some cases, the resource_id can be released after the handler is deleted.
	     *
	     * Its implementation is up to the viewer app.
	     * But we cannot force it to use only with valid handler.
	     */
	    DbgPrint("Using NULL handler\n");
	    pkgname = NULL;
	    id = NULL;
	    /*!
	     * \note
	     * Master will try to find the buffer handler using given resource_id. if the pkgname and id is not valid.
	     */
	} else {
	    if (!handler->common /* || handler->common->state != DBOX_STATE_CREATE */) {
		ErrPrint("Handler is invalid\n");
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	    }

	    if (!handler->common->id) {
		ErrPrint("Invalid handle\n");
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	    }

	    /**
	     * This can be called from extra_resource_created event.
	     * and it can be called before get the created event.
	     * then we didn't know this handler's buffer type yet
	     * so we cannot use its type to validate handle 
	     *
	     * handler->common.dbox.type == unknown
	     */

	    pkgname = handler->common->pkgname;
	    id = handler->common->id;
	}

	cmd = CMD_DBOX_RELEASE_PIXMAP;
    }

    packet = packet_create_noack((const char *)&cmd, "ssi", pkgname, id, resource_id);
    if (!packet) {
	ErrPrint("Failed to build a param\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    return master_rpc_request_only(handler, packet);
}

EAPI unsigned int dynamicbox_extra_resource_id(const dynamicbox_h handler, int gbar, int idx)
{
    if (idx < 0) {
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return 0u;
    }

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return 0u;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return 0u;
    }

    if (!handler->common->id) {
	ErrPrint("Invalid handler\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return 0u;
    }

    if (gbar) {
	/**
	 * This can be called from extra_resource_created event.
	 * and it can be called before get the created event.
	 * then we didn't know this handler's buffer type yet
	 * so we cannot use its type to validate handle 
	 *
	 * handler->common.gbar.type == unknown
	 */

	if (!handler->common->gbar.extra_buffer || handler->common->gbar.last_extra_buffer_idx < 0) {
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_NOT_EXIST);
	    return 0u;
	}

	return handler->common->gbar.extra_buffer[handler->common->gbar.last_extra_buffer_idx];
    } else {
	/**
	 * This can be called from extra_resource_created event.
	 * and it can be called before get the created event.
	 * then we didn't know this handler's buffer type yet
	 * so we cannot use its type to validate handle 
	 *
	 * handler->common.dbox.type == unknown
	 */

	if (!handler->common->dbox.extra_buffer || handler->common->dbox.last_extra_buffer_idx < 0) {
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_NOT_EXIST);
	    return 0u;
	}

	return handler->common->dbox.extra_buffer[handler->common->dbox.last_extra_buffer_idx];
    }
}

EAPI unsigned int dynamicbox_resource_id(const dynamicbox_h handler, int gbar)
{
    const char *id;
    unsigned int pixmap = 0u;

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return 0u;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return 0u;
    }

    if (!handler->common->id) {
	ErrPrint("Invalid handler\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return 0u;
    }

    if (gbar) {
	if (handler->common->gbar.type != GBAR_TYPE_SCRIPT && handler->common->gbar.type != GBAR_TYPE_BUFFER) {
	    ErrPrint("Invalid handler\n");
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	    return 0u;
	}

	id = fb_id(handler->common->gbar.fb);
	if (id && sscanf(id, SCHEMA_PIXMAP "%u", &pixmap) != 1) {
	    ErrPrint("PIXMAP Id is not valid\n");
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	    return 0u;
	}
    } else {
	if (handler->common->dbox.type != DBOX_TYPE_SCRIPT && handler->common->dbox.type != DBOX_TYPE_BUFFER) {
	    ErrPrint("Invalid handler\n");
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	    return 0u;
	}

	id = fb_id(handler->common->dbox.fb);
	if (id && sscanf(id, SCHEMA_PIXMAP "%u", &pixmap) != 1) {
	    ErrPrint("PIXMAP Id is not valid\n");
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	    return 0u;
	}
    }

    return pixmap;
}

EAPI void *dynamicbox_acquire_buffer(dynamicbox_h handler, int gbar)
{
    if (gbar) {
	if (!handler || handler->state != DBOX_STATE_CREATE) {
	    ErrPrint("Handler is invalid\n");
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	    return NULL;
	}

	if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	    ErrPrint("Handler is invalid\n");
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	    return NULL;
	}

	if (!handler->common->id) {
	    ErrPrint("Invalid handler\n");
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	    return NULL;
	}

	if (handler->common->gbar.type != GBAR_TYPE_SCRIPT && handler->common->gbar.type != GBAR_TYPE_BUFFER) {
	    ErrPrint("Handler is not valid type\n");
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	    return NULL;
	}

	return fb_acquire_buffer(handler->common->gbar.fb);
    } else {
	if (!handler || handler->state != DBOX_STATE_CREATE) {
	    ErrPrint("Handler is invalid\n");
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	    return NULL;
	}

	if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	    ErrPrint("Handler is invalid\n");
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	    return NULL;
	}

	if (!handler->common->id) {
	    ErrPrint("Invalid handle\n");
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	    return NULL;
	}

	if (handler->common->dbox.type != DBOX_TYPE_SCRIPT && handler->common->dbox.type != DBOX_TYPE_BUFFER) {
	    ErrPrint("Handler is not valid type\n");
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	    return NULL;
	}

	return fb_acquire_buffer(handler->common->dbox.fb);
    }
}

EAPI int dynamicbox_release_buffer(void *buffer)
{
    return fb_release_buffer(buffer);
}

EAPI int dynamicbox_buffer_refcnt(void *buffer)
{
    return fb_refcnt(buffer);
}

EAPI int dynamicbox_buffer_size(dynamicbox_h handler, int gbar)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Invalid handler\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (gbar) {
	return fb_size(handler->common->gbar.fb);
    } else {
	return fb_size(handler->common->dbox.fb);
    }
}

EAPI int dynamicbox_is_created_by_user(dynamicbox_h handler)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Invalid handler\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    return handler->common->is_user;
}

EAPI int dynamicbox_set_pinup(dynamicbox_h handler, int flag, dynamicbox_ret_cb cb, void *data)
{
    struct packet *packet;
    unsigned int cmd = CMD_PINUP_CHANGED;
    int ret;

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Invalid handler\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (handler->common->request.pinup) {
	ErrPrint("Previous pinup request is not finished\n");
	return DBOX_STATUS_ERROR_BUSY;
    }

    if (handler->common->is_pinned_up == flag) {
	DbgPrint("No changes\n");
	return DBOX_STATUS_ERROR_ALREADY;
    }

    packet = packet_create((const char *)&cmd, "ssi", handler->common->pkgname, handler->common->id, flag);
    if (!packet) {
	ErrPrint("Failed to build a param\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    if (!cb) {
	cb = default_pinup_cb;
    }

    ret = master_rpc_async_request(handler, packet, 0, pinup_done_cb, NULL);
    if (ret == (int)DBOX_STATUS_ERROR_NONE) {
	handler->cbs.pinup.cb = cb;
	handler->cbs.pinup.data = data;
	handler->common->request.pinup = 1;
    }

    return ret;
}

EAPI int dynamicbox_is_pinned_up(dynamicbox_h handler)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Invalid handler\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    return handler->common->is_pinned_up;
}

EAPI int dynamicbox_has_pinup(dynamicbox_h handler)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Invalid handler\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    return handler->common->dbox.pinup_supported;
}

EAPI int dynamicbox_set_data(dynamicbox_h handler, void *data)
{
    if (!handler) {
	ErrPrint("Handler is NIL\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    handler->data = data;
    return DBOX_STATUS_ERROR_NONE;
}

EAPI void *dynamicbox_data(dynamicbox_h handler)
{
    if (!handler) {
	ErrPrint("Handler is NIL\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return NULL;
    }

    if (handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return NULL;
    }

    return handler->data;
}

EAPI const char *dynamicbox_content(dynamicbox_h handler)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return NULL;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Invalid handle\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return NULL;
    }

    dynamicbox_set_last_status(DBOX_STATUS_ERROR_NONE);
    return handler->common->content;
}

EAPI const char *dynamicbox_title(dynamicbox_h handler)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return NULL;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Invalid handle\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return NULL;
    }

    dynamicbox_set_last_status(DBOX_STATUS_ERROR_NONE);
    return handler->common->title;
}

EAPI int dynamicbox_emit_text_signal(dynamicbox_h handler, dynamicbox_text_event_t event_info, dynamicbox_ret_cb cb, void *data)
{
    struct packet *packet;
    struct cb_info *cbinfo;
    unsigned int cmd = CMD_TEXT_SIGNAL;
    int ret;
    const char *emission;
    const char *source;

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (handler->common->dbox.type != DBOX_TYPE_TEXT && handler->common->gbar.type != GBAR_TYPE_TEXT) {
	DbgPrint("Not a text box, but send signal\n");
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!event_info) {
	ErrPrint("Invalid event info\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    emission = event_info->emission;
    if (!emission) {
	emission = "";
    }

    source = event_info->source;
    if (!source) {
	source = "";
    }

    packet = packet_create((const char *)&cmd, "ssssdddd",
	    handler->common->pkgname, handler->common->id,
	    emission, source,
	    event_info->geometry.sx, event_info->geometry.sy,
	    event_info->geometry.ex, event_info->geometry.ey);
    if (!packet) {
	ErrPrint("Failed to build a param\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    cbinfo = dbox_create_cb_info(cb, data);
    if (!cbinfo) {
	packet_destroy(packet);
	return DBOX_STATUS_ERROR_FAULT;
    }

    ret = master_rpc_async_request(handler, packet, 0, text_signal_cb, cbinfo);
    if (ret < 0) {
	dbox_destroy_cb_info(cbinfo);
    }

    return ret;
}

EAPI int dynamicbox_subscribe_group(const char *cluster, const char *category)
{
    struct packet *packet;
    unsigned int cmd = CMD_SUBSCRIBE;

    /*!
     * \todo
     * Validate the group info using DB
     * If the group info is not valid, do not send this request
     */

    packet = packet_create_noack((const char *)&cmd, "ss", cluster ? cluster : "", category ? category : "");
    if (!packet) {
	ErrPrint("Failed to create a packet\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    return master_rpc_request_only(NULL, packet);
}

EAPI int dynamicbox_unsubscribe_group(const char *cluster, const char *category)
{
    struct packet *packet;
    unsigned int cmd = CMD_UNSUBSCRIBE;

    /*!
     * \todo
     * Validate the group info using DB
     * If the group info is not valid, do not send this request
     * AND Check the subscribed or not too
     */

    packet = packet_create_noack((const char *)&cmd, "ss", cluster ? cluster : "", category ? category : "");
    if (!packet) {
	ErrPrint("Failed to create a packet\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    return master_rpc_request_only(NULL, packet);
}

EAPI int dynamicbox_refresh(dynamicbox_h handler, int force)
{
    struct packet *packet;
    unsigned int cmd = CMD_UPDATE;

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    packet = packet_create_noack((const char *)&cmd, "ssi", handler->common->pkgname, handler->common->id, force);
    if (!packet) {
	ErrPrint("Failed to create a packet\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    return master_rpc_request_only(handler, packet);
}

EAPI int dynamicbox_refresh_group(const char *cluster, const char *category, int force)
{
    struct packet *packet;
    unsigned int cmd = CMD_REFRESH_GROUP;

    if (!cluster || !category) {
	ErrPrint("Invalid argument\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    packet = packet_create_noack((const char *)&cmd, "ssi", cluster, category, force);
    if (!packet) {
	ErrPrint("Failed to create a packet\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    return master_rpc_request_only(NULL, packet);
}

EAPI int dynamicbox_set_visibility(dynamicbox_h handler, dynamicbox_visible_state_e state)
{
    int old_state;
    int ret;

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->is_user) {
	/* System cluster dynamicbox cannot be changed its visible states */
	if (state == DBOX_HIDE_WITH_PAUSE) {
	    ErrPrint("CA Livebox is not able to change the visibility\n");
	    return DBOX_STATUS_ERROR_PERMISSION_DENIED;
	}
    }

    if (handler->visible == state) {
	DbgPrint("%s has no changes\n", handler->common->pkgname);
	return DBOX_STATUS_ERROR_ALREADY;
    }

    old_state = handler->visible;
    handler->visible = state;

    ret = dbox_set_visibility(handler, state);
    if (ret < 0) {
	handler->visible = old_state;
    }

    return ret;
}

EAPI dynamicbox_visible_state_e dynamicbox_visibility(dynamicbox_h handler)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is invalid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_VISIBLE_ERROR;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is not valid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_VISIBLE_ERROR;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return DBOX_VISIBLE_ERROR;
    }

    return handler->visible;
}

EAPI int dynamicbox_viewer_set_paused(void)
{
    struct packet *packet;
    unsigned int cmd = CMD_CLIENT_PAUSED;

    packet = packet_create_noack((const char *)&cmd, "d", util_timestamp());
    if (!packet) {
	ErrPrint("Failed to create a pause packet\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    return master_rpc_request_only(NULL, packet);
}

EAPI int dynamicbox_viewer_set_resumed(void)
{
    struct packet *packet;
    unsigned int cmd = CMD_CLIENT_RESUMED;

    packet = packet_create_noack((const char *)&cmd, "d", util_timestamp());
    if (!packet) {
	ErrPrint("Failed to create a resume packet\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    return master_rpc_request_only(NULL, packet);
}

EAPI int dynamicbox_sync_buffer(dynamicbox_h handler, int gbar)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Invalid handle\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Invalid handle\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Invalid handle\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (gbar) {
	return dbox_sync_gbar_fb(handler->common);
    } else {
	return dbox_sync_dbox_fb(handler->common);
    }
}

EAPI const char *dynamicbox_alternative_icon(dynamicbox_h handler)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is not valid[%p]\n", handler);
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return NULL;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is not valid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return NULL;
    }

    return handler->common->alt.icon;
}

EAPI const char *dynamicbox_alternative_name(dynamicbox_h handler)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is not valid[%p]\n", handler);
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return NULL;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is not valid\n");
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	return NULL;
    }

    return handler->common->alt.name;
}

EAPI int dynamicbox_acquire_buffer_lock(dynamicbox_h handler, int is_gbar)
{
    int ret = DBOX_STATUS_ERROR_NONE;

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is not valid[%p]\n", handler);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Handler is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid[%p]\n", handler);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (is_gbar) {
	ret = dynamicbox_service_acquire_lock(handler->common->gbar.lock);
    } else {
	ret = dynamicbox_service_acquire_lock(handler->common->dbox.lock);
    }

    return ret == 0 ? DBOX_STATUS_ERROR_NONE : DBOX_STATUS_ERROR_FAULT;
}

EAPI int dynamicbox_release_buffer_lock(dynamicbox_h handler, int is_gbar)
{
    int ret = DBOX_STATUS_ERROR_NONE;

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Invalid handle\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Invalid handle\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid[%p]\n", handler);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (is_gbar) {
	ret = dynamicbox_service_release_lock(handler->common->gbar.lock);
    } else {
	ret = dynamicbox_service_release_lock(handler->common->dbox.lock);
    }

    return ret == 0 ? DBOX_STATUS_ERROR_NONE : DBOX_STATUS_ERROR_FAULT;
}

EAPI int dynamicbox_set_option(dynamicbox_option_type_e option, int state)
{
    int ret = DBOX_STATUS_ERROR_NONE;

    switch (option) {
	case DBOX_OPTION_MANUAL_SYNC:
	    conf_set_manual_sync(state);
	    break;
	case DBOX_OPTION_FRAME_DROP_FOR_RESIZE:
	    conf_set_frame_drop_for_resizing(state);
	    break;
	case DBOX_OPTION_SHARED_CONTENT:
	    conf_set_shared_content(state);
	    break;
	case DBOX_OPTION_DIRECT_UPDATE:
	    if (s_info.init_count) {
		DbgPrint("Already intialized, this option is not applied\n");
	    }
	    conf_set_direct_update(state);
	    break;
	case DBOX_OPTION_EXTRA_BUFFER_CNT:
	    ErrPrint("Permission denied\n");
	    ret = DBOX_STATUS_ERROR_PERMISSION_DENIED;
	    break;
	default:
	    ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
	    break;
    }

    return ret;
}

EAPI int dynamicbox_option(dynamicbox_option_type_e option)
{
    int ret;

    dynamicbox_set_last_status(DBOX_STATUS_ERROR_NONE);
    switch (option) {
	case DBOX_OPTION_MANUAL_SYNC:
	    ret = conf_manual_sync();
	    break;
	case DBOX_OPTION_FRAME_DROP_FOR_RESIZE:
	    ret = conf_frame_drop_for_resizing();
	    break;
	case DBOX_OPTION_SHARED_CONTENT:
	    ret = conf_shared_content();
	    break;
	case DBOX_OPTION_DIRECT_UPDATE:
	    ret = conf_direct_update();
	    break;
	case DBOX_OPTION_EXTRA_BUFFER_CNT:
	    ret = conf_extra_buffer_count();
	    break;
	default:
	    ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
	    dynamicbox_set_last_status(DBOX_STATUS_ERROR_INVALID_PARAMETER);
	    break;
    }

    return ret;
}

EAPI int dynamicbox_set_auto_launch_handler(dynamicbox_auto_launch_handler_cb dbox_launch_handler, void *data)
{
    s_info.launch.handler = dbox_launch_handler;
    s_info.launch.data = data;

    return DBOX_STATUS_ERROR_NONE;
}

EAPI int dynamicbox_damage_region_get(dynamicbox_h handler, int gbar, const dynamicbox_damage_region_t *region)
{
    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Invalid handle\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Invalid handle\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid[%p]\n", handler);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (gbar) {
	region = &handler->common->dbox.last_damage;
    } else {
	region = &handler->common->gbar.last_damage;
    }

    return DBOX_STATUS_ERROR_NONE;
}

EAPI int dynamicbox_get_affected_extra_buffer(dynamicbox_h handler, int gbar, int *idx, unsigned int *resource_id)
{
    int _idx;
    unsigned int _resource_id;

    if (!handler || handler->state != DBOX_STATE_CREATE) {
	ErrPrint("Invalid handle\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common || handler->common->state != DBOX_STATE_CREATE) {
	ErrPrint("Invalid handle\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!handler->common->id) {
	ErrPrint("Handler is not valid[%p]\n", handler);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!idx) {
	idx = &_idx;
    }

    if (!resource_id) {
	resource_id = &_resource_id;
    }

    if (gbar) {
	if (!handler->common->gbar.extra_buffer || handler->common->gbar.last_extra_buffer_idx < 0) {
	    return DBOX_STATUS_ERROR_NOT_EXIST;
	}

	*idx = handler->common->gbar.last_extra_buffer_idx;
	*resource_id = handler->common->gbar.extra_buffer[*idx];
    } else {
	if (!handler->common->dbox.extra_buffer || handler->common->dbox.last_extra_buffer_idx < 0) {
	    return DBOX_STATUS_ERROR_NOT_EXIST;
	}

	*idx = handler->common->dbox.last_extra_buffer_idx;
	*resource_id = handler->common->dbox.extra_buffer[*idx];
    }

    return DBOX_STATUS_ERROR_NONE;
}

/* End of a file */
