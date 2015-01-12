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
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>

#include <dlog.h>
#include <glib.h>

#include <vconf.h>
#include <vconf-keys.h>

#include <packet.h>
#include <com-core.h>
#include <com-core_packet.h>
#include <dynamicbox_errno.h>
#include <dynamicbox_service.h>
#include <dynamicbox_cmd_list.h>
#include <dynamicbox_buffer.h>
#include <secure_socket.h>

#include "debug.h"
#include "client.h"
#include "dynamicbox.h"
#include "dynamicbox_internal.h"
#include "desc_parser.h"
#include "fb.h"
#include "util.h"
#include "master_rpc.h"
#include "conf.h"
#include "file_service.h"
#include "dlist.h"

int errno;

#define MAX_DIRECT_ADDR 256

static struct info {
    int fd;
    int direct_fd;
    guint timer_id;
    char *client_addr;
    char *direct_addr;
} s_info = {
    .fd = -1,
    .direct_fd = -1,
    .timer_id = 0,
    .client_addr = NULL,
    .direct_addr = NULL,
};

static struct packet *master_fault_package(pid_t pid, int handle, const struct packet *packet)
{
    const char *pkgname;
    const char *id;
    const char *function;

    if (packet_get(packet, "sss", &pkgname, &id, &function) != 3) {
	ErrPrint("Invalid arguments\n");
	return NULL;
    }

    DbgPrint("[%s]\n", pkgname);
    master_rpc_clear_fault_package(pkgname);
    dbox_invoke_fault_handler(DBOX_FAULT_DEACTIVATED, pkgname, id, function);
    return NULL;
}

static struct packet *master_hold_scroll(pid_t pid, int handle, const struct packet *packet)
{
    struct dynamicbox_common *common;
    dynamicbox_h dynamicbox;
    const char *pkgname;
    const char *id;
    int seize;
    int ret;
    struct dlist *l;

    ret = packet_get(packet, "ssi", &pkgname, &id, &seize);
    if (ret != 3) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("Instance(%s) is not exists\n", id);
	goto out;
    }

    DbgPrint("HOLD: [%s] seize(%d)\n", id, seize);
    seize = seize ? DBOX_EVENT_HOLD_SCROLL : DBOX_EVENT_RELEASE_SCROLL;
    dlist_foreach(common->dynamicbox_list, l, dynamicbox) {
	dbox_invoke_event_handler(dynamicbox, seize);
    }

out:
    return NULL;
}

static struct packet *master_pinup(pid_t pid, int handle, const struct packet *packet)
{
    const char *pkgname;
    const char *id;
    const char *content;
    dynamicbox_h handler;
    struct dlist *l;
    struct dlist *n;
    struct dynamicbox_common *common;
    char *new_content;
    int ret;
    int status;
    int pinup;

    ret = packet_get(packet, "iisss", &status, &pinup, &pkgname, &id, &content);
    if (ret != 5) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("Instance (%s) is not exists\n", id);
	goto out;
    }

    if (status == (int)DBOX_STATUS_ERROR_NONE) {
	new_content = strdup(content);
	if (new_content) {
	    free(common->content);
	    common->content = new_content;
	    common->is_pinned_up = pinup;
	} else {
	    ErrPrint("Heap: %s\n", strerror(errno));
	    status = DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}
    }

    common->request.pinup = 0;

    dlist_foreach_safe(common->dynamicbox_list, l, n, handler) {
	if (handler->cbs.pinup.cb) {
	    dynamicbox_ret_cb cb;
	    void *cbdata;

	    /* Make sure that user can call pinup API in its result callback */
	    cb = handler->cbs.pinup.cb;
	    cbdata = handler->cbs.pinup.data;

	    handler->cbs.pinup.cb = NULL;
	    handler->cbs.pinup.data = NULL;

	    cb(handler, status, cbdata);
	} else if (status == (int)DBOX_STATUS_ERROR_NONE) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_PINUP_CHANGED);
	}
    }

out:
    return NULL;
}

static struct packet *master_deleted(pid_t pid, int handle, const struct packet *packet)
{
    const char *pkgname;
    const char *id;
    double timestamp;
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    struct dlist *l;
    struct dlist *n;
    int reason;

    if (packet_get(packet, "ssdi", &pkgname, &id, &timestamp, &reason) != 4) {
	ErrPrint("Invalid arguemnt\n");
	goto out;
    }

    DbgPrint("[%s]\n", pkgname);
    common = dbox_find_common_handle_by_timestamp(timestamp);
    if (!common) {
	/*!
	 * \note
	 * This can be happens only if the user delete a dynamicbox
	 * right after create it before receive created event.
	 */
	goto out;
    }

    /*!< Check validity of this "handler" */
    if (common->state != DBOX_STATE_CREATE) {
	if (common->state != DBOX_STATE_DELETE) {
	    /*!
	     * \note
	     * This is not possible
	     */
	    ErrPrint("Already deleted handler (%s - %s)\n", pkgname, id);
	    return NULL;
	}
    }

    common->request.deleted = 0;
    /*!
     * We should change the state of "common handler' before handling the callbacks.
     * Because if user tries to create a new handle in the callbacks,
     * find_sharable_common_handle will returns destroying object.
     * Then we will get panic.
     * To prevent it, we should change its state first.
     */
    common->state = DBOX_STATE_DELETE;

    dlist_foreach_safe(common->dynamicbox_list, l, n, handler) {
	if (handler->cbs.created.cb) {
	    dynamicbox_ret_cb cb;
	    void *cbdata;
	    /*!
	     * \note
	     *
	     * "if (handler->id == NULL) {"
	     *
	     * The instance is not created yet.
	     * But the master forcely destroy it and send destroyed event to this
	     * without the created event.
	     *
	     * It could be destroyed when a slave has critical error(fault)
	     * before creating an instance successfully.
	     */
	    if (handler->cbs.created.cb == handler->cbs.deleted.cb) {
		if (handler->cbs.created.data != handler->cbs.deleted.data) {
		    DbgPrint("cb is same but cbdata is different (%s - %s)\n", pkgname, id);
		}

		handler->cbs.deleted.cb = NULL;
		handler->cbs.deleted.data = NULL;
	    }

	    cb = handler->cbs.created.cb;
	    cbdata = handler->cbs.created.data;

	    handler->cbs.created.cb = NULL;
	    handler->cbs.created.data = NULL;

	    if (reason == (int)DBOX_STATUS_ERROR_NONE) {
		reason = DBOX_STATUS_ERROR_CANCEL;
	    }

	    cb(handler, reason, cbdata);
	} else if (common->id) {
	    if (handler->cbs.deleted.cb) {
		dynamicbox_ret_cb cb;
		void *cbdata;

		cb = handler->cbs.deleted.cb;
		cbdata = handler->cbs.deleted.data;

		handler->cbs.deleted.cb = NULL;
		handler->cbs.deleted.data = NULL;

		cb(handler, reason, cbdata);
	    } else {
		dbox_invoke_event_handler(handler, DBOX_EVENT_DELETED);
	    }
	}

	/* Just try to delete it, if a user didn't remove it from the live box list */
	dbox_unref(handler, 1);
    }

out:
    return NULL;
}

static struct packet *master_dbox_update_begin(pid_t pid, int handle, const struct packet *packet)
{
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    const char *pkgname;
    const char *id;
    const char *content;
    const char *title;
    const char *fbfile;
    double priority;
    int ret;

    ret = packet_get(packet, "ssdsss", &pkgname, &id, &priority, &content, &title, &fbfile);
    if (ret != 6) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("Instance[%s] is not exists\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	ErrPrint("(%s) is not created\n", id);
	goto out;
    }

    dbox_set_priority(common, priority);
    dbox_set_content(common, content);
    dbox_set_title(common, title);

