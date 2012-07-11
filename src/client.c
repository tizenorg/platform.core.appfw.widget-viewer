#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

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
	struct packet *result;

	if (packet_get(packet, "sss", &pkgname, &id, &function) != 3) {
		ErrPrint("Invalid arguments\n");
		result = packet_create_reply(packet, "i", -EINVAL);
		return result;
	}

	lb_invoke_fault_handler("deactivated", pkgname, id, function);
	DbgPrint("%s(%s) is deactivated\n", pkgname, id);

	result = packet_create_reply(packet, "i", 0);
	return result;
}

static struct packet *master_deleted(pid_t pid, int handle, const struct packet *packet)
{
	const char *pkgname;
	const char *id;
	double timestamp;
	struct livebox *handler;
	struct packet *result;

	if (packet_get(packet, "ssd", &pkgname, &id, &timestamp) != 3) {
		ErrPrint("Invalid arguemnt\n");
		result = packet_create_reply(packet, "i", -EINVAL);
		return result;
	}

	handler = lb_find_livebox_by_timestamp(timestamp);
	if (!handler) {
		/*!
		 * \note
		 * This can be happens only if the user delete a livebox
		 * right after create it before receive created event.
		 */
		result = packet_create_reply(packet, "i", -ENOENT);
		return result;
	}

	DbgPrint("[%p] %s(%s) is deleted\n", handler, pkgname, id);
	if (handler->created_cb && !handler->is_created)
		handler->created_cb(handler, -EFAULT, handler->created_cbdata);
	else if (handler->state == CREATE)
		lb_invoke_event_handler(handler, "lb,deleted");

	/* Just try to delete it, if a user didn't remove it from the live box list */
	lb_unref(handler);

	result = packet_create_reply(packet, "i", 0);
	return result;
}

static struct packet *master_lb_updated(pid_t pid, int handle, const struct packet *packet)
{
	const char *pkgname;
	const char *id;
	const char *fbfile;
	struct livebox *handler;
	int lb_w;
	int lb_h;
	double priority;
	struct packet *result;
	int ret;

	ret = packet_get(packet, "sssiid", &pkgname, &id, &fbfile, &lb_w, &lb_h, &priority);
	if (ret != 6) {
		ErrPrint("Invalid argument\n");
		result = packet_create_reply(packet, "i", -EINVAL);
		return result;
	}

