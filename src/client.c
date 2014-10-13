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
#include <livebox-errno.h>
#include <livebox-service.h>
#include <secure_socket.h>

#include "debug.h"
#include "client.h"
#include "livebox.h"
#include "livebox_internal.h"
#include "desc_parser.h"
#include "fb.h"
#include "util.h"
#include "master_rpc.h"
#include "conf.h"
#include "file_service.h"
#include "dlist.h"
#include "provider_cmd_list.h"

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
	lb_invoke_fault_handler(LB_FAULT_DEACTIVATED, pkgname, id, function);
	return NULL;
}

static struct packet *master_hold_scroll(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox_common *common;
	struct livebox *livebox;
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

	common = lb_find_common_handle(pkgname, id);
	if (!common) {
		ErrPrint("Instance(%s) is not exists\n", id);
		goto out;
	}

	DbgPrint("HOLD: [%s] seize(%d)\n", id, seize);
	seize = seize ? LB_EVENT_HOLD_SCROLL : LB_EVENT_RELEASE_SCROLL;
	dlist_foreach(common->livebox_list, l, livebox) {
		lb_invoke_event_handler(livebox, seize);
	}

out:
	return NULL;
}

static struct packet *master_pinup(pid_t pid, int handle, const struct packet *packet)
{
	const char *pkgname;
	const char *id;
	const char *content;
	struct livebox *handler;
	struct dlist *l;
	struct dlist *n;
	struct livebox_common *common;
	char *new_content;
	int ret;
	int status;
	int pinup;

	ret = packet_get(packet, "iisss", &status, &pinup, &pkgname, &id, &content);
	if (ret != 5) {
		ErrPrint("Invalid argument\n");
		goto out;
	}

	common = lb_find_common_handle(pkgname, id);
	if (!common) {
		ErrPrint("Instance (%s) is not exists\n", id);
		goto out;
	}

	if (status == (int)LB_STATUS_SUCCESS) {
		new_content = strdup(content);
		if (new_content) {
			free(common->content);
			common->content = new_content;
			common->is_pinned_up = pinup;
		} else {
			ErrPrint("Heap: %s\n", strerror(errno));
			status = LB_STATUS_ERROR_MEMORY;
		}
	}

	common->request.pinup = 0;