    /*!
     * \NOTE
     * Width & Height is not changed in this case.
     * If the active update is began, the size should not be changed,
     * And if the size is changed, the provider should finish the updating first.
     * And then begin updating again after change its size.
     */
    if (dbox_get_dbox_fb(common)) {
	(void)dbox_set_dbox_fb(common, fbfile);

	ret = dbox_sync_dbox_fb(common);

	if (ret != (int)DBOX_STATUS_ERROR_NONE) {
	    ErrPrint("Failed to do sync FB (%s - %s) (%d)\n", pkgname, fbfile, ret);
	} else {
	    struct dlist *l;
	    dlist_foreach(common->dynamicbox_list, l, handler) {
		dbox_invoke_event_handler(handler, DBOX_EVENT_DBOX_UPDATE_BEGIN);
	    }
	}
    } else {
	ErrPrint("Invalid request[%s], %s\n", id, fbfile);
    }

out:
    return NULL;
}

static struct packet *master_gbar_update_begin(pid_t pid, int handle, const struct packet *packet)
{
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    const char *pkgname;
    const char *id;
    const char *fbfile;
    int ret;

    ret = packet_get(packet, "sss", &pkgname, &id, &fbfile);
    if (ret != 2) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("Instance[%s] is not exists\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	ErrPrint("[%s] is not created\n", id);
	goto out;
    }

    if (dbox_get_gbar_fb(common)) {
	(void)dbox_set_gbar_fb(common, fbfile);

	ret = dbox_sync_gbar_fb(common);
	if (ret != (int)DBOX_STATUS_ERROR_NONE) {
	    ErrPrint("Failed to do sync FB (%s - %s) (%d)\n", pkgname, fbfile, ret);
	} else {
	    struct dlist *l;
	    dlist_foreach(common->dynamicbox_list, l, handler) {
		dbox_invoke_event_handler(handler, DBOX_EVENT_GBAR_UPDATE_BEGIN);
	    }
	}
    } else {
	ErrPrint("Invalid request[%s], %s\n", id, fbfile);
    }

out:
    return NULL;
}

static struct packet *master_dbox_update_end(pid_t pid, int handle, const struct packet *packet)
{
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    const char *pkgname;
    const char *id;
    int ret;

    ret = packet_get(packet, "ss", &pkgname, &id);
    if (ret != 2) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("Instance[%s] is not exists\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	ErrPrint("[%s] is not created\n", id);
	goto out;
    }

    if (dbox_get_dbox_fb(common)) {
	struct dlist *l;
	dlist_foreach(common->dynamicbox_list, l, handler) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_DBOX_UPDATE_END);
	}
    } else {
	ErrPrint("Invalid request[%s]\n", id);
    }

out:
    return NULL;
}

static struct packet *master_key_status(pid_t pid, int handle, const struct packet *packet)
{
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    struct dlist *l;
    const char *pkgname;
    const char *id;
    int ret;
    int status;

    ret = packet_get(packet, "ssi", &pkgname, &id, &status);
    if (ret != 3) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("Instance[%s] is not exists\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	ErrPrint("[%s] is not created\n", id);
	goto out;
    }

    common->request.key_event = 0;
    dlist_foreach(common->dynamicbox_list, l, handler) {
	if (handler->cbs.key_event.cb) {
	    dynamicbox_ret_cb cb;
	    void *cbdata;

	    cb = handler->cbs.key_event.cb;
	    cbdata = handler->cbs.key_event.data;

	    handler->cbs.key_event.cb = NULL;
	    handler->cbs.key_event.data = NULL;

	    cb(handler, status, cbdata);
	} else {
	    ErrPrint("Invalid event[%s]\n", id);
	}
    }

out:
    return NULL;
}

static struct packet *master_request_close_gbar(pid_t pid, int handle, const struct packet *packet)
{
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    struct dlist *l;
    const char *pkgname;
    const char *id;
    int ret;
    int reason;

    ret = packet_get(packet, "ssi", &pkgname, &id, &reason);
    if (ret != 3) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("Instance[%s] is not exists\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	ErrPrint("[%s] is not created\n", id);
	goto out;
    }

    if (!common->is_gbar_created) {
	DbgPrint("GBAR is not created, closing what?(%s)\n", id);
	goto out;
    }

    DbgPrint("Reason: %d\n", reason);

    dlist_foreach(common->dynamicbox_list, l, handler) {
	dbox_invoke_event_handler(handler, DBOX_EVENT_REQUEST_CLOSE_GBAR);
    }
out:
    return NULL;
}

static struct packet *master_access_status(pid_t pid, int handle, const struct packet *packet)
{
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    struct dlist *l;
    const char *pkgname;
    const char *id;
    int ret;
    int status;

    ret = packet_get(packet, "ssi", &pkgname, &id, &status);
    if (ret != 3) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("Instance[%s] is not exists\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	ErrPrint("[%s] is not created\n", id);
	goto out;
    }

    common->request.access_event = 0;
    dlist_foreach(common->dynamicbox_list, l, handler) {
	if (handler->cbs.access_event.cb) {
	    dynamicbox_ret_cb cb;
	    void *cbdata;

	    cb = handler->cbs.access_event.cb;
	    cbdata = handler->cbs.access_event.data;

	    handler->cbs.access_event.cb = NULL;
	    handler->cbs.access_event.data = NULL;

	    cb(handler, status, cbdata);
	}
    }
out:
    return NULL;
}

static struct packet *master_gbar_update_end(pid_t pid, int handle, const struct packet *packet)
{
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    const char *pkgname;
    const char *id;
    int ret;

    ret = packet_get(packet, "ss", &pkgname, &id);
    if (ret != 2) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("Instance[%s] is not exists\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	ErrPrint("[%s] is not created\n", id);
	goto out;
    }

    if (dbox_get_dbox_fb(common)) {
	struct dlist *l;

	dlist_foreach(common->dynamicbox_list, l, handler) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_GBAR_UPDATE_END);
	}
    } else {
	ErrPrint("Invalid request[%s]", id);
    }

out:
    return NULL;
}

static struct packet *master_extra_info(pid_t pid, int handle, const struct packet *packet)
{
    const char *pkgname;
    const char *id;
    const char *content;
    const char *title;
    const char *icon;
    const char *name;
    double priority;
    int ret;
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    struct dlist *l;
    struct dlist *n;

    ret = packet_get(packet, "ssssssd", &pkgname, &id,
	    &content, &title,
	    &icon, &name,
	    &priority);
    if (ret != 7) {
	ErrPrint("Invalid parameters\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("instance(%s) is not exists\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	/*!
	 * \note
	 * Already deleted by the user.
	 * Don't try to notice anything with this, Just ignore all events
	 * Beacuse the user doesn't wants know about this anymore
	 */
	ErrPrint("(%s) is not exists, but updated\n", id);
	goto out;
    }

    dbox_set_priority(common, priority);
    dbox_set_content(common, content);
    dbox_set_title(common, title);
    dbox_set_alt_icon(common, icon);
    dbox_set_alt_name(common, name);

    dlist_foreach_safe(common->dynamicbox_list, l, n, handler) {
	dbox_invoke_event_handler(handler, DBOX_EVENT_EXTRA_INFO_UPDATED);
    }
out:
    return NULL;
}

static struct packet *master_extra_updated(pid_t pid, int handle, const struct packet *packet)
{
    const char *pkgname;
    const char *id;
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    int ret;
    int x;
    int y;
    int w;
    int h;
    int is_gbar;
    int event_type;
    int idx;

    ret = packet_get(packet, "ssiiiiii", &pkgname, &id, &is_gbar, &idx, &x, &y, &w, &h);
    if (ret != 8) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("instance(%s) is not exists\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	/*!
	 * \note
	 * Already deleted by the user.
	 * Don't try to notice anything with this, Just ignore all events
	 * Beacuse the user doesn't wants know about this anymore
	 */
	ErrPrint("(%s) is not exists, but updated\n", id);
	goto out;
    }