	DbgPrint("pkgname: %s, id: %s, fbfile: %s, lb_w: %d, lb_h: %d, priority: %lf\n",
						pkgname, id, fbfile, lb_w, lb_h, priority);

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		result = packet_create_reply(packet, "i", -ENOENT);
		return result;
	}

	if (handler->state != CREATE) {
		/*!
		 * \note
		 * Already deleted by the user.
		 * Don't try to notice anything with this, Just ignore all events
		 * Beacuse the user doesn't wants know about this anymore
		 */
		result = packet_create_reply(packet, "i", 0);
		return result;
	}

	lb_set_priority(handler, priority);

	if (lb_text_lb(handler)) {
		lb_set_size(handler, lb_w, lb_h);
		ret = parse_desc(handler, URI_TO_PATH(id), 0);
		result = packet_create_reply(packet, "i", ret);
		return result;
	}

	if (lb_get_lb_fb(handler)) {
		lb_set_size(handler, lb_w, lb_h);
		lb_set_lb_fb(handler, fbfile);
		ret = fb_sync(lb_get_lb_fb(handler));
		if (ret < 0)
			ErrPrint("Failed to do sync FB (%s - %s)\n", pkgname, util_basename(URI_TO_PATH(id)));
	} else {
		lb_set_size(handler, lb_w, lb_h);
		ret = 0;
	}

	if (ret == 0)
		lb_invoke_event_handler(handler, "lb,updated");

	result = packet_create_reply(packet, "i", ret);
	return result;
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
	struct packet *result;

	ret = packet_get(packet, "ssssii", &pkgname, &id, &descfile, &fbfile, &pd_w, &pd_h);
	if (ret != 6) {
		ErrPrint("Invalid argument\n");
		result = packet_create_reply(packet, "i", -EINVAL);
		return result;
	}

	handler = lb_find_livebox(pkgname, id);
	if (!handler) {
		result = packet_create_reply(packet, "i", -ENOENT);
		return result;
	}

	if (handler->state != CREATE) {
		/*!
		 * \note
		 * This handler is already deleted by the user.
		 * So don't try to notice anything about this anymore.
		 * Just ignore all events.
		 */
		result = packet_create_reply(packet, "i", 0);
		return result;
	}

	lb_set_pdsize(handler, pd_w, pd_h);

	if (lb_text_pd(handler)) {
		ret = parse_desc(handler, URI_TO_PATH(id), 1);
	} else {
		if (lb_set_pd_fb(handler, fbfile) == 0) {
			ret = fb_create_buffer(lb_get_pd_fb(handler));
			if (ret < 0) {
				ErrPrint("Error: %s\n", strerror(ret));
				result = packet_create_reply(packet, "i", ret);
				return result;
			}
		}

		ret = fb_sync(lb_get_pd_fb(handler));
		if (ret < 0) {
			ErrPrint("Failed to do sync FB (%s - %s)\n", pkgname, util_basename(URI_TO_PATH(id)));
			result = packet_create_reply(packet, "i", ret);
			return result;
		}

		lb_invoke_event_handler(handler, "pd,updated");
		ret = 0;
	}

	result = packet_create_reply(packet, "i", ret);
	return result;
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

	double timestamp;
	int auto_launch;
	double priority;
	int size_list;
	int user;
	int pinup_supported;
	enum lb_type lb_type;
	enum pd_type pd_type;
	double period;
	struct packet *result;

	int ret;

	ret = packet_get(packet, "dsssiiiissssidiiiiid",
			&timestamp,
			&pkgname, &id, &content,
			&lb_w, &lb_h, &pd_w, &pd_h,
			&cluster, &category, &lb_fname, &pd_fname,
			&auto_launch, &priority, &size_list, &user, &pinup_supported,
			&lb_type, &pd_type, &period);
	if (ret != 20) {
		ErrPrint("Invalid argument\n");
		result = packet_create_reply(packet, "i", -EINVAL);
		return result;
	}

	ErrPrint("[%lf] pkgname: %s, id: %s, content: %s, "
		"pd_w: %d, pd_h: %d, lb_w: %d, lb_h: %d, "
		"cluster: %s, category: %s, lb_fname: \"%s\", pd_fname: \"%s\", "
		"auto_launch: %d, priority: %lf, size_list: %d, user: %d, pinup: %d, "
		"lb_type: %d, pd_type: %d, period: %lf\n",
		timestamp, pkgname, id, content,
		pd_w, pd_h, lb_w, lb_h,
		cluster, category, lb_fname, pd_fname,
		auto_launch, priority, size_list, user, pinup_supported,
		lb_type, pd_type, period);

	handler = lb_find_livebox_by_timestamp(timestamp);
	if (!handler) {
		handler = lb_new_livebox(pkgname, id, timestamp);
		if (!handler) {
			ErrPrint("Failed to create a new livebox\n");
			result = packet_create_reply(packet, "i", -EFAULT);
			return result;
		}
	} else {
		lb_set_id(handler, id);

		if (handler->state != CREATE) {
			lb_send_delete(handler);
			result = packet_create_reply(packet, "i", 0);
			return result;
		}
	}

	if (handler->is_created == 1) {
		ErrPrint("Already created: timestamp[%lf] "
			"pkgname[%s], id[%s] content[%s] "
			"cluster[%s] category[%s] lb_fname[%s] pd_fname[%s]\n",
				timestamp,
				pkgname, id,
				content,
				cluster, category,
				lb_fname, pd_fname);

		result = packet_create_reply(packet, "i", 0);
		return result;
	}

	lb_set_size(handler, lb_w, lb_h);
	handler->lb.type = lb_type;

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
			ErrPrint("Failed to do sync FB (%s - %s)\n", pkgname, util_basename(URI_TO_PATH(id)));
		break;
	case _LB_TYPE_TEXT:
		lb_set_text_lb(handler);
		break;
	default:
		break;
	}

	handler->pd.type = pd_type;
	lb_set_pdsize(handler, pd_w, pd_h);
	switch (pd_type) {
	case _PD_TYPE_SCRIPT:
	case _PD_TYPE_BUFFER:
		if (!strlen(pd_fname))
			break;

		lb_set_pd_fb(handler, pd_fname);
		ret = fb_sync(lb_get_pd_fb(handler));
		if (ret < 0)
			ErrPrint("Failed to do sync FB (%s - %s)\n", pkgname, util_basename(URI_TO_PATH(id)));
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

	lb_set_user(handler, user);

	lb_set_auto_launch(handler, auto_launch);
	lb_set_pinup(handler, pinup_supported);

	lb_set_period(handler, period);

	if (handler->created_cb) {
		handler->created_cb(handler, 0, handler->created_cbdata);
		handler->is_created = 1;
	} else {
		lb_invoke_event_handler(handler, "lb,created");
	}

	return packet_create_reply(packet, "i", 0);
}

static struct method s_table[] = {
	{
		.cmd = "fault_packet", /* pkgname, id, function, ret */
		.handler = master_fault_package,
	},
	{
		.cmd = "deleted", /* pkgname, id, timestamp, ret */
		.handler = master_deleted,
	},
	{
		.cmd = "lb_updated", /* pkgname, id, lb_w, lb_h, priority, ret */
		.handler = master_lb_updated,
	},
	{
		.cmd = "pd_updated", /* pkgname, id, descfile, pd_w, pd_h, ret */
		.handler = master_pd_updated,
	},
	{
		.cmd = "created", /* timestamp, pkgname, id, content, lb_w, lb_h, pd_w, pd_h, cluster, category, lb_file, pd_file, auto_launch, priority, size_list, is_user, pinup_supported, text_lb, text_pd, period, ret */
		.handler = master_created,
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

	s_info.fd = com_core_packet_client_init("/tmp/.live.socket", 0, s_table);
	if (s_info.fd < 0) {
		s_info.reconnector = g_timeout_add(10000, connector_cb, NULL); /*!< After 10 secs later, try to connect again */
		if (s_info.reconnector == 0)
			ErrPrint("Failed to fire the reconnector\n");

		ErrPrint("Try this again 10 secs later\n");
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
	s_info.fd = -1; /*!< Disconnected */

	if (s_info.reconnector > 0) {
		DbgPrint("Reconnector already fired\n");
		return 0;
	}

	s_info.reconnector = g_timeout_add(10000, connector_cb, NULL); /*!< After 10 secs later, try to connect again */
	if (s_info.reconnector == 0) {
		ErrPrint("Failed to fire the reconnector\n");
		make_connection();
	}

	lb_invoke_fault_handler("provider,disconnected", MASTER_PKGNAME, "default", "disconnected");
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
	return "/tmp/.live.socket";
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


