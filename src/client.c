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

#include <dlog.h>
#include <glib.h>

#include <vconf.h>
#include <vconf-keys.h>

#include <packet.h>
#include <com-core.h>
#include <com-core_packet.h>
#include <livebox-errno.h>

#include "debug.h"
#include "client.h"
#include "livebox.h"
#include "livebox_internal.h"
#include "desc_parser.h"
#include "fb.h"
#include "util.h"
#include "master_rpc.h"
#include "conf.h"
#include "critical_log.h"

static struct info {
	int fd;
	guint timer_id;
} s_info = {
	.fd = -1,
	.timer_id = 0,
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

	master_rpc_clear_fault_package(pkgname);
	lb_invoke_fault_handler(LB_FAULT_DEACTIVATED, pkgname, id, function);
	return NULL;
}

static struct packet *master_hold_scroll(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	const char *pkgname;
	const char *id;
	int seize;
	int ret;

	ret = packet_get(packet, "ssi", &pkgname, &id, &seize);
	if (ret != 3) {
		ErrPrint("Invalid argument\n");
		goto out;
	}

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ErrPrint("Instance(%s) is not exists\n", id);
		goto out;
	}

	DbgPrint("HOLD: %s %d\n", id, seize);
	lb_invoke_event_handler(handler, seize ? LB_EVENT_HOLD_SCROLL : LB_EVENT_RELEASE_SCROLL);

out:
	return NULL;
}

static struct packet *master_pinup(pid_t pid, int handle, const struct packet *packet)
{
	const char *pkgname;
	const char *id;
	const char *content;
	struct livebox *handler;
	char *new_content;
	int ret;
	int status;
	int pinup;

	ret = packet_get(packet, "iisss", &status, &pinup, &pkgname, &id, &content);
	if (ret != 5) {
		ErrPrint("Invalid argument\n");
		goto out;
	}

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ErrPrint("Instance (%s) is not exists\n", id);
		goto out;
	}

	if (status == 0) {
		new_content = strdup(content);
		if (new_content) {
			free(handler->content);
			handler->content = new_content;
			handler->is_pinned_up = pinup;
		} else {
			ErrPrint("Heap: %s\n", strerror(errno));
			status = LB_STATUS_ERROR_MEMORY;
		}
	}

	if (handler->pinup_cb) {
		ret_cb_t cb;
		void *cbdata;

		/* Make sure that user can call pinup API in its result callback */
		cb = handler->pinup_cb;
		cbdata = handler->pinup_cbdata;

		handler->pinup_cb = NULL;
		handler->pinup_cbdata = NULL;

		cb(handler, status, cbdata);
	} else if (status == 0) {
		lb_invoke_event_handler(handler, LB_EVENT_PINUP_CHANGED);
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

	if (packet_get(packet, "ssd", &pkgname, &id, &timestamp) != 3) {
		ErrPrint("Invalid arguemnt\n");
		goto out;
	}

	handler = lb_find_livebox_by_timestamp(timestamp);
	if (!handler) {
		/*!
		 * \note
		 * This can be happens only if the user delete a livebox
		 * right after create it before receive created event.
		 */
		goto out;
	}

	/*!< Check validity of this "handler" */
	if (handler->state != CREATE) {
		if (handler->state != DELETE) {
			/*!
			 * \note
			 * This is not possible
			 */
			CRITICAL_LOG("Already deleted handler (%s - %s)\n", pkgname, id);
			return NULL;
		}
	}

	if (handler->created_cb) {
		ret_cb_t cb;
		void *cbdata;
		/*!
		 * \note
		 *
		 * "if (handler->id == NULL)"
		 *
		 * The instance is not created yet.
		 * But the master forcely destroy it and send destroyed event to this
		 * without the created event.
		 *
		 * It could be destroyed when a slave has critical error(fault)
		 * before creating an instance successfully.
		 */
		if (handler->created_cb == handler->deleted_cb) {
			if (handler->created_cbdata != handler->deleted_cbdata)
				DbgPrint("cb is same but cbdata is different (%s - %s)\n", pkgname, id);

			handler->deleted_cb = NULL;
			handler->deleted_cbdata = NULL;
		}

		cb = handler->created_cb;
		cbdata = handler->created_cbdata;

		handler->created_cb = NULL;
		handler->created_cbdata = NULL;

		cb(handler, LB_STATUS_ERROR_CANCEL, cbdata);
	} else if (handler->id) {
		if (handler->deleted_cb) {
			ret_cb_t cb;
			void *cbdata;

			cb = handler->deleted_cb;
			cbdata = handler->deleted_cbdata;

			handler->deleted_cb = NULL;
			handler->deleted_cbdata = NULL;

			cb(handler, LB_STATUS_SUCCESS, cbdata);
		} else {
			lb_invoke_event_handler(handler, LB_EVENT_DELETED);
		}
	}

	/* Just try to delete it, if a user didn't remove it from the live box list */
	lb_unref(handler);

out:
	return NULL;
}

