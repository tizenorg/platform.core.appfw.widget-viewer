/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.tizenopensource.org/license
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

#include <packet.h>
#include <com-core.h>
#include <com-core_packet.h>

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

static inline void make_connection(void);

static struct info {
	int fd;
	guint reconnector;
} s_info = {
	.fd = -1,
	.reconnector = 0,
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
	DbgPrint("%s(%s) is deactivated\n", pkgname, id);
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
	int pinup;

	ret = packet_get(packet, "iisss", &ret, &pinup, &pkgname, &id, &content);
	if (ret != 5) {
		ErrPrint("Invalid argument\n");
		ret = -EINVAL;
		goto out;
	}

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ret = -ENOENT;
		goto out;
	}

	if (ret == 0) {
		new_content = strdup(content);
		if (new_content) {
			free(handler->content);
			handler->content = new_content;
			handler->is_pinned_up = pinup;
		} else {
			ErrPrint("Heap: %s\n", strerror(errno));
			ret = -ENOMEM;
		}
	}

	if (handler->pinup_cb) {
		handler->pinup_cb(handler, ret, handler->pinup_cbdata);

		handler->pinup_cb = NULL; /*!< Reset pinup cb */
		handler->pinup_cbdata = NULL;
	} else {
		lb_invoke_event_handler(handler, LB_EVENT_PINUP_CHANGED);
	}

	ret = 0;
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

		DbgPrint("Call the created cb with -ECANCELED\n");
		handler->created_cb(handler, -ECANCELED, handler->created_cbdata);
		handler->created_cb = NULL;
		handler->created_cbdata = NULL;
	} else if (handler->id) {
		if (handler->deleted_cb) {
			DbgPrint("Call the deleted cb\n");
			handler->deleted_cb(handler, 0, handler->deleted_cbdata);

			handler->deleted_cb = NULL;
			handler->deleted_cbdata = NULL;
		} else {
			DbgPrint("Call the lb,deleted\n");
			lb_invoke_event_handler(handler, LB_EVENT_DELETED);
		}
	}

	DbgPrint("[%p] %s(%s) is deleted\n", handler, pkgname, id);

	/* Just try to delete it, if a user didn't remove it from the live box list */
	lb_unref(handler);

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
		ret = -EINVAL;
		goto out;
	}

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ret = -ENOENT;
		goto out;
	}

	if (handler->state != CREATE) {
		/*!
		 * \note
		 * Already deleted by the user.
		 * Don't try to notice anything with this, Just ignore all events
		 * Beacuse the user doesn't wants know about this anymore
		 */
		ret = -EPERM;
		goto out;
	}

	lb_set_priority(handler, priority);
	lb_set_content(handler, content);
	lb_set_title(handler, title);

	if (lb_text_lb(handler)) {
		lb_set_size(handler, lb_w, lb_h);
		ret = parse_desc(handler, livebox_filename(handler), 0);
		/*!
		 * \note
		 * DESC parser will call the "text event callback".
		 */
		goto out;
	} else if (lb_get_lb_fb(handler)) {
		lb_set_size(handler, lb_w, lb_h);
		lb_set_lb_fb(handler, fbfile);
		ret = fb_sync(lb_get_lb_fb(handler));
		if (ret < 0)
			ErrPrint("Failed to do sync FB (%s - %s)\n", pkgname, util_basename(util_uri_to_path(id)));
	} else {
		lb_set_size(handler, lb_w, lb_h);
		ret = 0;
	}

	if (ret == 0)
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
		ret = -ENOENT;
		goto out;
	}

	if (handler->state != CREATE) {
		ret = -EPERM;
		goto out;
	}

	lb_set_pdsize(handler, width, height);
	if (lb_text_pd(handler)) {
		DbgPrint("Text TYPE does not need to handle this\n");
	} else {
		(void)lb_set_pd_fb(handler, buf_id);
		ret = fb_sync(lb_get_pd_fb(handler));
		if (ret < 0) {
			ErrPrint("Failed to do sync FB (%s - %s)\n",
					pkgname,
					util_basename(util_uri_to_path(id)));
		}
	}

	handler->is_pd_created = (status == 0);

	if (handler->pd_created_cb) {
		DbgPrint("pd_created_cb (%s) - %d\n", buf_id, status);
		handler->pd_created_cb(handler, status, handler->pd_created_cbdata);

		handler->pd_created_cb = NULL;
		handler->pd_created_cbdata = NULL;
	} else if (status == 0) {
		DbgPrint("LB_EVENT_PD_CREATED (%s) - %d\n", buf_id, status);
		lb_invoke_event_handler(handler, LB_EVENT_PD_CREATED);
	}

	ret = 0;
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
		ret = -ENOENT;
		goto out;
	}

	if (handler->state != CREATE) {
		ret = -EPERM;
		goto out;
	}

	handler->is_pd_created = 0;

	if (handler->pd_destroyed_cb) {
		DbgPrint("Invoke the PD Destroyed CB\n");
		handler->pd_destroyed_cb(handler, status, handler->pd_destroyed_cbdata);

		handler->pd_destroyed_cb = NULL;
		handler->pd_destroyed_cbdata = NULL;
	} else if (status == 0) {
		DbgPrint("Invoke the LB_EVENT_PD_DESTROYED event\n");
		lb_invoke_event_handler(handler, LB_EVENT_PD_DESTROYED);
	}

	ret = 0;
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
		ret = -EINVAL;
		goto out;
	}

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ret = -ENOENT;
		goto out;
	}

	if (handler->state != CREATE) {
		/*!
		 * \note
		 * This handler is already deleted by the user.
		 * So don't try to notice anything about this anymore.
		 * Just ignore all events.
		 */
		ret = -EPERM;
		goto out;
	}

	lb_set_pdsize(handler, pd_w, pd_h);

	if (lb_text_pd(handler)) {
		ret = parse_desc(handler, descfile, 1);
	} else {
		(void)lb_set_pd_fb(handler, fbfile);

		ret = fb_sync(lb_get_pd_fb(handler));
		if (ret < 0) {
			ErrPrint("Failed to do sync FB (%s - %s)\n",
					pkgname,
					util_basename(util_uri_to_path(id)));
			goto out;
		}

		lb_invoke_event_handler(handler, LB_EVENT_PD_UPDATED);
		ret = 0;
	}