    if (is_gbar) {
	common->gbar.last_damage.x = x;
	common->gbar.last_damage.y = y;
	common->gbar.last_damage.w = w;
	common->gbar.last_damage.h = h;
	common->gbar.last_extra_buffer_idx = idx;

	event_type = DBOX_EVENT_GBAR_EXTRA_UPDATED;
    } else {
	common->dbox.last_damage.x = x;
	common->dbox.last_damage.y = y;
	common->dbox.last_damage.w = w;
	common->dbox.last_damage.h = h;
	common->dbox.last_extra_buffer_idx = idx;

	event_type = DBOX_EVENT_DBOX_EXTRA_UPDATED;

	if (conf_frame_drop_for_resizing() && common->request.size_changed) {
	    /* Just for skipping the update event callback call, After request to resize buffer, update event will be discarded */
	    DbgPrint("Discards obsoloted update event\n");
	    ret = DBOX_STATUS_ERROR_BUSY;
	} else {
	    if (!conf_manual_sync()) {
		ret = dbox_sync_dbox_fb(common);
		if (ret != (int)DBOX_STATUS_ERROR_NONE) {
		    ErrPrint("Failed to do sync FB (%s - %s) (%d)\n", pkgname, util_basename(util_uri_to_path(id)), ret);
		}
	    } else {
		ret = DBOX_STATUS_ERROR_NONE;
	    }
	}
    }

    if (!common->request.created && ret == (int)DBOX_STATUS_ERROR_NONE) {
	struct dlist *l;
	struct dlist *n;

	dlist_foreach_safe(common->dynamicbox_list, l, n, handler) {
	    dbox_invoke_event_handler(handler, event_type);
	}
    }

out:
    return NULL;
}

static struct packet *master_dbox_updated(pid_t pid, int handle, const struct packet *packet)
{
    const char *pkgname;
    const char *id;
    const char *fbfile;
    const char *safe_file;
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    int ret;
    int x;
    int y;
    int w;
    int h;

    ret = packet_get(packet, "ssssiiii", &pkgname, &id, &fbfile, &safe_file, &x, &y, &w, &h);
    if (ret != 8) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("instance(%s) is not exists\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	/*!
	 * \note
	 * Already deleted by the user.
	 * Don't try to notice anything with this, Just ignore all events
	 * Beacuse the user doesn't wants know about this anymore
	 */
	ErrPrint("(%s) is not exists, but updated\n", id);
	goto out;
    }

    common->dbox.last_damage.x = x;
    common->dbox.last_damage.y = y;
    common->dbox.last_damage.w = w;
    common->dbox.last_damage.h = h;
    common->dbox.last_extra_buffer_idx = DBOX_PRIMARY_BUFFER;
    dbox_set_filename(common, safe_file);

    if (dbox_text_dbox(common)) {
	const char *common_filename;

	common_filename = common->filename ? common->filename : util_uri_to_path(common->id); 

	(void)parse_desc(common, common_filename, 0);
	/*!
	 * \note
	 * DESC parser will call the "text event callback".
	 * Don't need to call global event callback in this case.
	 */
	dbox_unlink_filename(common);
	goto out;
    } else if (dbox_get_dbox_fb(common)) {
	/*!
	 * \todo
	 * replace this with "flag" instead of "callback address"
	 */
	if (conf_frame_drop_for_resizing() && common->request.size_changed) {
	    /* Just for skipping the update event callback call, After request to resize buffer, update event will be discarded */
	    DbgPrint("Discards obsoloted update event\n");
	    ret = DBOX_STATUS_ERROR_BUSY;
	} else {
	    (void)dbox_set_dbox_fb(common, fbfile);

	    if (!conf_manual_sync()) {
		ret = dbox_sync_dbox_fb(common);
		if (ret != (int)DBOX_STATUS_ERROR_NONE) {
		    ErrPrint("Failed to do sync FB (%s - %s) (%d)\n", pkgname, util_basename(util_uri_to_path(id)), ret);
		}
	    } else {
		ret = DBOX_STATUS_ERROR_NONE;
	    }
	}
    } else {
	ret = DBOX_STATUS_ERROR_NONE;
    }

    if (ret == (int)DBOX_STATUS_ERROR_NONE && !common->request.created) {
	struct dlist *l;
	struct dlist *n;

	dlist_foreach_safe(common->dynamicbox_list, l, n, handler) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_DBOX_UPDATED);
	}
    }
    dbox_unlink_filename(common);

out:
    return NULL;
}

static struct packet *master_gbar_created(pid_t pid, int handle, const struct packet *packet)
{
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    const char *pkgname;
    const char *id;
    const char *buf_id;
    struct dlist *l;
    struct dlist *n;
    int width;
    int height;
    int ret;
    int status;

    ret = packet_get(packet, "sssiii", &pkgname, &id, &buf_id, &width, &height, &status);
    if (ret != 6) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    DbgPrint("[%s]\n", pkgname);
    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("Instance(%s) is not exists\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	ErrPrint("Instance(%s) is not created\n", id);
	goto out;
    }

    if (!common->request.gbar_created) {
	ErrPrint("GBAR create request is canceled\n");
	goto out;
    }

    common->is_gbar_created = (status == (int)DBOX_STATUS_ERROR_NONE);
    common->request.gbar_created = 0;

    if (common->is_gbar_created) {
	dbox_set_gbarsize(common, width, height);
	if (dbox_text_gbar(common)) {
	    DbgPrint("Text TYPE does not need to handle this\n");
	} else {
	    (void)dbox_set_gbar_fb(common, buf_id);

	    switch (common->gbar.type) {
		case GBAR_TYPE_SCRIPT:
		case GBAR_TYPE_BUFFER:
		    switch (fb_type(dbox_get_gbar_fb(common))) {
			case DBOX_FB_TYPE_FILE:
			case DBOX_FB_TYPE_SHM:
			    common->gbar.lock = dynamicbox_service_create_lock(common->id, DBOX_TYPE_GBAR, DBOX_LOCK_READ);
			    break;
			case DBOX_FB_TYPE_PIXMAP:
			case DBOX_FB_TYPE_ERROR:
			default:
			    break;
		    }
		    break;
		case GBAR_TYPE_UIFW:
		case GBAR_TYPE_TEXT:
		default:
		    break;
	    }

	    ret = dbox_sync_gbar_fb(common);
	    if (ret < 0) {
		ErrPrint("Failed to do sync FB (%s - %s)\n", pkgname, util_basename(util_uri_to_path(id)));
	    }
	}
    }

    DbgPrint("PERF_DBOX\n");
    dlist_foreach_safe(common->dynamicbox_list, l, n, handler) {
	if (handler->cbs.gbar_created.cb) {
	    dynamicbox_ret_cb cb;
	    void *cbdata;

	    cb = handler->cbs.gbar_created.cb;
	    cbdata = handler->cbs.gbar_created.data;

	    handler->cbs.gbar_created.cb = NULL;
	    handler->cbs.gbar_created.data = NULL;

	    /*!
	     * Before call the Callback function,
	     * gbar_create_cb must be reset.
	     * Because, in the create callback, user can call create_gbar function again.
	     */
	    cb(handler, status, cbdata);
	} else if (status == (int)DBOX_STATUS_ERROR_NONE) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_GBAR_CREATED);
	} 
    }

out:
    return NULL;
}

static struct packet *master_gbar_destroyed(pid_t pid, int handle, const struct packet *packet)
{
    dynamicbox_h handler;
    struct dlist *l;
    struct dynamicbox_common *common;
    const char *pkgname;
    const char *id;
    int ret;
    int status;