static struct packet *master_lb_update_begin(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
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

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ErrPrint("Instance[%s] is not exists\n", id);
		goto out;
	}

	if (handler->state != CREATE) {
		ErrPrint("(%s) is not created\n", id);
		goto out;
	}

	lb_set_priority(handler, priority);
	lb_set_content(handler, content);
	lb_set_title(handler, title);

	/*!
	 * \NOTE
	 * Width & Height is not changed in this case.
	 * If the active update is began, the size should not be changed,
	 * And if the size is changed, the provider should finish the updating first.
	 * And then begin updating again after change its size.
	 */
	if (lb_get_lb_fb(handler)) {
		lb_set_lb_fb(handler, fbfile);

		ret = fb_sync(lb_get_lb_fb(handler));
		if (ret != LB_STATUS_SUCCESS)
			ErrPrint("Failed to do sync FB (%s - %s) (%d)\n", pkgname, fbfile, ret);
		else
			lb_invoke_event_handler(handler, LB_EVENT_LB_UPDATE_BEGIN);
	} else {
		ErrPrint("Invalid request[%s], %s\n", id, fbfile);
	}

out:
	return NULL;
}

static struct packet *master_pd_update_begin(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	const char *pkgname;
	const char *id;
	const char *fbfile;
	int ret;

	ret = packet_get(packet, "sss", &pkgname, &id, &fbfile);
	if (ret != 2) {
		ErrPrint("Invalid argument\n");
		goto out;
	}

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ErrPrint("Instance[%s] is not exists\n", id);
		goto out;
	}

	if (handler->state != CREATE) {
		ErrPrint("[%s] is not created\n", id);
		goto out;
	}

	if (lb_get_pd_fb(handler)) {
		lb_set_lb_fb(handler, fbfile);

		ret = fb_sync(lb_get_lb_fb(handler));
		if (ret != LB_STATUS_SUCCESS)
			ErrPrint("Failed to do sync FB (%s - %s) (%d)\n", pkgname, fbfile, ret);
		else
			lb_invoke_event_handler(handler, LB_EVENT_PD_UPDATE_BEGIN);
	} else {
		ErrPrint("Invalid request[%s], %s\n", id, fbfile);
	}

out:
	return NULL;
}

static struct packet *master_lb_update_end(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	const char *pkgname;
	const char *id;
	int ret;

	ret = packet_get(packet, "ss", &pkgname, &id);
	if (ret != 2) {
		ErrPrint("Invalid argument\n");
		goto out;
	}

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ErrPrint("Instance[%s] is not exists\n", id);
		goto out;
	}

	if (handler->state != CREATE) {
		ErrPrint("[%s] is not created\n", id);
		goto out;
	}

	if (lb_get_lb_fb(handler)) {
		lb_invoke_event_handler(handler, LB_EVENT_LB_UPDATE_END);
	} else {
		ErrPrint("Invalid request[%s]\n", id);
	}

out:
	return NULL;
}

static struct packet *master_access_status(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	const char *pkgname;
	const char *id;
	int ret;
	int status;

	ret = packet_get(packet, "ssi", &pkgname, &id, &status);
	if (ret != 3) {
		ErrPrint("Invalid argument\n");
		goto out;
	}

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ErrPrint("Instance[%s] is not exists\n", id);
		goto out;
	}

	if (handler->state != CREATE) {
		ErrPrint("[%s] is not created\n", id);
		goto out;
	}

	if (handler->access_event_cb) {
		ret_cb_t cb;
		void *cbdata;

		cb = handler->access_event_cb;
		cbdata = handler->access_event_cbdata;

		handler->access_event_cb = NULL;
		handler->access_event_cbdata = NULL;

		cb(handler, status, cbdata);
	} else {
		ErrPrint("Invalid event[%s]\n", id);
	}
out:
	return NULL;
}