out:
	return NULL;
}

static struct packet *master_size_changed(pid_t pid, int handle, const struct packet *packet)
{
	struct livebox *handler;
	const char *pkgname;
	const char *id;
	int status;
	int ret;
	int w;
	int h;
	int is_pd;

	if (!packet) {
		ErrPrint("Invalid packet\n");
		ret = -EINVAL;
		goto out;
	}

	ret = packet_get(packet, "ssiiii", &pkgname, &id, &is_pd, &w, &h, &status);
	if (ret != 6) {
		ErrPrint("Invalid argument\n");
		ret = -EINVAL;
		goto out;
	}

	DbgPrint("Size is changed: %dx%d (%s)\n", w, h, id);

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ErrPrint("Livebox(%s - %s) is not found\n", pkgname, id);
		ret = -ENOENT;
		goto out;
	}

	if (handler->state != CREATE) {
		ErrPrint("Hander is not created yet\n");
		ret = -EPERM;
		goto out;
	}

	if (is_pd) {
		DbgPrint("PD is resized\n");
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
			ErrPrint("This is not possible. PD Size is changed but the return value is not ZERO\n");
		}
	} else {
		if (status == 0) {
			DbgPrint("LB is successfully resized (%dx%d)\n", w, h);
			lb_set_size(handler, w, h);

			/*!
			 * \NOTE
			 * If there is a created LB FB, 
			 * Update it too.
			 */
			if (lb_get_lb_fb(handler))
				lb_set_lb_fb(handler, id);

			/*!
			 * \NOTE
			 * I cannot believe client.
			 * So I added some log before & after call the user callback.
			 */
			if (handler->size_changed_cb) {
				handler->size_changed_cb(handler, status, handler->size_cbdata);

				handler->size_changed_cb = NULL;
				handler->size_cbdata = NULL;
			} else {
				lb_invoke_event_handler(handler, LB_EVENT_LB_SIZE_CHANGED);
			}
		} else {
			DbgPrint("LB is not resized: %dx%d (%d)\n", w, h, status);
			if (handler->size_changed_cb) {
				handler->size_changed_cb(handler, status, handler->size_cbdata);

				handler->size_changed_cb = NULL;
				handler->size_cbdata = NULL;
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
		ret = -EINVAL;
		goto out;
	}

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ErrPrint("Livebox(%s - %s) is not found\n", pkgname, id);
		ret = -ENOENT;
		goto out;
	}

	if (handler->state != CREATE) {
		ret = -EPERM;
		goto out;
	}

	DbgPrint("Update period is changed? %lf (%d)\n", period, status);
	if (status == 0)
		lb_set_period(handler, period);

	if (handler->period_changed_cb) {
		handler->period_changed_cb(handler, status, handler->group_cbdata);

		handler->period_changed_cb = NULL;
		handler->period_cbdata = NULL;
	} else {
		lb_invoke_event_handler(handler, LB_EVENT_PERIOD_CHANGED);
	}

	ret = 0;

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
		ret = -EINVAL;
		goto out;
	}

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		ret = -ENOENT;
		goto out;
	}

	if (handler->state != CREATE) {
		/*!
		 * \note
		 * Do no access this handler,
		 * You cannot believe this handler anymore.
		 */
		ret = -EPERM;
		goto out;
	}

	DbgPrint("Group is changed? [%s] / [%s] (%d)\n", cluster, category, status);
	if (status == 0)
		lb_set_group(handler, cluster, category);

	if (handler->group_changed_cb) {
		handler->group_changed_cb(handler, status, handler->group_cbdata);

		handler->group_changed_cb = NULL;
		handler->group_cbdata = NULL;
	} else {
		lb_invoke_event_handler(handler, LB_EVENT_GROUP_CHANGED);
	}

	ret = 0;

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
		ret = -EINVAL;
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
			ret = -EFAULT;
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
				ret = -EINVAL;
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

			ret = -EALREADY;
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
			DbgPrint("Invoke the created_cb\n");
			handler->created_cb(handler, ret, handler->created_cbdata);

			handler->created_cb = NULL;
			handler->created_cbdata = NULL;
		} else {
			DbgPrint("Invoke the lb,created\n");
			lb_invoke_event_handler(handler, LB_EVENT_CREATED);
		}
	}