    ret = packet_get(packet, "ssi", &pkgname, &id, &status);
    if (ret != 3) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    DbgPrint("[%s]\n", pkgname);
    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("Instance(%s) is not exists\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	ErrPrint("Instance(%s) is not created\n", id);
	goto out;
    }

    if (common->is_gbar_created == 0) {
	ErrPrint("GBAR is not created, event is ignored\n");
	goto out;
    }

    common->is_gbar_created = 0;
    common->request.gbar_destroyed = 0;

    dlist_foreach(common->dynamicbox_list, l, handler) {
	if (handler->cbs.gbar_destroyed.cb) {
	    dynamicbox_ret_cb cb;
	    void *cbdata;

	    cb = handler->cbs.gbar_destroyed.cb;
	    cbdata = handler->cbs.gbar_destroyed.data;

	    handler->cbs.gbar_destroyed.cb = NULL;
	    handler->cbs.gbar_destroyed.data = NULL;

	    /*!
	     * Before call the Callback function,
	     * gbar_destroyed_cb must be reset.
	     * Because, in the create callback, user can call destroy_gbar function again.
	     */
	    cb(handler, status, cbdata);
	} else if (status == (int)DBOX_STATUS_ERROR_NONE) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_GBAR_DESTROYED);
	}
    }

    /*!
     * \note
     * Lock file should be deleted after all callbacks are processed.
     */
    switch (common->gbar.type) {
	case GBAR_TYPE_SCRIPT:
	case GBAR_TYPE_BUFFER:
	    switch (fb_type(dbox_get_gbar_fb(common))) {
		case DBOX_FB_TYPE_FILE:
		case DBOX_FB_TYPE_SHM:
		    dynamicbox_service_destroy_lock(common->gbar.lock);
		    common->gbar.lock = NULL;
		    break;
		case DBOX_FB_TYPE_PIXMAP:
		case DBOX_FB_TYPE_ERROR:
		default:
		    break;
	    }
	    break;
	case GBAR_TYPE_UIFW:
	case GBAR_TYPE_TEXT:
	default:
	    break;
    }

out:
    return NULL;
}

static struct packet *master_gbar_updated(pid_t pid, int handle, const struct packet *packet)
{
    const char *pkgname;
    const char *id;
    const char *descfile;
    const char *fbfile;
    int ret;
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    struct dlist *l;
    int x;
    int y;
    int w;
    int h;

    ret = packet_get(packet, "ssssiiii", &pkgname, &id, &fbfile, &descfile, &x, &y, &w, &h);
    if (ret != 8) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("Instance(%s) is not exists\n", id);
	goto out;
    }

    common->gbar.last_damage.x = x;
    common->gbar.last_damage.y = y;
    common->gbar.last_damage.w = w;
    common->gbar.last_damage.h = h;

    if (common->state != DBOX_STATE_CREATE) {
	/*!
	 * \note
	 * This handler is already deleted by the user.
	 * So don't try to notice anything about this anymore.
	 * Just ignore all events.
	 */
	ErrPrint("Instance(%s) is not created\n", id);
	goto out;
    }

    if (dbox_text_gbar(common)) {
	(void)parse_desc(common, descfile, 1);
    } else {
	if (conf_frame_drop_for_resizing() && common->request.size_changed) {
	    /* Just for skipping the update event callback call, After request to resize buffer, update event will be discarded */
	    DbgPrint("Discards obsoloted update event\n");
	} else {
	    (void)dbox_set_gbar_fb(common, fbfile);

	    if (!conf_manual_sync()) {
		ret = dbox_sync_gbar_fb(common);
		if (ret < 0) {
		    ErrPrint("Failed to do sync FB (%s - %s), %d\n", pkgname, util_basename(util_uri_to_path(id)), ret);
		} else {
		    dlist_foreach(common->dynamicbox_list, l, handler) {
			dbox_invoke_event_handler(handler, DBOX_EVENT_GBAR_UPDATED);
		    }
		}
	    } else {
		dlist_foreach(common->dynamicbox_list, l, handler) {
		    dbox_invoke_event_handler(handler, DBOX_EVENT_GBAR_UPDATED);
		}
	    }
	}
    }

out:
    return NULL;
}

static struct packet *master_gbar_extra_buffer_destroyed(pid_t pid, int handle, const struct packet *packet)
{
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    const char *pkgname;
    struct dlist *l;
    struct dlist *n;
    const char *id;
    int pixmap;
    int idx;
    int ret;

    if (!packet) {
	ErrPrint("Invalid packet\n");
	goto out;
    }

    ret = packet_get(packet, "ssii", &pkgname, &id, &pixmap, &idx);
    if (ret != 4) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    if (idx < 0 || idx >= conf_extra_buffer_count()) {
	ErrPrint("Extra buffer count is not matched\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("DBOX(%s) is not found\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	ErrPrint("DBOX(%s) is not created yet\n", id);
	goto out;
    }

    if (!common->gbar.extra_buffer && conf_extra_buffer_count()) {
	common->gbar.extra_buffer = calloc(conf_extra_buffer_count(), sizeof(*common->gbar.extra_buffer));
	if (!common->gbar.extra_buffer) {
	    ErrPrint("DBOX(%s) calloc: %s\n", id, strerror(errno));
	}
    }

    common->gbar.last_extra_buffer_idx = idx;
    if (common->gbar.extra_buffer[idx] != pixmap) {
	DbgPrint("Extra buffer Pixmap is not matched %u <> %u\n", common->dbox.extra_buffer[idx], pixmap);
    }

    dlist_foreach_safe(common->dynamicbox_list, l, n, handler) {
	dbox_invoke_event_handler(handler, DBOX_EVENT_GBAR_EXTRA_BUFFER_DESTROYED);
    }
out:
    return NULL;
}

static struct packet *master_dbox_extra_buffer_destroyed(pid_t pid, int handle, const struct packet *packet)
{
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    const char *pkgname;
    struct dlist *l;
    struct dlist *n;
    const char *id;
    int idx;
    int pixmap;
    int ret;

    if (!packet) {
	ErrPrint("Invalid packet\n");
	goto out;
    }

    ret = packet_get(packet, "ssii", &pkgname, &id, &pixmap, &idx);
    if (ret != 4) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    if (idx < 0 || idx >= conf_extra_buffer_count()) {
	ErrPrint("Extra buffer count is not matched\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("DBOX(%s) is not found\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	ErrPrint("DBOX(%s) is not created yet\n", id);
	goto out;
    }

    if (!common->dbox.extra_buffer && conf_extra_buffer_count()) {
	common->dbox.extra_buffer = calloc(conf_extra_buffer_count(), sizeof(*common->dbox.extra_buffer));
	if (!common->dbox.extra_buffer) {
	    ErrPrint("DBOX(%s) calloc: %s\n", id, strerror(errno));
	}
    }

    common->dbox.last_extra_buffer_idx = idx;
    if (common->dbox.extra_buffer[idx] != pixmap) {
	DbgPrint("Extra buffer Pixmap is not matched %u <> %u\n", common->dbox.extra_buffer[idx], pixmap);
    }

    dlist_foreach_safe(common->dynamicbox_list, l, n, handler) {
	dbox_invoke_event_handler(handler, DBOX_EVENT_DBOX_EXTRA_BUFFER_DESTROYED);
    }
out:
    return NULL;
}

static struct packet *master_dbox_extra_buffer_created(pid_t pid, int handle, const struct packet *packet)
{
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    const char *pkgname;
    struct dlist *l;
    struct dlist *n;
    const char *id;
    int idx;
    int pixmap;
    int ret;

    if (!packet) {
	ErrPrint("Invalid packet\n");
	goto out;
    }