static struct packet *master_pd_update_end(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	const char *pkgname;
	const char *id;
	int ret;

	ret = packet_get(packet, "ss", &pkgname, &id);
	if (ret != 2) {
		ErrPrint("Invalid argument\n");
		goto out;
	}

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ErrPrint("Instance[%s] is not exists\n", id);
		goto out;
	}

	if (handler->state != CREATE) {
		ErrPrint("[%s] is not created\n", id);
		goto out;
	}

	if (lb_get_lb_fb(handler)) {
		lb_invoke_event_handler(handler, LB_EVENT_PD_UPDATE_END);
	} else {
		ErrPrint("Invalid request[%s]", id);
	}

out:
	return NULL;
}

static struct packet *master_lb_updated(pid_t pid, int handle, const struct packet *packet)
{
	const char *pkgname;
	const char *id;
	const char *fbfile;
	const char *content;
	const char *title;
	struct livebox *handler;
	int lb_w;
	int lb_h;
	double priority;
	int ret;

	ret = packet_get(packet, "sssiidss",
				&pkgname, &id,
				&fbfile, &lb_w, &lb_h,
				&priority, &content, &title);
	if (ret != 8) {
		ErrPrint("Invalid argument\n");
		goto out;
	}

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ErrPrint("instance(%s) is not exists\n", id);
		goto out;
	}

	if (handler->state != CREATE) {
		/*!
		 * \note
		 * Already deleted by the user.
		 * Don't try to notice anything with this, Just ignore all events
		 * Beacuse the user doesn't wants know about this anymore
		 */
		ErrPrint("(%s) is not exists, but updated\n", id);
		goto out;
	}

	lb_set_priority(handler, priority);
	lb_set_content(handler, content);
	lb_set_title(handler, title);
	lb_set_size(handler, lb_w, lb_h);

	if (lb_text_lb(handler)) {
		(void)parse_desc(handler, livebox_filename(handler), 0);
		/*!
		 * \note
		 * DESC parser will call the "text event callback".
		 * Don't need to call global event callback in this case.
		 */
		goto out;
	} else if (lb_get_lb_fb(handler)) {
		lb_set_lb_fb(handler, fbfile);
		ret = fb_sync(lb_get_lb_fb(handler));
		if (ret != LB_STATUS_SUCCESS)
			ErrPrint("Failed to do sync FB (%s - %s) (%d)\n", pkgname, util_basename(util_uri_to_path(id)), ret);
	} else {
		ret = LB_STATUS_SUCCESS;
	}

	if (ret == LB_STATUS_SUCCESS)
		lb_invoke_event_handler(handler, LB_EVENT_LB_UPDATED);

out:
	return NULL;
}

static struct packet *master_pd_created(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	const char *pkgname;
	const char *id;
	const char *buf_id;
	int width;
	int height;
	int ret;
	int status;

	ret = packet_get(packet, "sssiii", &pkgname, &id, &buf_id, &width, &height, &status);
	if (ret != 6) {
		ErrPrint("Invalid argument\n");
		goto out;
	}

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ErrPrint("Instance(%s) is not exists\n", id);
		goto out;
	}

	if (handler->state != CREATE) {
		ErrPrint("Instance(%s) is not created\n", id);
		goto out;
	}

	lb_set_pdsize(handler, width, height);
	if (lb_text_pd(handler)) {
		DbgPrint("Text TYPE does not need to handle this\n");
	} else {
		(void)lb_set_pd_fb(handler, buf_id);
		ret = fb_sync(lb_get_pd_fb(handler));
		if (ret < 0)
			ErrPrint("Failed to do sync FB (%s - %s)\n", pkgname, util_basename(util_uri_to_path(id)));
	}

	handler->is_pd_created = (status == 0);

	if (handler->pd_created_cb) {
		ret_cb_t cb;
		void *cbdata;

		cb = handler->pd_created_cb;
		cbdata = handler->pd_created_cbdata;

		handler->pd_created_cb = NULL;
		handler->pd_created_cbdata = NULL;

		/*!
		 * Before call the Callback function,
		 * pd_create_cb must be reset.
		 * Because, in the create callback, user can call create_pd function again.
		 */
		DbgPrint("PERF_DBOX\n");
		cb(handler, status, cbdata);
	} else if (handler->is_pd_created) {
		lb_invoke_event_handler(handler, LB_EVENT_PD_CREATED);
	}

out:
	return NULL;
}

