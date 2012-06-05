#include <stdio.h>
#include <gio/gio.h>
#include <libgen.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <dlog.h>

#include "debug.h"
#include "dlist.h"
#include "livebox.h"
#include "livebox_internal.h"
#include "dbus.h"
#include "master_rpc.h"

#define DEFAULT_TTL 10

struct packet {
	int ttl;
	char *funcname;
	GVariant *param;
	struct livebox *handler;
	void (*ret_cb)(struct livebox *handler, GVariant *result, void *data);
	void *data;
};

int errno;

static struct {
	guint cmd_timer;
	struct dlist *cmd_list;
} s_info = {
	.cmd_timer = 0,
	.cmd_list = NULL,
};

static void done_cb(GDBusProxy *proxy, GAsyncResult *res, void *data);

static inline struct packet *pop_packet(void)
{
	struct dlist *l;
	struct packet *packet;

	l = dlist_nth(s_info.cmd_list, 0);
	if (!l)
		return NULL;

	packet = dlist_data(l);
	s_info.cmd_list = dlist_remove(s_info.cmd_list, l);
	return packet;
}

static inline struct packet *create_packet(struct livebox *handler, const char *funcname, GVariant *param)
{
	struct packet *packet;

	packet = malloc(sizeof(*packet));
	if (!packet) {
		ErrPrint("Failed to allocate mem for packet\n");
		return NULL;
	}

	packet->funcname = strdup(funcname);
	if (!packet->funcname) {
		ErrPrint("Failed to allocate mem for funcname - %s\n", funcname);
		free(packet);
		return NULL;
	}

	packet->handler = lb_ref(handler);
	packet->param = g_variant_ref(param);
	return packet;
}

static inline void destroy_packet(struct packet *packet)
{
	g_variant_unref(packet->param);
	lb_unref(packet->handler);
	free(packet->funcname);
	free(packet);
}

static gboolean cmd_consumer(gpointer user_data)
{
	struct packet *packet;

	if (!dbus_proxy())
		return TRUE;

	packet = pop_packet();
	if (!packet) {
		s_info.cmd_timer = 0;
		return FALSE;
	}

	/*!
	 * \NOTE:
	 * Item will be deleted in the "done_cb"
	 *
	 * item->param be release by the g_dbus_proxy_call
	 * so to use it again from the done_cb function,
	 * increate the reference counter of the item->param
	 */
	g_dbus_proxy_call(dbus_proxy(),
		packet->funcname,
		packet->param,
		G_DBUS_CALL_FLAGS_NO_AUTO_START,
		-1, NULL, (GAsyncReadyCallback)done_cb, packet);

	return TRUE;
}

static inline void check_and_fire_consumer(void)
{
	if (!s_info.cmd_list || s_info.cmd_timer)
		return;

	s_info.cmd_timer = g_timeout_add(10, cmd_consumer, NULL);
	if (!s_info.cmd_timer)
		ErrPrint("Failed to add timer\n");
}

static inline void prepend_packet(struct packet *packet)
{
	DbgPrint("Re-send a packet: %s (ttl: %d)\n", packet->funcname, packet->ttl);
	s_info.cmd_list = dlist_prepend(s_info.cmd_list, packet);
	check_and_fire_consumer();
}

static void done_cb(GDBusProxy *proxy, GAsyncResult *res, void *data)
{
	GVariant *result;
	GError *err;
	struct packet *packet;

	packet = data;

	err = NULL;
	result = g_dbus_proxy_call_finish(proxy, res, &err);
	if (!result) {
		if (err) {
			ErrPrint("%s Error: %s\n", packet->funcname, err->message);
			g_error_free(err);
		}

		/*! \NOTE:
		 * Release resource even if
		 * we failed to finish the method call
		 */
		packet->ttl--;
		if (packet->ttl > 0) {
			prepend_packet(packet);
			return;
		}

		if (packet->ret_cb)
			packet->ret_cb(packet->handler, NULL, packet->data);

		goto out;
	}

	if (packet->ret_cb)
		packet->ret_cb(packet->handler, result, packet->data);
	else
		g_variant_unref(result);

out:
	destroy_packet(packet);
}

static inline void push_packet(struct packet *packet)
{
	s_info.cmd_list = dlist_append(s_info.cmd_list, packet);
	check_and_fire_consumer();
}

/*!
 * \note
 * "handler" could be NULL
 */
int master_rpc_async_request(struct livebox *handler, const char *funcname, GVariant *param, void (*ret_cb)(struct livebox *handler, GVariant *result, void *data), void *data)
{
	struct packet *packet;

	packet = create_packet(handler, funcname, param);
	if (!packet) {
		if (ret_cb)
			ret_cb(handler, NULL, data);

		g_variant_unref(param);
		return -EFAULT;
	}

	packet->ret_cb = ret_cb;
	packet->data = data;
	packet->ttl = DEFAULT_TTL;

	push_packet(packet);
	return 0;
}

int master_rpc_sync_request(struct livebox *handler, const char *func, GVariant *param)
{
	GVariant *result;
	GError *err;
	int ret;

	err = NULL;
	result = g_dbus_proxy_call_sync(dbus_proxy(), func, param, G_DBUS_CALL_FLAGS_NO_AUTO_START, -1, NULL, &err);
	if (!result) {
		if (err) {
			ErrPrint("acquire Error: %s\n", err->message);
			g_error_free(err);
		}

		ErrPrint("Failed to send 'acquire'\n");
		return -EIO;
	}

	g_variant_get(result, "(i)", &ret);
	g_variant_unref(result);

	return ret;
}

/* End of a file */