    ret = packet_get(packet, "ssii", &pkgname, &id, &pixmap, &idx);
    if (ret != 4) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    if (idx < 0 || idx >= conf_extra_buffer_count()) {
	ErrPrint("Extra buffer count is not matched\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("DBOX(%s) is not found\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	ErrPrint("DBOX(%s) is not created yet\n", id);
	goto out;
    }

    if (!common->dbox.extra_buffer && conf_extra_buffer_count()) {
	common->dbox.extra_buffer = calloc(conf_extra_buffer_count(), sizeof(*common->dbox.extra_buffer));
	if (!common->dbox.extra_buffer) {
	    ErrPrint("DBOX(%s) calloc: %s\n", id, strerror(errno));
	}
    }

    common->dbox.last_extra_buffer_idx = idx;
    common->dbox.extra_buffer[idx] = pixmap;

    dlist_foreach_safe(common->dynamicbox_list, l, n, handler) {
	dbox_invoke_event_handler(handler, DBOX_EVENT_DBOX_EXTRA_BUFFER_CREATED);
    }
out:
    return NULL;
}

static struct packet *master_gbar_extra_buffer_created(pid_t pid, int handle, const struct packet *packet)
{
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    const char *pkgname;
    struct dlist *l;
    struct dlist *n;
    const char *id;
    int pixmap;
    int idx;
    int ret;

    if (!packet) {
	ErrPrint("Invalid packet\n");
	goto out;
    }

    ret = packet_get(packet, "ssii", &pkgname, &id, &pixmap, &idx);
    if (ret != 4) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    if (idx < 0 || idx >= conf_extra_buffer_count()) {
	ErrPrint("Extra buffer count is not matched\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("DBOX(%s) is not found\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	ErrPrint("DBOX(%s) is not created yet\n", id);
	goto out;
    }

    if (!common->gbar.extra_buffer && conf_extra_buffer_count()) {
	common->gbar.extra_buffer = calloc(conf_extra_buffer_count(), sizeof(*common->gbar.extra_buffer));
	if (!common->gbar.extra_buffer) {
	    ErrPrint("DBOX(%s) calloc: %s\n", id, strerror(errno));
	}
    }

    common->gbar.last_extra_buffer_idx = idx;
    common->gbar.extra_buffer[idx] = pixmap;

    dlist_foreach_safe(common->dynamicbox_list, l, n, handler) {
	dbox_invoke_event_handler(handler, DBOX_EVENT_GBAR_EXTRA_BUFFER_CREATED);
    }
out:
    return NULL;
}

static struct packet *master_update_mode(pid_t pid, int handle, const struct packet *packet)
{
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    struct dlist *l;
    struct dlist *n;
    const char *pkgname;
    const char *id;
    int active_mode;
    int status;
    int ret;

    if (!packet) {
	ErrPrint("Invalid packet\n");
	goto out;
    }

    ret = packet_get(packet, "ssii", &pkgname, &id, &status, &active_mode);
    if (ret != 4) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("DBOX(%s) is not found\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	ErrPrint("DBOX(%s) is not created yet\n", id);
	goto out;
    }

    if (status == (int)DBOX_STATUS_ERROR_NONE) {
	dbox_set_update_mode(common, active_mode);
    }

    common->request.update_mode = 0;
    dlist_foreach_safe(common->dynamicbox_list, l, n, handler) {
	if (handler->cbs.update_mode.cb) {
	    dynamicbox_ret_cb cb;
	    void *cbdata;

	    cb = handler->cbs.update_mode.cb;
	    cbdata = handler->cbs.update_mode.data;

	    handler->cbs.update_mode.cb = NULL;
	    handler->cbs.update_mode.data = NULL;

	    cb(handler, status, cbdata);
	} else if (status == (int)DBOX_STATUS_ERROR_NONE) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_UPDATE_MODE_CHANGED);
	}
    }

out:
    return NULL;
}

static struct packet *master_size_changed(pid_t pid, int handle, const struct packet *packet)
{
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    const char *pkgname;
    const char *id;
    const char *fbfile;
    int status;
    int ret;
    int w;
    int h;
    int is_gbar;

    if (!packet) {
	ErrPrint("Invalid packet\n");
	goto out;
    }

    ret = packet_get(packet, "sssiiii", &pkgname, &id, &fbfile, &is_gbar, &w, &h, &status);
    if (ret != 7) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("DBOX(%s) is not found\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	ErrPrint("DBOX(%s) is not created yet\n", id);
	goto out;
    }

    common->request.size_changed = 0;
    if (is_gbar) {
	/*!
	 * \NOTE
	 * GBAR is not able to resized by the client.
	 * GBAR is only can be managed by the provider.
	 * So the GBAR has no private resized event handler.
	 * Notify it via global event handler only.
	 */
	if (status == (int)DBOX_STATUS_ERROR_NONE) {
	    struct dlist *l;

	    dbox_set_gbarsize(common, w, h);
	    dlist_foreach(common->dynamicbox_list, l, handler) {
		dbox_invoke_event_handler(handler, DBOX_EVENT_GBAR_SIZE_CHANGED);
	    }
	} else {
	    ErrPrint("This is not possible. GBAR Size is changed but the return value is not ZERO (%d)\n", status);
	}
    } else {
	struct dlist *l;
	struct dlist *n;

	if (status == (int)DBOX_STATUS_ERROR_NONE) {
	    dbox_set_size(common, w, h);

	    /*!
	     * \NOTE
	     * If there is a created DBOX FB, 
	     * Update it too.
	     */
	    if (dbox_get_dbox_fb(common)) {
		(void)dbox_set_dbox_fb(common, fbfile);

		ret = dbox_sync_dbox_fb(common);
		if (ret < 0) {
		    ErrPrint("Failed to do sync FB (%s - %s)\n", pkgname, util_basename(util_uri_to_path(id)));
		}

		/* Just update the size info only. */
	    }
	}

	/*!
	 * \NOTE
	 * I cannot believe client.
	 * So I added some log before & after call the user callback.
	 */
	dlist_foreach_safe(common->dynamicbox_list, l, n, handler) {
	    if (handler->cbs.size_changed.cb) {
		dynamicbox_ret_cb cb;
		void *cbdata;

		cb = handler->cbs.size_changed.cb;
		cbdata = handler->cbs.size_changed.data;

		handler->cbs.size_changed.cb = NULL;
		handler->cbs.size_changed.data = NULL;

		cb(handler, status, cbdata);
	    } else if (status == (int)DBOX_STATUS_ERROR_NONE) {
		dbox_invoke_event_handler(handler, DBOX_EVENT_DBOX_SIZE_CHANGED);
	    }
	}
    }

out:
    return NULL;
}

static struct packet *master_period_changed(pid_t pid, int handle, const struct packet *packet)
{
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    struct dlist *l;
    struct dlist *n;
    const char *pkgname;
    const char *id;
    int ret;
    double period;
    int status;

    ret = packet_get(packet, "idss", &status, &period, &pkgname, &id);
    if (ret != 4) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("DBOX(%s) is not found\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	ErrPrint("DBOX(%s) is not created\n", id);
	goto out;
    }

    if (status == (int)DBOX_STATUS_ERROR_NONE) {
	dbox_set_period(common, period);
    }

    common->request.period_changed = 0;

    dlist_foreach_safe(common->dynamicbox_list, l, n, handler) {
	if (handler->cbs.period_changed.cb) {
	    dynamicbox_ret_cb cb;
	    void *cbdata;

	    cb = handler->cbs.period_changed.cb;
	    cbdata = handler->cbs.period_changed.data;

	    handler->cbs.period_changed.cb = NULL;
	    handler->cbs.period_changed.data = NULL;

	    cb(handler, status, cbdata);
	} else if (status == (int)DBOX_STATUS_ERROR_NONE) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_PERIOD_CHANGED);
	}
    }

out:
    return NULL;
}

static struct packet *master_group_changed(pid_t pid, int handle, const struct packet *packet)
{
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    struct dlist *l;
    struct dlist *n;
    const char *pkgname;
    const char *id;
    int ret;
    const char *cluster;
    const char *category;
    int status;

