#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <dlog.h>

#include <dynamicbox_errno.h>
#include <dynamicbox_service.h>
#include <dynamicbox_buffer.h>

#include <packet.h>

#include "dlist.h"
#include "debug.h"
#include "dynamicbox.h"
#include "dynamicbox_internal.h"
#include "fb.h"
#include "conf.h"
#include "util.h"
#include "master_rpc.h"

int errno;

typedef enum event_state {
    INFO_STATE_CALLBACK_IN_IDLE = 0x00,
    INFO_STATE_CALLBACK_IN_PROCESSING = 0x01
} event_state_e;

struct event_info {
    int is_deleted;
    int (*handler)(dynamicbox_h handler, dynamicbox_event_type_e event, void *data);
    void *user_data;
};

struct fault_info {
    int is_deleted;
    int (*handler)(dynamicbox_fault_type_e event, const char *pkgname, const char *filename, const char *func, void *data);
    void *user_data;
};

static struct info {
    struct dlist *dynamicbox_common_list;
    struct dlist *dynamicbox_list;
    struct dlist *event_list;
    struct dlist *fault_list;
    event_state_e event_state;
    event_state_e fault_state;
} s_info = {
    .dynamicbox_common_list = NULL,
    .dynamicbox_list = NULL,
    .event_list = NULL,
    .fault_list = NULL,
    .event_state = INFO_STATE_CALLBACK_IN_IDLE,
    .fault_state = INFO_STATE_CALLBACK_IN_IDLE,
};

static inline void default_delete_cb(dynamicbox_h handler, int ret, void *data)
{
    DbgPrint("Default deleted event handler: %d\n", ret);
}

static void del_ret_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    struct cb_info *info = data;
    int ret;
    dynamicbox_ret_cb cb;
    void *cbdata;

    cb = info->cb;
    cbdata = info->data;
    dbox_destroy_cb_info(info);

    if (!result) {
	ErrPrint("Connection lost?\n");
	ret = DBOX_STATUS_ERROR_FAULT;
    } else if (packet_get(result, "i", &ret) != 1) {
	ErrPrint("Invalid argument\n");
	ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
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
     *     handler->cbs.deleted.cb(handler, ret, handler->cbs.deleted.data);
     */
}

struct dynamicbox_common *dbox_create_common_handle(dynamicbox_h handle, const char *pkgname, const char *cluster, const char *category)
{
    struct dynamicbox_common *common;

    common = calloc(1, sizeof(*common));
    if (!common) {
	ErrPrint("Heap: %s\n", strerror(errno));
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_OUT_OF_MEMORY);
	return NULL;
    }

    common->pkgname = strdup(pkgname);
    if (!common->pkgname) {
	free(common);
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_OUT_OF_MEMORY);
	return NULL;
    }

    common->cluster = strdup(cluster);
    if (!common->cluster) {
	ErrPrint("Error: %s\n", strerror(errno));
	free(common->pkgname);
	free(common);
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_OUT_OF_MEMORY);
	return NULL;
    }

    common->category = strdup(category);
    if (!common->category) {
	ErrPrint("Error: %s\n", strerror(errno));
	free(common->cluster);
	free(common->pkgname);
	free(common);
	dynamicbox_set_last_status(DBOX_STATUS_ERROR_OUT_OF_MEMORY);
	return NULL;
    }

    /* Data provider will set this */
    common->dbox.type = DBOX_TYPE_FILE;
    common->gbar.type = GBAR_TYPE_SCRIPT;

    /* Used for handling the mouse event on a box */
    common->dbox.mouse_event = 0;

    /* Cluster infomration is not determined yet */
    common->nr_of_sizes = 0x01;

    common->timestamp = util_timestamp();
    common->is_user = 1;
    common->delete_type = DBOX_DELETE_PERMANENTLY;

    common->gbar.lock = NULL;
    common->gbar.last_extra_buffer_idx = DBOX_UNKNOWN_BUFFER;

    common->dbox.lock = NULL;
    common->dbox.last_extra_buffer_idx = DBOX_UNKNOWN_BUFFER;

    common->state = DBOX_STATE_CREATE;
    common->visible = DBOX_SHOW;

    s_info.dynamicbox_common_list = dlist_append(s_info.dynamicbox_common_list, common);
    return common;
}