	dlist_foreach_safe(common->livebox_list, l, n, handler) {
		if (handler->cbs.pinup.cb) {
			ret_cb_t cb;
			void *cbdata;

			/* Make sure that user can call pinup API in its result callback */
			cb = handler->cbs.pinup.cb;
			cbdata = handler->cbs.pinup.data;

			handler->cbs.pinup.cb = NULL;
			handler->cbs.pinup.data = NULL;

			cb(handler, status, cbdata);
		} else if (status == (int)LB_STATUS_SUCCESS) {
			lb_invoke_event_handler(handler, LB_EVENT_PINUP_CHANGED);
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
	struct livebox *handler;
	struct livebox_common *common;
	struct dlist *l;
	struct dlist *n;
	int reason;

	if (packet_get(packet, "ssdi", &pkgname, &id, &timestamp, &reason) != 4) {
		ErrPrint("Invalid arguemnt\n");
		goto out;
	}

	DbgPrint("[%s]\n", pkgname);
	common = lb_find_common_handle_by_timestamp(timestamp);
	if (!common) {
		/*!
		 * \note
		 * This can be happens only if the user delete a livebox
		 * right after create it before receive created event.
		 */
		goto out;
	}

	/*!< Check validity of this "handler" */
	if (common->state != CREATE) {
		if (common->state != DELETE) {
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
	common->state = DELETE;

	dlist_foreach_safe(common->livebox_list, l, n, handler) {
		if (handler->cbs.created.cb) {
			ret_cb_t cb;
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

			if (reason == (int)LB_STATUS_SUCCESS) {
				reason = LB_STATUS_ERROR_CANCEL;
			}

			cb(handler, reason, cbdata);
		} else if (common->id) {
			if (handler->cbs.deleted.cb) {
				ret_cb_t cb;
				void *cbdata;

				cb = handler->cbs.deleted.cb;
				cbdata = handler->cbs.deleted.data;

				handler->cbs.deleted.cb = NULL;
				handler->cbs.deleted.data = NULL;

				cb(handler, reason, cbdata);
			} else {
				lb_invoke_event_handler(handler, LB_EVENT_DELETED);
			}
		}

		/* Just try to delete it, if a user didn't remove it from the live box list */
		lb_unref(handler, 1);
	}

out:
	return NULL;
}

static struct packet *master_lb_update_begin(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	struct livebox_common *common;
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

	common = lb_find_common_handle(pkgname, id);
	if (!common) {
		ErrPrint("Instance[%s] is not exists\n", id);
		goto out;
	}

	if (common->state != CREATE) {
		ErrPrint("(%s) is not created\n", id);
		goto out;
	}

	lb_set_priority(common, priority);
	lb_set_content(common, content);
	lb_set_title(common, title);

	/*!
	 * \NOTE
	 * Width & Height is not changed in this case.
	 * If the active update is began, the size should not be changed,
	 * And if the size is changed, the provider should finish the updating first.
	 * And then begin updating again after change its size.
	 */
	if (lb_get_lb_fb(common)) {
		(void)lb_set_lb_fb(common, fbfile);

		ret = lb_sync_lb_fb(common);

		if (ret != (int)LB_STATUS_SUCCESS) {
			ErrPrint("Failed to do sync FB (%s - %s) (%d)\n", pkgname, fbfile, ret);
		} else {
			struct dlist *l;
			dlist_foreach(common->livebox_list, l, handler) {
				lb_invoke_event_handler(handler, LB_EVENT_LB_UPDATE_BEGIN);
			}
		}
	} else {
		ErrPrint("Invalid request[%s], %s\n", id, fbfile);
	}

out:
	return NULL;
}

static struct packet *master_pd_update_begin(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	struct livebox_common *common;
	const char *pkgname;
	const char *id;
	const char *fbfile;
	int ret;

	ret = packet_get(packet, "sss", &pkgname, &id, &fbfile);
	if (ret != 2) {
		ErrPrint("Invalid argument\n");
		goto out;
	}

	common = lb_find_common_handle(pkgname, id);
	if (!common) {
		ErrPrint("Instance[%s] is not exists\n", id);
		goto out;
	}

	if (common->state != CREATE) {
		ErrPrint("[%s] is not created\n", id);
		goto out;
	}

	if (lb_get_pd_fb(common)) {
		(void)lb_set_pd_fb(common, fbfile);

		ret = lb_sync_pd_fb(common);
		if (ret != (int)LB_STATUS_SUCCESS) {
			ErrPrint("Failed to do sync FB (%s - %s) (%d)\n", pkgname, fbfile, ret);
		} else {
			struct dlist *l;
			dlist_foreach(common->livebox_list, l, handler) {
				lb_invoke_event_handler(handler, LB_EVENT_PD_UPDATE_BEGIN);
			}
		}
	} else {
		ErrPrint("Invalid request[%s], %s\n", id, fbfile);
	}

out:
	return NULL;
}

static struct packet *master_lb_update_end(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	struct livebox_common *common;
	const char *pkgname;
	const char *id;
	int ret;

	ret = packet_get(packet, "ss", &pkgname, &id);
	if (ret != 2) {
		ErrPrint("Invalid argument\n");
		goto out;
	}

	common = lb_find_common_handle(pkgname, id);
	if (!common) {
		ErrPrint("Instance[%s] is not exists\n", id);
		goto out;
	}

	if (common->state != CREATE) {
		ErrPrint("[%s] is not created\n", id);
		goto out;
	}

	if (lb_get_lb_fb(common)) {
		struct dlist *l;
		dlist_foreach(common->livebox_list, l, handler) {
			lb_invoke_event_handler(handler, LB_EVENT_LB_UPDATE_END);
		}
	} else {
		ErrPrint("Invalid request[%s]\n", id);
	}

out:
	return NULL;
}

static struct packet *master_key_status(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	struct livebox_common *common;
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

	common = lb_find_common_handle(pkgname, id);
	if (!common) {
		ErrPrint("Instance[%s] is not exists\n", id);
		goto out;
	}

	if (common->state != CREATE) {
		ErrPrint("[%s] is not created\n", id);
		goto out;
	}

	common->request.key_event = 0;
	dlist_foreach(common->livebox_list, l, handler) {
		if (handler->cbs.key_event.cb) {
			ret_cb_t cb;
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

static struct packet *master_request_close_pd(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	struct livebox_common *common;
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

	common = lb_find_common_handle(pkgname, id);
	if (!common) {
		ErrPrint("Instance[%s] is not exists\n", id);
		goto out;
	}

	if (common->state != CREATE) {
		ErrPrint("[%s] is not created\n", id);
		goto out;
	}

	if (!common->is_pd_created) {
		DbgPrint("PD is not created, closing what?(%s)\n", id);
		goto out;
	}

	DbgPrint("Reason: %d\n", reason);

	dlist_foreach(common->livebox_list, l, handler) {
		lb_invoke_event_handler(handler, LB_EVENT_REQUEST_CLOSE_PD);
	}
out:
	return NULL;
}

static struct packet *master_access_status(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	struct livebox_common *common;
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

	common = lb_find_common_handle(pkgname, id);
	if (!common) {
		ErrPrint("Instance[%s] is not exists\n", id);
		goto out;
	}

	if (common->state != CREATE) {
		ErrPrint("[%s] is not created\n", id);
		goto out;
	}

	common->request.access_event = 0;
	dlist_foreach(common->livebox_list, l, handler) {
		if (handler->cbs.access_event.cb) {
			ret_cb_t cb;
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

static struct packet *master_pd_update_end(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	struct livebox_common *common;
	const char *pkgname;
	const char *id;
	int ret;

	ret = packet_get(packet, "ss", &pkgname, &id);
	if (ret != 2) {
		ErrPrint("Invalid argument\n");
		goto out;
	}

	common = lb_find_common_handle(pkgname, id);
	if (!common) {
		ErrPrint("Instance[%s] is not exists\n", id);
		goto out;
	}

	if (common->state != CREATE) {
		ErrPrint("[%s] is not created\n", id);
		goto out;
	}

	if (lb_get_lb_fb(common)) {
		struct dlist *l;

		dlist_foreach(common->livebox_list, l, handler) {
			lb_invoke_event_handler(handler, LB_EVENT_PD_UPDATE_END);
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
	struct livebox *handler;
	struct livebox_common *common;
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

	common = lb_find_common_handle(pkgname, id);
	if (!common) {
		ErrPrint("instance(%s) is not exists\n", id);
		goto out;
	}

	if (common->state != CREATE) {
		/*!
		 * \note
		 * Already deleted by the user.
		 * Don't try to notice anything with this, Just ignore all events
		 * Beacuse the user doesn't wants know about this anymore
		 */
		ErrPrint("(%s) is not exists, but updated\n", id);
		goto out;
	}

	lb_set_priority(common, priority);
	lb_set_content(common, content);
	lb_set_title(common, title);
	lb_set_alt_icon(common, icon);
	lb_set_alt_name(common, name);

	dlist_foreach_safe(common->livebox_list, l, n, handler) {
		lb_invoke_event_handler(handler, LB_EVENT_EXTRA_INFO_UPDATED);
	}
out:
	return NULL;
}

static struct packet *master_lb_updated(pid_t pid, int handle, const struct packet *packet)
{
	const char *pkgname;
	const char *id;
	const char *fbfile;
	const char *safe_file;
	struct livebox *handler;
	struct livebox_common *common;
	int lb_w;
	int lb_h;
	int ret;

	ret = packet_get(packet, "ssssii", &pkgname, &id, &fbfile, &safe_file, &lb_w, &lb_h);
	if (ret != 6) {
		ErrPrint("Invalid argument\n");
		goto out;
	}

	common = lb_find_common_handle(pkgname, id);
	if (!common) {
		ErrPrint("instance(%s) is not exists\n", id);
		goto out;
	}

	if (common->state != CREATE) {
		/*!
		 * \note
		 * Already deleted by the user.
		 * Don't try to notice anything with this, Just ignore all events
		 * Beacuse the user doesn't wants know about this anymore
		 */
		ErrPrint("(%s) is not exists, but updated\n", id);
		goto out;
	}

	lb_set_size(common, lb_w, lb_h);
	lb_set_filename(common, safe_file);

	if (lb_text_lb(common)) {
		const char *common_filename;

		common_filename = common->filename ? common->filename : util_uri_to_path(common->id); 

		(void)parse_desc(common, common_filename, 0);
		/*!
		 * \note
		 * DESC parser will call the "text event callback".
		 * Don't need to call global event callback in this case.
		 */
		goto out;
	} else if (lb_get_lb_fb(common)) {
		/*!
		 * \todo
		 * replace this with "flag" instead of "callback address"
		 */
		if (conf_frame_drop_for_resizing() && common->request.size_changed) {
			/* Just for skipping the update event callback call, After request to resize buffer, update event will be discarded */
			DbgPrint("Discards obsoloted update event\n");
			ret = LB_STATUS_ERROR_BUSY;
		} else {
			(void)lb_set_lb_fb(common, fbfile);

			if (!conf_manual_sync()) {
				ret = lb_sync_lb_fb(common);
				if (ret != (int)LB_STATUS_SUCCESS) {
					ErrPrint("Failed to do sync FB (%s - %s) (%d)\n", pkgname, util_basename(util_uri_to_path(id)), ret);
				}
			} else {
				ret = LB_STATUS_SUCCESS;
			}
		}
	} else {
		ret = LB_STATUS_SUCCESS;
	}

	if (ret == (int)LB_STATUS_SUCCESS) {
		struct dlist *l;
		struct dlist *n;

		dlist_foreach_safe(common->livebox_list, l, n, handler) {
			lb_invoke_event_handler(handler, LB_EVENT_LB_UPDATED);
		}
	}

out:
	return NULL;
}

static struct packet *master_pd_created(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	struct livebox_common *common;
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
	common = lb_find_common_handle(pkgname, id);
	if (!common) {
		ErrPrint("Instance(%s) is not exists\n", id);
		goto out;
	}

	if (common->state != CREATE) {
		ErrPrint("Instance(%s) is not created\n", id);
		goto out;
	}

	if (!common->request.pd_created) {
		ErrPrint("PD create request is canceled\n");
		goto out;
	}

	common->is_pd_created = (status == (int)LB_STATUS_SUCCESS);
	common->request.pd_created = 0;

	if (common->is_pd_created) {
		lb_set_pdsize(common, width, height);
		if (lb_text_pd(common)) {
			DbgPrint("Text TYPE does not need to handle this\n");
		} else {
			(void)lb_set_pd_fb(common, buf_id);

			switch (common->pd.type) {
			case _PD_TYPE_SCRIPT:
			case _PD_TYPE_BUFFER:
				switch (fb_type(lb_get_pd_fb(common))) {
				case BUFFER_TYPE_FILE:
				case BUFFER_TYPE_SHM:
					lb_create_lock_file(common, 1);
					break;
				case BUFFER_TYPE_PIXMAP:
				case BUFFER_TYPE_ERROR:
				default:
					break;
				}
				break;
			case _PD_TYPE_ELEMENTARY:
			case _PD_TYPE_TEXT:
			default:
				break;
			}

			ret = lb_sync_pd_fb(common);
			if (ret < 0) {
				ErrPrint("Failed to do sync FB (%s - %s)\n", pkgname, util_basename(util_uri_to_path(id)));
			}
		}
	}

	DbgPrint("PERF_DBOX\n");
	dlist_foreach_safe(common->livebox_list, l, n, handler) {
		if (handler->cbs.pd_created.cb) {
			ret_cb_t cb;
			void *cbdata;

			cb = handler->cbs.pd_created.cb;
			cbdata = handler->cbs.pd_created.data;

			handler->cbs.pd_created.cb = NULL;
			handler->cbs.pd_created.data = NULL;

			/*!
			 * Before call the Callback function,
			 * pd_create_cb must be reset.
			 * Because, in the create callback, user can call create_pd function again.
			 */
			cb(handler, status, cbdata);
		} else if (status == (int)LB_STATUS_SUCCESS) {
			lb_invoke_event_handler(handler, LB_EVENT_PD_CREATED);
		} 
	}

out:
	return NULL;
}

static struct packet *master_pd_destroyed(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	struct dlist *l;
	struct livebox_common *common;
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
	common = lb_find_common_handle(pkgname, id);
	if (!common) {
		ErrPrint("Instance(%s) is not exists\n", id);
		goto out;
	}

	if (common->state != CREATE) {
		ErrPrint("Instance(%s) is not created\n", id);
		goto out;
	}

	if (common->is_pd_created == 0) {
		ErrPrint("PD is not created, event is ignored\n");
		goto out;
	}

	common->is_pd_created = 0;
	common->request.pd_destroyed = 0;

	dlist_foreach(common->livebox_list, l, handler) {
		if (handler->cbs.pd_destroyed.cb) {
			ret_cb_t cb;
			void *cbdata;

			cb = handler->cbs.pd_destroyed.cb;
			cbdata = handler->cbs.pd_destroyed.data;

			handler->cbs.pd_destroyed.cb = NULL;
			handler->cbs.pd_destroyed.data = NULL;

			/*!
			 * Before call the Callback function,
			 * pd_destroyed_cb must be reset.
			 * Because, in the create callback, user can call destroy_pd function again.
			 */
			cb(handler, status, cbdata);
		} else if (status == (int)LB_STATUS_SUCCESS) {
			lb_invoke_event_handler(handler, LB_EVENT_PD_DESTROYED);
		}
	}

	/*!
	 * \note
	 * Lock file should be deleted after all callbacks are processed.
	 */
	switch (common->pd.type) {
	case _PD_TYPE_SCRIPT:
	case _PD_TYPE_BUFFER:
		switch (fb_type(lb_get_pd_fb(common))) {
		case BUFFER_TYPE_FILE:
		case BUFFER_TYPE_SHM:
			lb_destroy_lock_file(common, 1);
			break;
		case BUFFER_TYPE_PIXMAP:
		case BUFFER_TYPE_ERROR:
		default:
			break;
		}
		break;
	case _PD_TYPE_ELEMENTARY:
	case _PD_TYPE_TEXT:
	default:
		break;
	}

out:
	return NULL;
}

static struct packet *master_pd_updated(pid_t pid, int handle, const struct packet *packet)
{
	const char *pkgname;
	const char *id;
	const char *descfile;
	const char *fbfile;
	int ret;
	struct livebox *handler;
	struct livebox_common *common;
	struct dlist *l;
	int pd_w;
	int pd_h;

	ret = packet_get(packet, "ssssii",
				&pkgname, &id,
				&descfile, &fbfile,
				&pd_w, &pd_h);
	if (ret != 6) {
		ErrPrint("Invalid argument\n");
		goto out;
	}

	DbgPrint("[%s]\n", pkgname);
	common = lb_find_common_handle(pkgname, id);
	if (!common) {
		ErrPrint("Instance(%s) is not exists\n", id);
		goto out;
	}

	if (common->state != CREATE) {
		/*!
		 * \note
		 * This handler is already deleted by the user.
		 * So don't try to notice anything about this anymore.
		 * Just ignore all events.
		 */
		ErrPrint("Instance(%s) is not created\n", id);
		goto out;
	}

	lb_set_pdsize(common, pd_w, pd_h);

	if (lb_text_pd(common)) {
		(void)parse_desc(common, descfile, 1);
	} else {
		if (conf_frame_drop_for_resizing() && common->request.size_changed) {
			/* Just for skipping the update event callback call, After request to resize buffer, update event will be discarded */
			DbgPrint("Discards obsoloted update event\n");
		} else {
			(void)lb_set_pd_fb(common, fbfile);

			 if (!conf_manual_sync()) {
				ret = lb_sync_pd_fb(common);
				if (ret < 0) {
					ErrPrint("Failed to do sync FB (%s - %s), %d\n", pkgname, util_basename(util_uri_to_path(id)), ret);
				} else {
					dlist_foreach(common->livebox_list, l, handler) {
						lb_invoke_event_handler(handler, LB_EVENT_PD_UPDATED);
					}
				}
			} else {
				dlist_foreach(common->livebox_list, l, handler) {
					lb_invoke_event_handler(handler, LB_EVENT_PD_UPDATED);
				}
			}
		}
	}

out:
	return NULL;
}

static struct packet *master_update_mode(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	struct livebox_common *common;
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

	common = lb_find_common_handle(pkgname, id);
	if (!common) {
		ErrPrint("Livebox(%s) is not found\n", id);
		goto out;
	}

	if (common->state != CREATE) {
		ErrPrint("Livebox(%s) is not created yet\n", id);
		goto out;
	}

	if (status == (int)LB_STATUS_SUCCESS) {
		lb_set_update_mode(common, active_mode);
	}

	common->request.update_mode = 0;
	dlist_foreach_safe(common->livebox_list, l, n, handler) {
		if (handler->cbs.update_mode.cb) {
			ret_cb_t cb;
			void *cbdata;

			cb = handler->cbs.update_mode.cb;
			cbdata = handler->cbs.update_mode.data;

			handler->cbs.update_mode.cb = NULL;
			handler->cbs.update_mode.data = NULL;

			cb(handler, status, cbdata);
		} else if (status == (int)LB_STATUS_SUCCESS) {
			lb_invoke_event_handler(handler, LB_EVENT_UPDATE_MODE_CHANGED);
		}
	}

out:
	return NULL;
}

static struct packet *master_size_changed(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	struct livebox_common *common;
	const char *pkgname;
	const char *id;
	const char *fbfile;
	int status;
	int ret;
	int w;
	int h;
	int is_pd;

	if (!packet) {
		ErrPrint("Invalid packet\n");
		goto out;
	}

	ret = packet_get(packet, "sssiiii", &pkgname, &id, &fbfile, &is_pd, &w, &h, &status);
	if (ret != 7) {
		ErrPrint("Invalid argument\n");
		goto out;
	}

	common = lb_find_common_handle(pkgname, id);
	if (!common) {
		ErrPrint("Livebox(%s) is not found\n", id);
		goto out;
	}

	if (common->state != CREATE) {
		ErrPrint("Livebox(%s) is not created yet\n", id);
		goto out;
	}

	common->request.size_changed = 0;
	if (is_pd) {
		/*!
		 * \NOTE
		 * PD is not able to resized by the client.
		 * PD is only can be managed by the provider.
		 * So the PD has no private resized event handler.
		 * Notify it via global event handler only.
		 */
		if (status == (int)LB_STATUS_SUCCESS) {
			struct dlist *l;

			lb_set_pdsize(common, w, h);
			dlist_foreach(common->livebox_list, l, handler) {
				lb_invoke_event_handler(handler, LB_EVENT_PD_SIZE_CHANGED);
			}
		} else {
			ErrPrint("This is not possible. PD Size is changed but the return value is not ZERO (%d)\n", status);
		}
	} else {
		struct dlist *l;
		struct dlist *n;

		if (status == (int)LB_STATUS_SUCCESS) {
			lb_set_size(common, w, h);

			/*!
			 * \NOTE
			 * If there is a created LB FB, 
			 * Update it too.
			 */
			if (lb_get_lb_fb(common)) {
				(void)lb_set_lb_fb(common, fbfile);

				ret = lb_sync_lb_fb(common);
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
		dlist_foreach_safe(common->livebox_list, l, n, handler) {
			if (handler->cbs.size_changed.cb) {
				ret_cb_t cb;
				void *cbdata;

				cb = handler->cbs.size_changed.cb;
				cbdata = handler->cbs.size_changed.data;

				handler->cbs.size_changed.cb = NULL;
				handler->cbs.size_changed.data = NULL;

				cb(handler, status, cbdata);
			} else if (status == (int)LB_STATUS_SUCCESS) {
				lb_invoke_event_handler(handler, LB_EVENT_LB_SIZE_CHANGED);
			}
		}
	}

out:
	return NULL;
}

static struct packet *master_period_changed(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	struct livebox_common *common;
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

	common = lb_find_common_handle(pkgname, id);
	if (!common) {
		ErrPrint("Livebox(%s) is not found\n", id);
		goto out;
	}

	if (common->state != CREATE) {
		ErrPrint("Livebox(%s) is not created\n", id);
		goto out;
	}

	if (status == (int)LB_STATUS_SUCCESS) {
		lb_set_period(common, period);
	}

	common->request.period_changed = 0;

	dlist_foreach_safe(common->livebox_list, l, n, handler) {
		if (handler->cbs.period_changed.cb) {
			ret_cb_t cb;
			void *cbdata;

			cb = handler->cbs.period_changed.cb;
			cbdata = handler->cbs.period_changed.data;

			handler->cbs.period_changed.cb = NULL;
			handler->cbs.period_changed.data = NULL;

			cb(handler, status, cbdata);
		} else if (status == (int)LB_STATUS_SUCCESS) {
			lb_invoke_event_handler(handler, LB_EVENT_PERIOD_CHANGED);
		}
	}

out:
	return NULL;
}

static struct packet *master_group_changed(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	struct livebox_common *common;
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

	common = lb_find_common_handle(pkgname, id);
	if (!common) {
		ErrPrint("Livebox(%s) is not exists\n", id);
		goto out;
	}

	if (common->state != CREATE) {
		/*!
		 * \note
		 * Do no access this handler,
		 * You cannot believe this handler anymore.
		 */
		ErrPrint("Livebox(%s) is not created\n", id);
		goto out;
	}

	if (status == (int)LB_STATUS_SUCCESS) {
		(void)lb_set_group(common, cluster, category);
	}

	common->request.group_changed = 0;

	dlist_foreach_safe(common->livebox_list, l, n, handler) {
		if (handler->cbs.group_changed.cb) {
			ret_cb_t cb;
			void *cbdata;

			cb = handler->cbs.group_changed.cb;
			cbdata = handler->cbs.group_changed.data;

			handler->cbs.group_changed.cb = NULL;
			handler->cbs.group_changed.data = NULL;

			cb(handler, status, cbdata);
		} else if (status == (int)LB_STATUS_SUCCESS) {
			lb_invoke_event_handler(handler, LB_EVENT_GROUP_CHANGED);
		}
	}

out:
	return NULL;
}

static struct packet *master_created(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	struct livebox_common *common;
	struct dlist *l;

	int lb_w;
	int lb_h;
	int pd_w;
	int pd_h;
	const char *pkgname;
	const char *id;

	const char *content;
	const char *cluster;
	const char *category;
	const char *lb_fname;
	const char *pd_fname;
	const char *title;

	double timestamp;
	const char *auto_launch;
	double priority;
	int size_list;
	int user;
	int pinup_supported;
	enum lb_type lb_type;
	enum pd_type pd_type;
	double period;
	int is_pinned_up;

	int old_state = DESTROYED;

	int ret;

	ret = packet_get(packet, "dsssiiiisssssdiiiiidsi",
			&timestamp,
			&pkgname, &id, &content,
			&lb_w, &lb_h, &pd_w, &pd_h,
			&cluster, &category, &lb_fname, &pd_fname,
			&auto_launch, &priority, &size_list, &user, &pinup_supported,
			&lb_type, &pd_type, &period, &title, &is_pinned_up);
	if (ret != 22) {
		ErrPrint("Invalid argument\n");
		ret = LB_STATUS_ERROR_INVALID;
		goto out;
	}

	ErrPrint("[%lf] pkgname: %s, id: %s, content: %s, "
		"pd_w: %d, pd_h: %d, lb_w: %d, lb_h: %d, "
		"cluster: %s, category: %s, lb_fname: \"%s\", pd_fname: \"%s\", "
		"auto_launch: %s, priority: %lf, size_list: %d, user: %d, pinup: %d, "
		"lb_type: %d, pd_type: %d, period: %lf, title: [%s], is_pinned_up: %d\n",
		timestamp, pkgname, id, content,
		pd_w, pd_h, lb_w, lb_h,
		cluster, category, lb_fname, pd_fname,
		auto_launch, priority, size_list, user, pinup_supported,
		lb_type, pd_type, period, title, is_pinned_up);

	common = lb_find_common_handle_by_timestamp(timestamp);
	if (!common) {
		handler = lb_new_livebox(pkgname, id, timestamp, cluster, category);
		if (!handler) {
			ErrPrint("Failed to create a new livebox\n");
			ret = LB_STATUS_ERROR_FAULT;
			goto out;
		}
		common = handler->common;
		old_state = common->state;
	} else {
		if (common->state != CREATE) {
			if (common->state != DELETE) {
				/*!
				 * \note
				 * This is not possible!!!
				 */
				ErrPrint("Invalid handler\n");
				ret = LB_STATUS_ERROR_INVALID;
				goto out;
			}

			/*!
			 * \note
			 * After get the delete states,
			 * call the create callback with deleted result.
			 */
		}

		old_state = common->state;

		if (common->id) {
			ErrPrint("Already created: timestamp[%lf] "
				"pkgname[%s], id[%s] content[%s] "
				"cluster[%s] category[%s] lb_fname[%s] pd_fname[%s]\n",
					timestamp, pkgname, id,
					content, cluster, category,
					lb_fname, pd_fname);

			ret = LB_STATUS_ERROR_ALREADY;
			goto out;
		}

		lb_set_id(common, id);
	}

	common->request.created = 0;
	lb_set_size(common, lb_w, lb_h);
	common->lb.type = lb_type;
	common->is_pinned_up = is_pinned_up;

	switch (lb_type) {
	case _LB_TYPE_ELEMENTARY:
	case _LB_TYPE_FILE:
		break;
	case _LB_TYPE_SCRIPT:
	case _LB_TYPE_BUFFER:
		if (!strlen(lb_fname)) {
			break;
		}
		(void)lb_set_lb_fb(common, lb_fname);

		/*!
		 * \note
		 * Livebox should create the lock file from here.
		 * Even if the old_state == DELETE,
		 * the lock file will be deleted from deleted event callback.
		 */
		switch (fb_type(lb_get_lb_fb(common))) {
		case BUFFER_TYPE_FILE:
		case BUFFER_TYPE_SHM:
			lb_create_lock_file(common, 0);
			break;
		case BUFFER_TYPE_PIXMAP:
		case BUFFER_TYPE_ERROR:
		default:
			break;
		}

		ret = lb_sync_lb_fb(common);
		if (ret < 0) {
			ErrPrint("Failed to do sync FB (%s - %s)\n", pkgname, util_basename(util_uri_to_path(id)));
		}
		break;
	case _LB_TYPE_TEXT:
		lb_set_text_lb(common);
		break;
	default:
		break;
	}

	common->pd.type = pd_type;
	lb_set_pdsize(common, pd_w, pd_h);
	lb_set_default_pdsize(common, pd_w, pd_h);
	switch (pd_type) {
	case _PD_TYPE_SCRIPT:
	case _PD_TYPE_BUFFER:
		if (!strlen(pd_fname)) {
			break;
		}

		lb_set_pd_fb(common, pd_fname);

		ret = lb_sync_pd_fb(common);
		if (ret < 0) {
			ErrPrint("Failed to do sync FB (%s - %s)\n", pkgname, util_basename(util_uri_to_path(id)));
		}

		/*!
		 * \brief
		 * PD doesn't need to create the lock file from here.
		 * Just create it from PD_CREATED event.
		 */

		break;
	case _PD_TYPE_TEXT:
		lb_set_text_pd(common);
		break;
	case _PD_TYPE_ELEMENTARY:
	default:
		break;
	}

	lb_set_priority(common, priority);

	lb_set_size_list(common, size_list);
	lb_set_group(common, cluster, category);

	lb_set_content(common, content);
	lb_set_title(common, title);

	lb_set_user(common, user);

	lb_set_auto_launch(common, auto_launch);
	lb_set_pinup(common, pinup_supported);

	lb_set_period(common, period);

	ret = 0;

	if (common->state == CREATE) {
		dlist_foreach(common->livebox_list, l, handler) {
			/*!
			 * \note
			 * These callback can change the handler->state.
			 * So we have to use the "old_state" which stored state before call these callbacks
			 */

			if (handler->cbs.created.cb) {
				ret_cb_t cb;
				void *cbdata;

				cb = handler->cbs.created.cb;
				cbdata = handler->cbs.created.data;

				handler->cbs.created.cb = NULL;
				handler->cbs.created.data = NULL;

				cb(handler, ret, cbdata);
			} else {
				lb_invoke_event_handler(handler, LB_EVENT_CREATED);
			}
		}
	}

out:
	if (ret == 0 && old_state == DELETE) {
		struct dlist *n;

		DbgPrint("Take place an unexpected case [%d]\n", common->refcnt);
		dlist_foreach_safe(common->livebox_list, l, n, handler) {
			if (handler->cbs.created.cb) {
				if (!handler->common->request.deleted) {
					if (lb_send_delete(handler, common->delete_type, handler->cbs.created.cb, handler->cbs.created.data) < 0) {
						/*!
						 * \note
						 * Already sent or something else happens.
						 * Callback will be called in any cases
						 */
					}
				} else if (handler->state != DELETE) {
					handler->cbs.created.cb(handler, LB_STATUS_ERROR_CANCEL, handler->cbs.created.data);
					lb_unref(handler, 1);
				}
			} else {
				lb_invoke_event_handler(handler, LB_EVENT_DELETED);
				lb_unref(handler, 1);
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
		 * Then the call the deleted event callback with LB_STATUS_ERROR_CANCEL errno.
		 */
	}

	return NULL;
}

static struct method s_table[] = {
	{
		.cmd = "lb_updated", /* pkgname, id, lb_w, lb_h, priority, ret */
		.handler = master_lb_updated,
	},
	{
		.cmd = "pd_updated", /* pkgname, id, descfile, pd_w, pd_h, ret */
		.handler = master_pd_updated,
	},
	{
		.cmd = "extra_info",
		.handler = master_extra_info,
	},
	{
		.cmd = "deleted", /* pkgname, id, timestamp, ret */
		.handler = master_deleted,
	},
	{
		.cmd = "fault_package", /* pkgname, id, function, ret */
		.handler = master_fault_package,
	},
	{
		.cmd = "scroll",
		.handler = master_hold_scroll,
	},
	{
		.cmd = "lb_update_begin",
		.handler = master_lb_update_begin,
	},
	{
		.cmd = "lb_update_end",
		.handler = master_lb_update_end,
	},

	{
		.cmd = "pd_update_begin",
		.handler = master_pd_update_begin,
	},
	{
		.cmd = "pd_update_end",
		.handler = master_pd_update_end,
	},
	{
		.cmd = "access_status",
		.handler = master_access_status,
	},
	{
		.cmd = "key_status",
		.handler = master_key_status,
	},
	{
		.cmd = "close_pd",
		.handler = master_request_close_pd,
	},

	{
		.cmd = "pd_created",
		.handler = master_pd_created,
	},
	{
		.cmd = "pd_destroyed",
		.handler = master_pd_destroyed,
	},
	{
		.cmd = "created", /* timestamp, pkgname, id, content, lb_w, lb_h, pd_w, pd_h, cluster, category, lb_file, pd_file, auto_launch, priority, size_list, is_user, pinup_supported, text_lb, text_pd, period, ret */
		.handler = master_created,
	},
	{
		.cmd = "group_changed",
		.handler = master_group_changed,
	},
	{
		.cmd = "period_changed",
		.handler = master_period_changed,
	},
	{
		.cmd = "size_changed",
		.handler = master_size_changed,
	},
	{
		.cmd = "pinup",
		.handler = master_pinup,
	},

	{
		.cmd = "update_mode",
		.handler = master_update_mode,
	},

	{
		.cmd = NULL,
		.handler = NULL,
	},
};

static void acquire_cb(struct livebox *handler, const struct packet *result, void *data)
{
	if (!result) {
		DbgPrint("Result packet is not valid\n");
	} else {
		int ret;

		if (packet_get(result, "i", &ret) != 1) {
			ErrPrint("Invalid argument\n");
		} else {
			DbgPrint("Acquire returns: %d\n", ret);
		}
	}

	return;
}

static inline int make_connection(void)
{
	struct packet *packet;
	int ret;
	unsigned int cmd = CMD_ACQUIRE;

	DbgPrint("Let's making connection!\n");

	s_info.fd = com_core_packet_client_init(client_addr(), 0, s_table);
	if (s_info.fd < 0) {
		ErrPrint("Try this again later\n");
		return LB_STATUS_ERROR_IO;
	}

	packet = packet_create((const char *)&cmd, "ds", util_timestamp(), client_direct_addr());
	if (!packet) {
		com_core_packet_client_fini(s_info.fd);
		s_info.fd = -1;
		return LB_STATUS_ERROR_FAULT;
	}

	ret = master_rpc_async_request(NULL, packet, 1, acquire_cb, NULL);
	if (ret < 0) {
		ErrPrint("Master RPC returns %d\n", ret);
		com_core_packet_client_fini(s_info.fd);
		s_info.fd = -1;
		return LB_STATUS_ERROR_IO;
	}

	return LB_STATUS_SUCCESS;
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
	if (state == 1 && make_connection() == (int)LB_STATUS_SUCCESS) {
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
	lb_invoke_fault_handler(LB_FAULT_PROVIDER_DISCONNECTED, MASTER_PKGNAME, "default", "disconnected");

	lb_delete_all();

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
		.cmd = "lb_updated", /* pkgname, id, lb_w, lb_h, priority, ret */
		.handler = master_lb_updated,
	},
	{
		.cmd = "pd_updated", /* pkgname, id, descfile, pd_w, pd_h, ret */
		.handler = master_pd_updated,
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
			return LB_STATUS_ERROR_MEMORY;
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
	return LB_STATUS_SUCCESS;
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
	return LB_STATUS_SUCCESS;
}

/* End of a file */