    ret = packet_get(packet, "ssiss", &pkgname, &id, &status, &cluster, &category);
    if (ret != 5) {
	ErrPrint("Invalid argument\n");
	goto out;
    }

    common = dbox_find_common_handle(pkgname, id);
    if (!common) {
	ErrPrint("DBOX(%s) is not exists\n", id);
	goto out;
    }

    if (common->state != DBOX_STATE_CREATE) {
	/*!
	 * \note
	 * Do no access this handler,
	 * You cannot believe this handler anymore.
	 */
	ErrPrint("DBOX(%s) is not created\n", id);
	goto out;
    }

    if (status == (int)DBOX_STATUS_ERROR_NONE) {
	(void)dbox_set_group(common, cluster, category);
    }

    common->request.group_changed = 0;

    dlist_foreach_safe(common->dynamicbox_list, l, n, handler) {
	if (handler->cbs.group_changed.cb) {
	    dynamicbox_ret_cb cb;
	    void *cbdata;

	    cb = handler->cbs.group_changed.cb;
	    cbdata = handler->cbs.group_changed.data;

	    handler->cbs.group_changed.cb = NULL;
	    handler->cbs.group_changed.data = NULL;

	    cb(handler, status, cbdata);
	} else if (status == (int)DBOX_STATUS_ERROR_NONE) {
	    dbox_invoke_event_handler(handler, DBOX_EVENT_GROUP_CHANGED);
	}
    }

out:
    return NULL;
}

static struct packet *master_update_id(pid_t pid, int handle, const struct packet *packet)
{
    int ret;
    double timestamp;
    const char *id;
    struct dynamicbox_common *common;

    if (!packet) {
	ErrPrint("Invalid packet\n");
	return NULL;
    }

    ret = packet_get(packet, "ds", &timestamp, &id);
    if (ret != 2) {
	ErrPrint("Invalid paramter\n");
	return NULL;
    }

    common = dbox_find_common_handle_by_timestamp(timestamp);
    if (!common) {
	ErrPrint("Handle is not found for %d\n", timestamp);
	return NULL;
    }

    dbox_set_id(common, id);
    DbgPrint("Update ID(%s) for %lf\n", id, timestamp);
    return NULL;
}

static struct packet *master_created(pid_t pid, int handle, const struct packet *packet)
{
    dynamicbox_h handler;
    struct dynamicbox_common *common;
    struct dlist *l;

    int dbox_w;
    int dbox_h;
    int gbar_w;
    int gbar_h;
    const char *pkgname;
    const char *id;

    const char *content;
    const char *cluster;
    const char *category;
    const char *dbox_fname;
    const char *gbar_fname;
    const char *title;

    double timestamp;
    const char *auto_launch;
    double priority;
    int size_list;
    int user;
    int pinup_supported;
    dynamicbox_dbox_type_e dbox_type;
    dynamicbox_gbar_type_e gbar_type;
    double period;
    int is_pinned_up;

    int old_state = DBOX_STATE_DESTROYED;

    int ret;

    ret = packet_get(packet, "dsssiiiisssssdiiiiidsi",
	    &timestamp,
	    &pkgname, &id, &content,
	    &dbox_w, &dbox_h, &gbar_w, &gbar_h,
	    &cluster, &category, &dbox_fname, &gbar_fname,
	    &auto_launch, &priority, &size_list, &user, &pinup_supported,
	    &dbox_type, &gbar_type, &period, &title, &is_pinned_up);
    if (ret != 22) {
	ErrPrint("Invalid argument\n");
	ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
	goto out;
    }

    ErrPrint("[%lf] pkgname: %s, id: %s, content: %s, "
	    "gbar_w: %d, gbar_h: %d, dbox_w: %d, dbox_h: %d, "
	    "cluster: %s, category: %s, dbox_fname: \"%s\", gbar_fname: \"%s\", "
	    "auto_launch: %s, priority: %lf, size_list: %d, user: %d, pinup: %d, "
	    "dbox_type: %d, gbar_type: %d, period: %lf, title: [%s], is_pinned_up: %d\n",
	    timestamp, pkgname, id, content,
	    gbar_w, gbar_h, dbox_w, dbox_h,
	    cluster, category, dbox_fname, gbar_fname,
	    auto_launch, priority, size_list, user, pinup_supported,
	    dbox_type, gbar_type, period, title, is_pinned_up);

    common = dbox_find_common_handle_by_timestamp(timestamp);
    if (!common) {
	handler = dbox_new_dynamicbox(pkgname, id, timestamp, cluster, category);
	if (!handler) {
	    ErrPrint("Failed to create a new dynamicbox\n");
	    ret = DBOX_STATUS_ERROR_FAULT;
	    goto out;
	}
	common = handler->common;
	old_state = common->state;
    } else {
	if (common->state != DBOX_STATE_CREATE) {
	    if (common->state != DBOX_STATE_DELETE) {
		/*!
		 * \note
		 * This is not possible!!!
		 */
		ErrPrint("Invalid handler\n");
		ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;
		goto out;
	    }

	    /*!
	     * \note
	     * After get the delete states,
	     * call the create callback with deleted result.
	     */
	}

	old_state = common->state;

	if (common->id && common->request.created == 0) {
	    ErrPrint("Already created: timestamp[%lf] "
		    "pkgname[%s], id[%s] content[%s] "
		    "cluster[%s] category[%s] dbox_fname[%s] gbar_fname[%s]\n",
		    timestamp, pkgname, id,
		    content, cluster, category,
		    dbox_fname, gbar_fname);

	    ret = DBOX_STATUS_ERROR_ALREADY;
	    goto out;
	}

	dbox_set_id(common, id);
    }

    common->request.created = 0;
    dbox_set_size(common, dbox_w, dbox_h);
    common->dbox.type = dbox_type;
    common->is_pinned_up = is_pinned_up;

    switch (dbox_type) {
	case DBOX_TYPE_UIFW:
	case DBOX_TYPE_FILE:
	    break;
	case DBOX_TYPE_SCRIPT:
	case DBOX_TYPE_BUFFER:
	    if (!strlen(dbox_fname)) {
		break;
	    }
	    (void)dbox_set_dbox_fb(common, dbox_fname);

	    /*!
	     * \note
	     * DBOX should create the lock file from here.
	     * Even if the old_state == DBOX_STATE_DELETE,
	     * the lock file will be deleted from deleted event callback.
	     */
	    switch (fb_type(dbox_get_dbox_fb(common))) {
		case DBOX_FB_TYPE_FILE:
		case DBOX_FB_TYPE_SHM:
		    common->dbox.lock = dynamicbox_service_create_lock(common->id, DBOX_TYPE_DBOX, DBOX_LOCK_READ);
		    break;
		case DBOX_FB_TYPE_PIXMAP:
		case DBOX_FB_TYPE_ERROR:
		default:
		    break;
	    }

	    ret = dbox_sync_dbox_fb(common);
	    if (ret < 0) {
		ErrPrint("Failed to do sync FB (%s - %s)\n", pkgname, util_basename(util_uri_to_path(id)));
	    }
	    break;
	case DBOX_TYPE_TEXT:
	    dbox_set_text_dbox(common);
	    break;
	default:
	    break;
    }

    common->gbar.type = gbar_type;
    dbox_set_gbarsize(common, gbar_w, gbar_h);
    dbox_set_default_gbarsize(common, gbar_w, gbar_h);
    switch (gbar_type) {
	case GBAR_TYPE_SCRIPT:
	case GBAR_TYPE_BUFFER:
	    if (!strlen(gbar_fname)) {
		break;
	    }

	    dbox_set_gbar_fb(common, gbar_fname);

	    ret = dbox_sync_gbar_fb(common);
	    if (ret < 0) {
		ErrPrint("Failed to do sync FB (%s - %s)\n", pkgname, util_basename(util_uri_to_path(id)));
	    }

	    /*!
	     * \brief
	     * GBAR doesn't need to create the lock file from here.
	     * Just create it from GBAR_CREATED event.
	     */

	    break;
	case GBAR_TYPE_TEXT:
	    dbox_set_text_gbar(common);
	    break;
	case GBAR_TYPE_UIFW:
	default:
	    break;
    }