static struct packet *master_pd_destroyed(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	const char *pkgname;
	const char *id;
	int ret;
	int status;

	ret = packet_get(packet, "ssi", &pkgname, &id, &status);
	if (ret != 3) {
		ErrPrint("Invalid argument\n");
		goto out;
	}

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ErrPrint("Instance(%s) is not exists\n", id);
		goto out;
	}

	if (handler->state != CREATE) {
		ErrPrint("Instance(%s) is not created\n", id);
		goto out;
	}

	handler->is_pd_created = 0;

	if (handler->pd_destroyed_cb) {
		ret_cb_t cb;
		void *cbdata;

		cb = handler->pd_destroyed_cb;
		cbdata = handler->pd_destroyed_cbdata;

		handler->pd_destroyed_cb = NULL;
		handler->pd_destroyed_cbdata = NULL;

		/*!
		 * Before call the Callback function,
		 * pd_destroyed_cb must be reset.
		 * Because, in the create callback, user can call destroy_pd function again.
		 */
		cb(handler, status, cbdata);
	} else if (status == 0) {
		lb_invoke_event_handler(handler, LB_EVENT_PD_DESTROYED);
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

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ErrPrint("Instance(%s) is not exists\n", id);
		goto out;
	}

	if (handler->state != CREATE) {
		/*!
		 * \note
		 * This handler is already deleted by the user.
		 * So don't try to notice anything about this anymore.
		 * Just ignore all events.
		 */
		ErrPrint("Instance(%s) is not created\n", id);
		goto out;
	}

	lb_set_pdsize(handler, pd_w, pd_h);

	if (lb_text_pd(handler)) {
		(void)parse_desc(handler, descfile, 1);
	} else {
		(void)lb_set_pd_fb(handler, fbfile);

		ret = fb_sync(lb_get_pd_fb(handler));
		if (ret < 0)
			ErrPrint("Failed to do sync FB (%s - %s), %d\n", pkgname, util_basename(util_uri_to_path(id)), ret);
		else
			lb_invoke_event_handler(handler, LB_EVENT_PD_UPDATED);
	}

out:
	return NULL;
}

static struct packet *master_update_mode(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
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

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ErrPrint("Livebox(%s) is not found\n", id);
		goto out;
	}

	if (handler->state != CREATE) {
		ErrPrint("Livebox(%s) is not created yet\n", id);
		goto out;
	}

	if (status == LB_STATUS_SUCCESS)
		lb_set_update_mode(handler, active_mode);

	if (handler->update_mode_cb) {
		ret_cb_t cb;
		void *cbdata;

		cb = handler->update_mode_cb;
		cbdata = handler->update_mode_cbdata;

		handler->update_mode_cb = NULL;
		handler->update_mode_cbdata = NULL;

		cb(handler, status, cbdata);
	} else {
		lb_invoke_event_handler(handler, LB_EVENT_UPDATE_MODE_CHANGED);
	}

out:
	return NULL;
}

static struct packet *master_size_changed(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
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

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ErrPrint("Livebox(%s) is not found\n", id);
		goto out;
	}

	if (handler->state != CREATE) {
		ErrPrint("Livebox(%s) is not created yet\n", id);
		goto out;
	}

	if (is_pd) {
		/*!
		 * \NOTE
		 * PD is not able to resized by the client.
		 * PD is only can be managed by the provider.
		 * So the PD has no private resized event handler.
		 * Notify it via global event handler only.
		 */
		if (status == 0) {
			lb_set_pdsize(handler, w, h);
			lb_invoke_event_handler(handler, LB_EVENT_PD_SIZE_CHANGED);
		} else {
			ErrPrint("This is not possible. PD Size is changed but the return value is not ZERO (%d)\n", status);
		}
	} else {
		if (status == 0) {
			lb_set_size(handler, w, h);

			/*!
			 * \NOTE
			 * If there is a created LB FB, 
			 * Update it too.
			 */
			if (lb_get_lb_fb(handler)) {
				lb_set_lb_fb(handler, fbfile);

				ret = fb_sync(lb_get_lb_fb(handler));
				if (ret < 0)
					ErrPrint("Failed to do sync FB (%s - %s)\n", pkgname, util_basename(util_uri_to_path(id)));

				/* Just update the size info only. */
			}

			/*!
			 * \NOTE
			 * I cannot believe client.
			 * So I added some log before & after call the user callback.
			 */
			if (handler->size_changed_cb) {
				ret_cb_t cb;
				void *cbdata;

				cb = handler->size_changed_cb;
				cbdata = handler->size_cbdata;

				handler->size_changed_cb = NULL;
				handler->size_cbdata = NULL;

				cb(handler, status, cbdata);
			} else {
				lb_invoke_event_handler(handler, LB_EVENT_LB_SIZE_CHANGED);
			}
		} else {
			if (handler->size_changed_cb) {
				ret_cb_t cb;
				void *cbdata;

				cb = handler->size_changed_cb;
				cbdata = handler->size_cbdata;

				handler->size_changed_cb = NULL;
				handler->size_cbdata = NULL;

				cb(handler, status, cbdata);
			}
		}
	}