out:
	if (ret == 0 && old_state == DELETE) {
		DbgPrint("Send the delete request\n");
		lb_send_delete(handler, handler->created_cb, handler->created_cbdata);

		/*!
		 * \note
		 * handler->created_cb = NULL;
		 * handler->created_cbdata = NULL;
		 *
		 * Do not clear this to use this from the deleted event callback.
		 * if this value is not cleared when the deleted event callback check it,
		 * it means that the created function is not called yet.
		 * Then the call the deleted event callback with -ECANCELED errno.
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

static gboolean connector_cb(gpointer user_data)
{
	s_info.reconnector = 0;

	if (s_info.fd > 0) {
		DbgPrint("Connection is already made\n");
		return FALSE;
	}

	make_connection();
	return FALSE;
}

static inline void make_connection(void)
{
	struct packet *packet;
	int ret;

	DbgPrint("Let's making connection!\n");

	s_info.fd = com_core_packet_client_init(CLIENT_SOCKET, 0, s_table);
	if (s_info.fd < 0) {
		/*!< After 10 secs later, try to connect again */
		s_info.reconnector = g_timeout_add(RECONNECT_PERIOD, connector_cb, NULL);
		if (s_info.reconnector == 0)
			ErrPrint("Failed to fire the reconnector\n");

		ErrPrint("Try this again A sec later\n");
		return;
	}

	packet = packet_create("acquire", "d", util_timestamp());
	if (!packet) {
		com_core_packet_client_fini(s_info.fd);
		s_info.fd = -1;
		return;
	}

	ret = master_rpc_async_request(NULL, packet, 1, acquire_cb, NULL);
	if (ret < 0) {
		ErrPrint("Master RPC returns %d\n", ret);
		com_core_packet_client_fini(s_info.fd);
		s_info.fd = -1;
	}

}

static int connected_cb(int handle, void *data)
{
	master_rpc_check_and_fire_consumer();
	return 0;
}

static int disconnected_cb(int handle, void *data)
{
	if (s_info.fd != handle) {
		/*!< This handle is not my favor */
		return 0;
	}

	s_info.fd = -1; /*!< Disconnected */

	if (s_info.reconnector > 0) {
		DbgPrint("Reconnector already fired\n");
		return 0;
	}

	/*!< After 10 secs later, try to connect again */
	s_info.reconnector = g_timeout_add(RECONNECT_PERIOD, connector_cb, NULL);
	if (s_info.reconnector == 0) {
		ErrPrint("Failed to fire the reconnector\n");
		make_connection();
	}

	master_rpc_clear_all_request();
	lb_invoke_fault_handler(LB_FAULT_PROVIDER_DISCONNECTED, MASTER_PKGNAME, "default", "disconnected");

	lb_delete_all();
	return 0;
}

int client_init(void)
{
	com_core_add_event_callback(CONNECTOR_DISCONNECTED, disconnected_cb, NULL);
	com_core_add_event_callback(CONNECTOR_CONNECTED, connected_cb, NULL);
	make_connection();
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
	com_core_packet_client_fini(s_info.fd);
	com_core_del_event_callback(CONNECTOR_DISCONNECTED, disconnected_cb, NULL);
	com_core_del_event_callback(CONNECTOR_CONNECTED, connected_cb, NULL);
	s_info.fd = -1;
	return 0;
}

/* End of a file */