    dbox_set_priority(common, priority);

    dbox_set_size_list(common, size_list);
    dbox_set_group(common, cluster, category);

    dbox_set_content(common, content);
    dbox_set_title(common, title);

    dbox_set_user(common, user);

    dbox_set_auto_launch(common, auto_launch);
    dbox_set_pinup(common, pinup_supported);

    dbox_set_period(common, period);

    ret = 0;

    if (common->state == DBOX_STATE_CREATE) {
	dlist_foreach(common->dynamicbox_list, l, handler) {
	    /*!
	     * \note
	     * These callback can change the handler->state.
	     * So we have to use the "old_state" which stored state before call these callbacks
	     */

	    if (handler->cbs.created.cb) {
		dynamicbox_ret_cb cb;
		void *cbdata;

		cb = handler->cbs.created.cb;
		cbdata = handler->cbs.created.data;

		handler->cbs.created.cb = NULL;
		handler->cbs.created.data = NULL;

		cb(handler, ret, cbdata);
	    } else {
		dbox_invoke_event_handler(handler, DBOX_EVENT_CREATED);
	    }

	    /**
	     * If there is any updates before get this event,
	     * Invoke all update event forcely
	     */
	    switch (common->dbox.last_extra_buffer_idx) {
	    case DBOX_UNKNOWN_BUFFER:
		break;
	    case DBOX_PRIMARY_BUFFER:
		DbgPrint("Primary buffer updated\n");
		dbox_invoke_event_handler(handler, DBOX_EVENT_DBOX_UPDATED);
		break;
	    default:
		DbgPrint("Extra buffer updated\n");
		dbox_invoke_event_handler(handler, DBOX_EVENT_DBOX_EXTRA_UPDATED);
		break;
	    }
	}
    }

out:
    if (ret == 0 && old_state == DBOX_STATE_DELETE) {
	struct dlist *n;

	DbgPrint("Take place an unexpected case [%d]\n", common->refcnt);
	dlist_foreach_safe(common->dynamicbox_list, l, n, handler) {
	    if (handler->cbs.created.cb) {
		if (!handler->common->request.deleted) {
		    if (dbox_send_delete(handler, common->delete_type, handler->cbs.created.cb, handler->cbs.created.data) < 0) {
			/*!
			 * \note
			 * Already sent or something else happens.
			 * Callback will be called in any cases
			 */
		    }
		} else if (handler->state != DBOX_STATE_DELETE) {
		    handler->cbs.created.cb(handler, DBOX_STATUS_ERROR_CANCEL, handler->cbs.created.data);
		    dbox_unref(handler, 1);
		}
	    } else {
		dbox_invoke_event_handler(handler, DBOX_EVENT_DELETED);
		dbox_unref(handler, 1);
	    }
	}

	/*!
	 * \note
	 * handler->cbs.created.cb = NULL;
	 * handler->cbs.created.data = NULL;
	 *
	 * Do not clear this to use this from the deleted event callback.
	 * if this value is not cleared when the deleted event callback check it,
	 * it means that the created function is not called yet.
	 * Then the call the deleted event callback with DBOX_STATUS_ERROR_CANCEL errno.
	 */
    }

    return NULL;
}

static struct method s_table[] = {
    { /* DBOX_UPDATED */
	.cmd = CMD_STR_DBOX_UPDATED, /* pkgname, id, dbox_w, dbox_h, priority, ret */
	.handler = master_dbox_updated,
    },
    { /* GBAR_UPDATED */
	.cmd = CMD_STR_GBAR_UPDATED, /* pkgname, id, descfile, pd_w, pd_h, ret */
	.handler = master_gbar_updated,
    },
    { /* EXTRA_UPDATED */
	.cmd = CMD_STR_EXTRA_UPDATED,
	.handler = master_extra_updated,
    },
    { /* EXTRA_INFO */
	.cmd = CMD_STR_EXTRA_INFO,
	.handler = master_extra_info,
    },
    { /* DELETED */
	.cmd = CMD_STR_DELETED, /* pkgname, id, timestamp, ret */
	.handler = master_deleted,
    },
    { /* FAULTED */
	.cmd = CMD_STR_FAULT_PACKAGE, /* pkgname, id, function, ret */
	.handler = master_fault_package,
    },
    { /* SCROLL */
	.cmd = CMD_STR_SCROLL,
	.handler = master_hold_scroll,
    },
    { /* DBOX_UPDATE_BEGIN */
	.cmd = CMD_STR_DBOX_UPDATE_BEGIN,
	.handler = master_dbox_update_begin,
    },
    { /* DBOX_UPDATE_END */
	.cmd = CMD_STR_DBOX_UPDATE_END,
	.handler = master_dbox_update_end,
    },
    { /* GBAR_UPDATE_BEGIN */
	.cmd = CMD_STR_GBAR_UPDATE_BEGIN,
	.handler = master_gbar_update_begin,
    },
    { /* GBAR_UPDATE_END */
	.cmd = CMD_STR_GBAR_UPDATE_END,
	.handler = master_gbar_update_end,
    },
    { /* ACCESS_STATUS */
	.cmd = CMD_STR_ACCESS_STATUS,
	.handler = master_access_status,
    },
    { /* KEY_STATUS */
	.cmd = CMD_STR_KEY_STATUS,
	.handler = master_key_status,
    },
    { /* CLOSE_GBAR */
	.cmd = CMD_STR_CLOSE_GBAR,
	.handler = master_request_close_gbar,
    },
    { /* GBAR_CREATED */
	.cmd = CMD_STR_GBAR_CREATED,
	.handler = master_gbar_created,
    },
    { /* GBAR_DESTROYED */
	.cmd = CMD_STR_GBAR_DESTROYED,
	.handler = master_gbar_destroyed,
    },
    { /* CREATED */
	.cmd = CMD_STR_CREATED,
	.handler = master_created,
    },
    { /* GROUP_CHANGED */
	.cmd = CMD_STR_GROUP_CHANGED,
	.handler = master_group_changed,
    },
    { /* PERIOD_CHANGED */
	.cmd = CMD_STR_PERIOD_CHANGED,
	.handler = master_period_changed,
    },
    { /* SIZE_CHANGED */
	.cmd = CMD_STR_SIZE_CHANGED,
	.handler = master_size_changed,
    },
    { /* PINUP */
	.cmd = CMD_STR_PINUP,
	.handler = master_pinup,
    },
    { /* UPDATE_MODE */
	.cmd = CMD_STR_UPDATE_MODE,
	.handler = master_update_mode,
    },
    { /* DBOX_CREATE_XBUF */
	.cmd = CMD_STR_DBOX_CREATE_XBUF,
	.handler = master_dbox_extra_buffer_created,
    },
    { /* GBAR_CREATE_XBUF */
	.cmd = CMD_STR_GBAR_CREATE_XBUF,
	.handler = master_gbar_extra_buffer_created,
    },
    { /* DBOX_DESTROY_XBUF */
	.cmd = CMD_STR_DBOX_DESTROY_XBUF,
	.handler = master_dbox_extra_buffer_destroyed,
    },
    { /* GBAR_DESTROY_XBUF */
	.cmd = CMD_STR_GBAR_DESTROY_XBUF,
	.handler = master_gbar_extra_buffer_destroyed,
    },
    { /* UPDATE_ID */
	.cmd = CMD_STR_UPDATE_ID,
	.handler = master_update_id,
    },
    {
	.cmd = NULL,
	.handler = NULL,
    },
};