out:
	return NULL;
}

static struct packet *master_period_changed(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
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

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ErrPrint("Livebox(%s) is not found\n", id);
		goto out;
	}

	if (handler->state != CREATE) {
		ErrPrint("Livebox(%s) is not created\n", id);
		goto out;
	}

	if (status == 0)
		lb_set_period(handler, period);

	if (handler->period_changed_cb) {
		ret_cb_t cb;
		void *cbdata;

		cb = handler->period_changed_cb;
		cbdata = handler->period_cbdata;

		handler->period_changed_cb = NULL;
		handler->period_cbdata = NULL;

		cb(handler, status, cbdata);
	} else if (status == 0) {
		lb_invoke_event_handler(handler, LB_EVENT_PERIOD_CHANGED);
	}

out:
	return NULL;
}

static struct packet *master_group_changed(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
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

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ErrPrint("Livebox(%s) is not exists\n", id);
		goto out;
	}

	if (handler->state != CREATE) {
		/*!
		 * \note
		 * Do no access this handler,
		 * You cannot believe this handler anymore.
		 */
		ErrPrint("Livebox(%s) is not created\n", id);
		goto out;
	}

	if (status == 0)
		(void)lb_set_group(handler, cluster, category);

	if (handler->group_changed_cb) {
		ret_cb_t cb;
		void *cbdata;

		cb = handler->group_changed_cb;
		cbdata = handler->group_cbdata;

		handler->group_changed_cb = NULL;
		handler->group_cbdata = NULL;

		cb(handler, status, cbdata);
	} else if (status == 0) {
		lb_invoke_event_handler(handler, LB_EVENT_GROUP_CHANGED);
	}

out:
	return NULL;
}