int dbox_destroy_common_handle(struct dynamicbox_common *common)
{
    dlist_remove_data(s_info.dynamicbox_common_list, common);

    common->state = DBOX_STATE_DESTROYED;

    if (common->filename) {
	(void)util_unlink(common->filename);
    }

    free(common->cluster);
    free(common->category);
    free(common->id);
    free(common->pkgname);
    free(common->filename);
    free(common->dbox.auto_launch);
    free(common->alt.icon);
    free(common->alt.name);

    if (common->dbox.fb) {
	fb_destroy(common->dbox.fb);
	common->dbox.fb = NULL;
    }

    if (common->gbar.fb) {
	fb_destroy(common->gbar.fb);
	common->gbar.fb = NULL;
    }

    return 0;
}

int dbox_common_ref(struct dynamicbox_common *common, dynamicbox_h handle)
{
    common->dynamicbox_list = dlist_append(common->dynamicbox_list, handle);
    common->refcnt++;

    return common->refcnt;
}

int dbox_common_unref(struct dynamicbox_common *common, dynamicbox_h handle)
{
    int refcnt;
    dlist_remove_data(common->dynamicbox_list, handle);
    refcnt = --common->refcnt;

    return refcnt;
}

int dbox_set_group(struct dynamicbox_common *common, const char *cluster, const char *category)
{
    void *pc = NULL;
    void *ps = NULL;

    if (cluster) {
	pc = strdup(cluster);
	if (!pc) {
	    ErrPrint("Heap: %s (cluster: %s)\n", strerror(errno), cluster);
	    return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}
    }

    if (category) {
	ps = strdup(category);
	if (!ps) {
	    ErrPrint("Heap: %s (category: %s)\n", strerror(errno), category);
	    free(pc);
	    return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
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

    return DBOX_STATUS_ERROR_NONE;
}

void dbox_set_size(struct dynamicbox_common *common, int w, int h)
{
    int size_type;

    common->dbox.width = w;
    common->dbox.height = h;

    size_type = dynamicbox_service_size_type(w, h);
    if (size_type != DBOX_SIZE_TYPE_UNKNOWN) {
	common->dbox.mouse_event = dynamicbox_service_mouse_event(common->pkgname, size_type);
    }
}

void dbox_set_update_mode(struct dynamicbox_common *common, int active_mode)
{
    common->is_active_update = active_mode;
}

void dbox_set_gbarsize(struct dynamicbox_common *common, int w, int h)
{
    common->gbar.width = w;
    common->gbar.height = h;
}

void dbox_set_default_gbarsize(struct dynamicbox_common *common, int w, int h)
{
    common->gbar.default_width = w;
    common->gbar.default_height = h;
}

void dbox_invoke_fault_handler(dynamicbox_fault_type_e event, const char *pkgname, const char *file, const char *func)
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

void dbox_invoke_event_handler(dynamicbox_h handler, dynamicbox_event_type_e event)
{
    struct dlist *l;
    struct dlist *n;
    struct event_info *info;

    if (event == DBOX_EVENT_DBOX_UPDATED && handler->common->refcnt > 1) {
	if (handler->visible != DBOX_SHOW) {
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

struct dynamicbox_common *dbox_find_common_handle(const char *pkgname, const char *id)
{
    struct dlist *l;
    struct dynamicbox_common *common;

    dlist_foreach(s_info.dynamicbox_common_list, l, common) {
	if (!common->id) {
	    continue;
	}

	if (!strcmp(common->pkgname, pkgname) && !strcmp(common->id, id)) {
	    return common;
	}
    }

    return NULL;
}

struct dynamicbox_common *dbox_find_common_handle_by_timestamp(double timestamp)
{
    struct dlist *l;
    struct dynamicbox_common *common;

    dlist_foreach(s_info.dynamicbox_common_list, l, common) {
	if (common->timestamp == timestamp) {
	    return common;
	}
    }

    return NULL;
}

dynamicbox_h dbox_new_dynamicbox(const char *pkgname, const char *id, double timestamp, const char *cluster, const char *category)
{
    dynamicbox_h handler;

    handler = calloc(1, sizeof(*handler));
    if (!handler) {
	ErrPrint("Failed to create a new dynamicbox\n");
	return NULL;
    }

    handler->common = dbox_create_common_handle(handler, pkgname, cluster, category);
    if (!handler->common) {
	ErrPrint("Heap: %s\n", strerror(errno));
	free(handler);
	return NULL;
    }

    dbox_common_ref(handler->common, handler);
    dbox_set_id(handler->common, id);
    handler->common->timestamp = timestamp;
    handler->common->state = DBOX_STATE_CREATE;
    handler->visible = DBOX_SHOW;
    s_info.dynamicbox_list = dlist_append(s_info.dynamicbox_list, handler);

    return dbox_ref(handler);
}

int dbox_delete_all(void)
{
    struct dlist *l;
    struct dlist *n;
    dynamicbox_h handler;

    dlist_foreach_safe(s_info.dynamicbox_list, l, n, handler) {
	dbox_invoke_event_handler(handler, DBOX_EVENT_DELETED);
	dbox_unref(handler, 1);
    }

    return DBOX_STATUS_ERROR_NONE;
}

int dbox_set_content(struct dynamicbox_common *common, const char *content)
{
    char *pc = NULL;

    if (content) {
	pc = strdup(content);
	if (!pc) {
	    ErrPrint("heap: %s [%s]\n", strerror(errno), content);
	    return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}
    }

    free(common->content);
    common->content = pc;
    return DBOX_STATUS_ERROR_NONE;
}

int dbox_set_title(struct dynamicbox_common *common, const char *title)
{
    char *pt = NULL;

    if (title) {
	pt = strdup(title);
	if (!pt) {
	    ErrPrint("heap: %s [%s]\n", strerror(errno), title);
	    return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}
    }

    free(common->title);
    common->title = pt;
    return DBOX_STATUS_ERROR_NONE;
}

void dbox_set_size_list(struct dynamicbox_common *common, int size_list)
{
    common->dbox.size_list = size_list;
}

void dbox_set_auto_launch(struct dynamicbox_common *common, const char *auto_launch)
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

    free(common->dbox.auto_launch);
    common->dbox.auto_launch = pa;
}

void dbox_set_priority(struct dynamicbox_common *common, double priority)
{
    common->dbox.priority = priority;
}

void dbox_set_id(struct dynamicbox_common *common, const char *id)
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

void dbox_unlink_filename(struct dynamicbox_common *common)
{
    if (common->dbox.type == DBOX_TYPE_FILE || common->dbox.type == DBOX_TYPE_TEXT) {
	if (common->filename && common->filename[0] && unlink(common->filename) < 0) {
	    ErrPrint("unlink: %s (%s)\n", strerror(errno), common->filename);
	}
    }
}

void dbox_set_filename(struct dynamicbox_common *common, const char *filename)
{
    if (common->filename) {
	free(common->filename);
    }

    common->filename = strdup(filename);
    if (!common->filename) {
	ErrPrint("Heap: %s\n", strerror(errno));
    }
}

void dbox_set_alt_icon(struct dynamicbox_common *common, const char *icon)
{
    char *_icon = NULL;

    if (icon && strlen(icon)) {
	_icon = strdup(icon);
	if (!_icon) {
	    ErrPrint("Heap: %s\n", strerror(errno));
	}
    }

    free(common->alt.icon);
    common->alt.icon = _icon;
}

void dbox_set_alt_name(struct dynamicbox_common *common, const char *name)
{
    char *_name = NULL;

    if (name && strlen(name)) {
	_name = strdup(name);
	if (!_name) {
	    ErrPrint("Heap: %s\n", strerror(errno));
	}
    }

    free(common->alt.name);
    common->alt.name = _name;
}

int dbox_set_dbox_fb(struct dynamicbox_common *common, const char *filename)
{
    struct fb_info *fb;

    if (!common) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    fb = common->dbox.fb;
    if (fb && !strcmp(fb_id(fb), filename)) { /*!< BUFFER is not changed, */
	return DBOX_STATUS_ERROR_NONE;
    }

    common->dbox.fb = NULL;

    if (!filename || filename[0] == '\0') {
	if (fb) {
	    fb_destroy(fb);
	}
	return DBOX_STATUS_ERROR_NONE;
    }

    common->dbox.fb = fb_create(filename, common->dbox.width, common->dbox.height);
    if (!common->dbox.fb) {
	ErrPrint("Faield to create a FB\n");
	if (fb) {
	    fb_destroy(fb);
	}
	return DBOX_STATUS_ERROR_FAULT;
    }

    if (fb) {
	fb_destroy(fb);
    }

    return DBOX_STATUS_ERROR_NONE;
}

int dbox_set_gbar_fb(struct dynamicbox_common *common, const char *filename)
{
    struct fb_info *fb;

    if (!common || common->state != DBOX_STATE_CREATE) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    fb = common->gbar.fb;
    if (fb && !strcmp(fb_id(fb), filename)) {
	/* BUFFER is not changed, just update the content */
	return DBOX_STATUS_ERROR_EXIST;
    }
    common->gbar.fb = NULL;

    if (!filename || filename[0] == '\0') {
	if (fb) {
	    fb_destroy(fb);
	}
	return DBOX_STATUS_ERROR_NONE;
    }

    common->gbar.fb = fb_create(filename, common->gbar.width, common->gbar.height);
    if (!common->gbar.fb) {
	ErrPrint("Failed to create a FB\n");
	if (fb) {
	    fb_destroy(fb);
	}
	return DBOX_STATUS_ERROR_FAULT;
    }

    if (fb) {
	fb_destroy(fb);
    }
    return DBOX_STATUS_ERROR_NONE;
}

struct fb_info *dbox_get_dbox_fb(struct dynamicbox_common *common)
{
    return common->dbox.fb;
}

struct fb_info *dbox_get_gbar_fb(struct dynamicbox_common *common)
{
    return common->gbar.fb;
}

void dbox_set_user(struct dynamicbox_common *common, int user)
{
    common->is_user = user;
}

void dbox_set_pinup(struct dynamicbox_common *common, int pinup_supported)
{
    common->dbox.pinup_supported = pinup_supported;
}

void dbox_set_text_dbox(struct dynamicbox_common *common)
{
    common->dbox.type = DBOX_TYPE_TEXT;
}

void dbox_set_text_gbar(struct dynamicbox_common *common)
{
    common->gbar.type = GBAR_TYPE_TEXT;
}

int dbox_text_dbox(struct dynamicbox_common *common)
{
    return common->dbox.type == DBOX_TYPE_TEXT;
}

int dbox_text_gbar(struct dynamicbox_common *common)
{
    return common->gbar.type == GBAR_TYPE_TEXT;
}

void dbox_set_period(struct dynamicbox_common *common, double period)
{
    common->dbox.period = period;
}

dynamicbox_h dbox_ref(dynamicbox_h handler)
{
    if (!handler) {
	return NULL;
    }

    handler->refcnt++;
    return handler;
}

dynamicbox_h dbox_unref(dynamicbox_h handler, int destroy_common)
{
    if (!handler) {
	return NULL;
    }

    handler->refcnt--;
    if (handler->refcnt > 0) {
	return handler;
    }

    if (handler->cbs.created.cb) {
	handler->cbs.created.cb(handler, DBOX_STATUS_ERROR_FAULT, handler->cbs.created.data);
	handler->cbs.created.cb = NULL;
	handler->cbs.created.data = NULL;
    }

    if (handler->cbs.deleted.cb) {
	handler->cbs.deleted.cb(handler, DBOX_STATUS_ERROR_FAULT, handler->cbs.deleted.data);
	handler->cbs.deleted.cb = NULL;
	handler->cbs.deleted.data = NULL;
    }

    if (handler->cbs.pinup.cb) {
	handler->cbs.pinup.cb(handler, DBOX_STATUS_ERROR_FAULT, handler->cbs.pinup.data);
	handler->cbs.pinup.cb = NULL;
	handler->cbs.pinup.data = NULL;
    }

    if (handler->cbs.group_changed.cb) {
	handler->cbs.group_changed.cb(handler, DBOX_STATUS_ERROR_FAULT, handler->cbs.group_changed.data);
	handler->cbs.group_changed.cb = NULL;
	handler->cbs.group_changed.data = NULL;
    }

    if (handler->cbs.period_changed.cb) {
	handler->cbs.period_changed.cb(handler, DBOX_STATUS_ERROR_FAULT, handler->cbs.period_changed.data);
	handler->cbs.period_changed.cb = NULL;
	handler->cbs.period_changed.data = NULL;
    }

    if (handler->cbs.size_changed.cb) {
	handler->cbs.size_changed.cb(handler, DBOX_STATUS_ERROR_FAULT, handler->cbs.size_changed.data);
	handler->cbs.size_changed.cb = NULL;
	handler->cbs.size_changed.data = NULL;
    }

    if (handler->cbs.gbar_created.cb) {
	handler->cbs.gbar_created.cb(handler, DBOX_STATUS_ERROR_FAULT, handler->cbs.gbar_created.data);
	handler->cbs.gbar_created.cb = NULL;
	handler->cbs.gbar_created.data = NULL;
    }

    if (handler->cbs.gbar_destroyed.cb) {
	handler->cbs.gbar_destroyed.cb(handler, DBOX_STATUS_ERROR_FAULT, handler->cbs.gbar_destroyed.data);
	handler->cbs.gbar_destroyed.cb = NULL;
	handler->cbs.gbar_destroyed.data = NULL;
    }

    if (handler->cbs.update_mode.cb) {
	handler->cbs.update_mode.cb(handler, DBOX_STATUS_ERROR_FAULT, handler->cbs.update_mode.data);
	handler->cbs.update_mode.cb = NULL;
	handler->cbs.update_mode.data = NULL;
    }

    if (handler->cbs.access_event.cb) {
	handler->cbs.access_event.cb(handler, DBOX_ACCESS_STATUS_ERROR, handler->cbs.access_event.data);
	handler->cbs.access_event.cb = NULL;
	handler->cbs.access_event.data = NULL;
    }

    if (handler->cbs.key_event.cb) {
	handler->cbs.key_event.cb(handler, DBOX_KEY_STATUS_ERROR, handler->cbs.key_event.data);
	handler->cbs.key_event.cb = NULL;
	handler->cbs.key_event.data = NULL;
    }

    dlist_remove_data(s_info.dynamicbox_list, handler);

    handler->state = DBOX_STATE_DESTROYED;
    if (dbox_common_unref(handler->common, handler) == 0) {
	if (destroy_common) {
	    /*!
	     * \note
	     * Lock file should be deleted after all callbacks are processed.
	     */
	    (void)dynamicbox_service_destroy_lock(handler->common->dbox.lock);
	    handler->common->dbox.lock = NULL;
	    dbox_destroy_common_handle(handler->common);
	}
    }
    free(handler);
    DbgPrint("Handler is released\n");
    return NULL;
}

int dbox_send_delete(dynamicbox_h handler, int type, dynamicbox_ret_cb cb, void *data)
{
    struct packet *packet;
    struct cb_info *cbinfo;
    int ret;

    if (handler->common->request.deleted) {
	ErrPrint("Already in-progress\n");
	if (cb) {
	    cb(handler, DBOX_STATUS_ERROR_NONE, data);
	}
	return DBOX_STATUS_ERROR_BUSY;
    }

    if (!cb) {
	cb = default_delete_cb;
    }

    packet = packet_create("delete", "ssid", handler->common->pkgname, handler->common->id, type, handler->common->timestamp);
    if (!packet) {
	ErrPrint("Failed to build a param\n");
	if (cb) {
	    cb(handler, DBOX_STATUS_ERROR_FAULT, data);
	}

	return DBOX_STATUS_ERROR_FAULT;
    }

    cbinfo = dbox_create_cb_info(cb, data);
    if (!cbinfo) {
	packet_destroy(packet);
	ErrPrint("Failed to create cbinfo\n");
	if (cb) {
	    cb(handler, DBOX_STATUS_ERROR_FAULT, data);
	}

	return DBOX_STATUS_ERROR_FAULT;
    }

    ret = master_rpc_async_request(handler, packet, 0, del_ret_cb, cbinfo);
    if (ret < 0) {
	/*!
	 * Packet is destroyed by master_rpc_async_request.
	 */
	dbox_destroy_cb_info(cbinfo);

	if (cb) {
	    cb(handler, DBOX_STATUS_ERROR_FAULT, data);
	}
    } else {
	handler->common->request.deleted = 1;
    }

    return ret;
}

int dbox_sync_dbox_fb(struct dynamicbox_common *common)
{
    int ret;

    if (fb_type(dbox_get_dbox_fb(common)) == DBOX_FB_TYPE_FILE) {
	(void)dynamicbox_service_acquire_lock(common->dbox.lock);
	ret = fb_sync(dbox_get_dbox_fb(common), common->dbox.last_damage.x, common->dbox.last_damage.y, common->dbox.last_damage.w, common->dbox.last_damage.h);
	(void)dynamicbox_service_release_lock(common->dbox.lock);
    } else {
	ret = fb_sync(dbox_get_dbox_fb(common), common->dbox.last_damage.x, common->dbox.last_damage.y, common->dbox.last_damage.w, common->dbox.last_damage.h);
    }

    return ret;
}

int dbox_sync_gbar_fb(struct dynamicbox_common *common)
{
    int ret;

    if (fb_type(dbox_get_gbar_fb(common)) == DBOX_FB_TYPE_FILE) {
	(void)dynamicbox_service_acquire_lock(common->gbar.lock);
	ret = fb_sync(dbox_get_gbar_fb(common), common->gbar.last_damage.x, common->gbar.last_damage.y, common->gbar.last_damage.w, common->gbar.last_damage.h);
	(void)dynamicbox_service_release_lock(common->gbar.lock);
    } else {
	ret = fb_sync(dbox_get_gbar_fb(common), common->gbar.last_damage.x, common->gbar.last_damage.y, common->gbar.last_damage.w, common->gbar.last_damage.h);
    }

    return ret;
}

struct dynamicbox_common *dbox_find_sharable_common_handle(const char *pkgname, const char *content, int w, int h, const char *cluster, const char *category)
{
    struct dlist *l;
    struct dynamicbox_common *common;

    if (!conf_shared_content()) {
	/*!
	 * Shared content option is turnned off.
	 */
	return NULL;
    }

    dlist_foreach(s_info.dynamicbox_common_list, l, common) {
	if (common->state != DBOX_STATE_CREATE) {
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

	if (common->dbox.width != w || common->dbox.height != h) {
	    DbgPrint("Size mismatched\n");
	    continue;
	}

	DbgPrint("common handle is found: %p\n", common);
	return common;
    }

    return NULL;
}

dynamicbox_h dbox_find_dbox_in_show(struct dynamicbox_common *common)
{
    struct dlist *l;
    dynamicbox_h item;

    dlist_foreach(common->dynamicbox_list, l, item) {
	if (item->visible == DBOX_SHOW) {
	    DbgPrint("%s visibility is not changed\n", common->pkgname);
	    return item;
	}
    }

    return NULL;
}

dynamicbox_h dbox_get_dbox_nth(struct dynamicbox_common *common, int nth)
{
    dynamicbox_h item;
    struct dlist *l;

    l = dlist_nth(common->dynamicbox_list, nth);
    item = dlist_data(l);

    return item;
}

int dbox_add_event_handler(dynamicbox_event_handler_cb dbox_cb, void *data)
{
    struct event_info *info;
    info = malloc(sizeof(*info));
    if (!info) {
	ErrPrint("Heap: %s\n", strerror(errno));
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    info->handler = dbox_cb;
    info->user_data = data;
    info->is_deleted = 0;

    s_info.event_list = dlist_append(s_info.event_list, info);
    return DBOX_STATUS_ERROR_NONE;
}

void *dbox_remove_event_handler(dynamicbox_event_handler_cb dbox_cb)
{
    struct event_info *info;
    struct dlist *l;

    dlist_foreach(s_info.event_list, l, info) {
	if (info->handler == dbox_cb) {
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

int dbox_add_fault_handler(dynamicbox_fault_handler_cb dbox_cb, void *data)
{
    struct fault_info *info;
    info = malloc(sizeof(*info));
    if (!info) {
	ErrPrint("Heap: %s\n", strerror(errno));
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    info->handler = dbox_cb;
    info->user_data = data;
    info->is_deleted = 0;

    s_info.fault_list = dlist_append(s_info.fault_list, info);
    return DBOX_STATUS_ERROR_NONE;
}

void *dbox_remove_fault_handler(dynamicbox_fault_handler_cb dbox_cb)
{
    struct fault_info *info;
    struct dlist *l;

    dlist_foreach(s_info.fault_list, l, info) {
	if (info->handler == dbox_cb) {
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

struct cb_info *dbox_create_cb_info(dynamicbox_ret_cb cb, void *data)
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

void dbox_destroy_cb_info(struct cb_info *info)
{
    free(info);
}

/* End of a file */