static void acquire_cb(dynamicbox_h handler, const struct packet *result, void *data)
{
    if (!result) {
	DbgPrint("Result packet is not valid\n");
    } else {
	int ret;
	int extra_buffer_count;

	if (packet_get(result, "ii", &ret, &extra_buffer_count) != 2) {
	    ErrPrint("Invalid argument\n");
	} else {
	    DbgPrint("Acquire returns: %d (%d)\n", ret, extra_buffer_count);
	    conf_set_extra_buffer_count(extra_buffer_count);
	}
    }

    return;
}

static inline int make_connection(void)
{
    struct packet *packet;
    unsigned int cmd = CMD_ACQUIRE;
    int ret;

    DbgPrint("Let's making connection!\n");

    s_info.fd = com_core_packet_client_init(client_addr(), 0, s_table);
    if (s_info.fd < 0) {
	ErrPrint("Try this again later\n");
	return DBOX_STATUS_ERROR_IO_ERROR;
    }

    packet = packet_create((const char *)&cmd, "ds", util_timestamp(), client_direct_addr());
    if (!packet) {
	com_core_packet_client_fini(s_info.fd);
	s_info.fd = -1;
	return DBOX_STATUS_ERROR_FAULT;
    }

    ret = master_rpc_async_request(NULL, packet, 1, acquire_cb, NULL);
    if (ret < 0) {
	ErrPrint("Master RPC returns %d\n", ret);
	com_core_packet_client_fini(s_info.fd);
	s_info.fd = -1;
	return DBOX_STATUS_ERROR_IO_ERROR;
    }

    return DBOX_STATUS_ERROR_NONE;
}

static int connected_cb(int handle, void *data)
{
    master_rpc_check_and_fire_consumer();
    return 0;
}

static void master_started_cb(keynode_t *node, void *data)
{
    int state = 0;

    if (vconf_get_bool(VCONFKEY_MASTER_STARTED, &state) < 0) {
	ErrPrint("Unable to get [%s]\n", VCONFKEY_MASTER_STARTED);
    }

    DbgPrint("Master state: %d\n", state);
    if (state == 1 && make_connection() == (int)DBOX_STATUS_ERROR_NONE) {
	int ret;
	ret = vconf_ignore_key_changed(VCONFKEY_MASTER_STARTED, master_started_cb);
	if (ret < 0) {
	    DbgPrint("master_started vconf key de-registered [%d]\n", ret);
	}
    }
}

static gboolean timeout_cb(gpointer data)
{
    if (vconf_notify_key_changed(VCONFKEY_MASTER_STARTED, master_started_cb, NULL) < 0) {
	ErrPrint("Failed to add vconf for monitoring service state\n");
    } else {
	DbgPrint("vconf event callback is registered\n");
    }

    master_started_cb(NULL, NULL);

    s_info.timer_id = 0;
    return FALSE;
}

static int disconnected_cb(int handle, void *data)
{
    if (s_info.fd != handle) {
	/*!< This handle is not my favor */
	return 0;
    }

    s_info.fd = -1; /*!< Disconnected */

    master_rpc_clear_all_request();
    dbox_invoke_fault_handler(DBOX_FAULT_PROVIDER_DISCONNECTED, MASTER_PKGNAME, "default", "disconnected");

    dbox_delete_all();

    /* Try to reconnect after 1 sec later */
    if (!s_info.timer_id) {
	DbgPrint("Reconnecting timer is added\n");
	s_info.timer_id = g_timeout_add(1000, timeout_cb, NULL);
	if (s_info.timer_id == 0) {
	    ErrPrint("Unable to add reconnecting timer\n");
	    return 0;
	}
    } else {
	ErrPrint("Reconnecting timer is already exists\n");
    }

    return 0;
}

static struct method s_direct_table[] = {
    {
	.cmd = CMD_STR_DBOX_UPDATED, /* pkgname, id, lb_w, lb_h, priority, ret */
	.handler = master_dbox_updated,
    },
    {
	.cmd = CMD_STR_GBAR_UPDATED, /* pkgname, id, descfile, pd_w, pd_h, ret */
	.handler = master_gbar_updated,
    },
    {
	.cmd = CMD_STR_EXTRA_UPDATED,
	.handler = master_extra_updated,
    },
    {
	.cmd = NULL,
	.handler = NULL,
    },
};

static void prepare_direct_update(void)
{
    char path[MAX_DIRECT_ADDR];

    if (!conf_direct_update()) {
	return;
    }

    if (!strncmp(s_info.client_addr, COM_CORE_REMOTE_SCHEME, strlen(COM_CORE_REMOTE_SCHEME))) {
	ErrPrint("Remote model is not support this\n");
	return;
    }

    snprintf(path, sizeof(path) - 1, "/tmp/.%d.%lf.dbox.viewer", getpid(), util_timestamp());

    s_info.direct_addr = strdup(path);
    if (!s_info.direct_addr) {
	ErrPrint("strdup: %s\n", strerror(errno));
	return;
    }

    s_info.direct_fd = com_core_packet_server_init(client_direct_addr(), s_direct_table);
    if (s_info.direct_fd < 0) {
	ErrPrint("Failed to prepare server: %s\n", client_direct_addr());
	return;
    }

    DbgPrint("Direct update is prepared: %s - %d\n", client_direct_addr(), client_direct_fd());
}

int client_init(int use_thread)
{
    com_core_packet_use_thread(use_thread);

    s_info.client_addr = vconf_get_str(VCONFKEY_MASTER_CLIENT_ADDR);
    if (!s_info.client_addr) {
	s_info.client_addr = strdup(CLIENT_SOCKET);
	if (!s_info.client_addr) {
	    ErrPrint("Heap: %s\n", strerror(errno));
	    return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}
    }

    (void)file_service_init();

    DbgPrint("Server Address: %s\n", s_info.client_addr);

    com_core_add_event_callback(CONNECTOR_DISCONNECTED, disconnected_cb, NULL);
    com_core_add_event_callback(CONNECTOR_CONNECTED, connected_cb, NULL);

    /**
     * @note
     * Before creating a connection with master,
     * Initiate the private channel for getting the updated event from providers
     * How could we propagates the our address to every providers?
     */
    prepare_direct_update();

    if (vconf_notify_key_changed(VCONFKEY_MASTER_STARTED, master_started_cb, NULL) < 0) {
	ErrPrint("Failed to add vconf for service state\n");
    } else {
	DbgPrint("vconf event callback is registered\n");
    }

    master_started_cb(NULL, NULL);
    return DBOX_STATUS_ERROR_NONE;
}

int client_fd(void)
{
    return s_info.fd;
}

const char *client_addr(void)
{
    return s_info.client_addr;
}

const char *client_direct_addr(void)
{
    return s_info.direct_addr;
}

int client_direct_fd(void)
{
    return s_info.direct_fd;
}

int client_fini(void)
{
    int ret;

    (void)file_service_fini();

    ret = vconf_ignore_key_changed(VCONFKEY_MASTER_STARTED, master_started_cb);
    if (ret < 0) {
	DbgPrint("Ignore vconf key: %d\n", ret);
    }

    com_core_del_event_callback(CONNECTOR_DISCONNECTED, disconnected_cb, NULL);
    com_core_del_event_callback(CONNECTOR_CONNECTED, connected_cb, NULL);
    com_core_packet_client_fini(s_info.fd);
    s_info.fd = -1;

    if (s_info.direct_fd >= 0) {
	com_core_packet_server_fini(s_info.direct_fd);
	s_info.direct_fd = -1;
    }

    free(s_info.client_addr);
    s_info.client_addr = NULL;

    free(s_info.direct_addr);
    s_info.direct_addr = NULL;
    return DBOX_STATUS_ERROR_NONE;
}

/* End of a file */