static struct packet *master_created(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;

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

	handler = lb_find_livebox_by_timestamp(timestamp);
	if (!handler) {
		handler = lb_new_livebox(pkgname, id, timestamp);
		if (!handler) {
			ErrPrint("Failed to create a new livebox\n");
			ret = LB_STATUS_ERROR_FAULT;
			goto out;
		}

		old_state = handler->state;
	} else {
		if (handler->state != CREATE) {
			if (handler->state != DELETE) {
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

		old_state = handler->state;

		if (handler->id) {
			ErrPrint("Already created: timestamp[%lf] "
				"pkgname[%s], id[%s] content[%s] "
				"cluster[%s] category[%s] lb_fname[%s] pd_fname[%s]\n",
					timestamp, pkgname, id,
					content, cluster, category,
					lb_fname, pd_fname);

			ret = LB_STATUS_ERROR_ALREADY;
			goto out;
		}

		lb_set_id(handler, id);
	}

	lb_set_size(handler, lb_w, lb_h);
	handler->lb.type = lb_type;
	handler->is_pinned_up = is_pinned_up;

	switch (lb_type) {
	case _LB_TYPE_FILE:
		break;
	case _LB_TYPE_SCRIPT:
	case _LB_TYPE_BUFFER:
		if (!strlen(lb_fname))
			break;
		lb_set_lb_fb(handler, lb_fname);
		ret = fb_sync(lb_get_lb_fb(handler));
		if (ret < 0)
			ErrPrint("Failed to do sync FB (%s - %s)\n", pkgname, util_basename(util_uri_to_path(id)));
		break;
	case _LB_TYPE_TEXT:
		lb_set_text_lb(handler);
		break;
	default:
		break;
	}

	handler->pd.type = pd_type;
	lb_set_pdsize(handler, pd_w, pd_h);
	lb_set_default_pdsize(handler, pd_w, pd_h);
	switch (pd_type) {
	case _PD_TYPE_SCRIPT:
	case _PD_TYPE_BUFFER:
		if (!strlen(pd_fname))
			break;

		lb_set_pd_fb(handler, pd_fname);
		ret = fb_sync(lb_get_pd_fb(handler));
		if (ret < 0)
			ErrPrint("Failed to do sync FB (%s - %s)\n", pkgname, util_basename(util_uri_to_path(id)));
		break;
	case _PD_TYPE_TEXT:
		lb_set_text_pd(handler);
		break;
	default:
		break;
	}

	lb_set_priority(handler, priority);

	lb_set_size_list(handler, size_list);
	lb_set_group(handler, cluster, category);

	lb_set_content(handler, content);
	lb_set_title(handler, title);

	lb_set_user(handler, user);

	lb_set_auto_launch(handler, auto_launch);
	lb_set_pinup(handler, pinup_supported);

	lb_set_period(handler, period);

	ret = 0;

	if (handler->state == CREATE) {
		/*!
		 * \note
		 * These callback can change the handler->state.
		 * So we have to use the "old_state" which stored state before call these callbacks
		 */

		if (handler->created_cb) {
			ret_cb_t cb;
			void *cbdata;

			cb = handler->created_cb;
			cbdata = handler->created_cbdata;

			handler->created_cb = NULL;
			handler->created_cbdata = NULL;

			cb(handler, ret, cbdata);
		} else {
			lb_invoke_event_handler(handler, LB_EVENT_CREATED);
		}
	}

out:
	if (ret == 0 && old_state == DELETE) {
		lb_send_delete(handler, handler->created_cb, handler->created_cbdata);

		/*!
		 * \note
		 * handler->created_cb = NULL;
		 * handler->created_cbdata = NULL;
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
		.cmd = "pd_created",
		.handler = master_pd_created,
	},
	{
		.cmd = "pd_destroyed",
		.handler = master_pd_destroyed,
	},
	{
		.cmd = "fault_package", /* pkgname, id, function, ret */
		.handler = master_fault_package,
	},
	{
		.cmd = "deleted", /* pkgname, id, timestamp, ret */
		.handler = master_deleted,
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
		.cmd = "scroll",
		.handler = master_hold_scroll,
	},

	{
		.cmd = "update_mode",
		.handler = master_update_mode,
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

		if (packet_get(result, "i", &ret) != 1)
			ErrPrint("Invalid argument\n");
		else
			DbgPrint("Acquire returns: %d\n", ret);
	}

	return;
}

static inline int make_connection(void)
{
	struct packet *packet;
	int ret;

	DbgPrint("Let's making connection!\n");

	s_info.fd = com_core_packet_client_init(CLIENT_SOCKET, 0, s_table);
	if (s_info.fd < 0) {
		ErrPrint("Try this again later\n");
		return LB_STATUS_ERROR_IO;
	}

	packet = packet_create("acquire", "d", util_timestamp());
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

	if (vconf_get_bool(VCONFKEY_MASTER_STARTED, &state) < 0)
		ErrPrint("Unable to get [%s]\n", VCONFKEY_MASTER_STARTED);

	DbgPrint("Master state: %d\n", state);
	if (state == 1 && make_connection() == LB_STATUS_SUCCESS) {
		int ret;
		ret = vconf_ignore_key_changed(VCONFKEY_MASTER_STARTED, master_started_cb);
		DbgPrint("master_started vconf key de-registered [%d]\n", ret);
	}
}

static gboolean timeout_cb(gpointer data)
{
	if (vconf_notify_key_changed(VCONFKEY_MASTER_STARTED, master_started_cb, NULL) < 0)
		ErrPrint("Failed to add vconf for monitoring service state\n");
	else
		DbgPrint("vconf event callback is registered\n");

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

int client_init(void)
{
	com_core_add_event_callback(CONNECTOR_DISCONNECTED, disconnected_cb, NULL);
	com_core_add_event_callback(CONNECTOR_CONNECTED, connected_cb, NULL);
	if (vconf_notify_key_changed(VCONFKEY_MASTER_STARTED, master_started_cb, NULL) < 0)
		ErrPrint("Failed to add vconf for service state\n");
	else
		DbgPrint("vconf event callback is registered\n");

	master_started_cb(NULL, NULL);
	return 0;
}

int client_fd(void)
{
	return s_info.fd;
}

const char *client_addr(void)
{
	return CLIENT_SOCKET;
}

int client_fini(void)
{
	int ret;
	ret = vconf_ignore_key_changed(VCONFKEY_MASTER_STARTED, master_started_cb);
	if (ret < 0)
		DbgPrint("Ignore vconf key: %d\n", ret);
	com_core_del_event_callback(CONNECTOR_DISCONNECTED, disconnected_cb, NULL);
	com_core_del_event_callback(CONNECTOR_CONNECTED, connected_cb, NULL);
	com_core_packet_client_fini(s_info.fd);
	s_info.fd = -1;
	return LB_STATUS_SUCCESS;
}

/* End of a file */
