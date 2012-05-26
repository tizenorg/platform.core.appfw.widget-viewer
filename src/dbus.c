#include <stdio.h>
#include <stdlib.h> /* free */
#include <errno.h>
#include <string.h> /* strdup */
#include <libgen.h>

#include <gio/gio.h>

#include <dlog.h>

#include "debug.h"
#include "dlist.h"
#include "fb_file.h"
#include "livebox.h"
#include "livebox0.h"
#include "dbus.h"
#include "desc_parser.h"

static struct info {
	GDBusNodeInfo *node_info;
	GDBusProxy *proxy;
	struct dlist *cmd_list;
	guint cmd_timer;
	guint recon_timer;
	guint reg_id;
	const gchar *xml_data;
} s_info = {
	.proxy = NULL,
	.cmd_list = NULL,
	.cmd_timer = 0,
	.recon_timer = 0,
	.reg_id = 0,
	.node_info = NULL,
	.xml_data = "<node>"
	"<interface name='" SERVICE_INTERFACE "'>"
	" <method name='fault_package'>"
	"  <arg type='s' name='pkgname' direction='in' />"
	"  <arg type='s' name='filename' direction='in' />"
	"  <arg type='s' name='function' direction='in' />"
	"  <arg type='i' name='result' direction='out' />"
	" </method>"
	" <method name='deleted'>"
	"  <arg type='s' name='pkgname' direction='in' />"
	"  <arg type='s' name='filename' direction='in' />"
	"  <arg type='i' name='result' direction='out' />"
	" </method>"
	" <method name='lb_updated'>"
	"  <arg type='s' name='pkgname' direction='in' />"
	"  <arg type='s' name='filename' direction='in' />"
	"  <arg type='i' name='lb_w' direction='in' />"
	"  <arg type='i' name='lb_h' direction='in' />"
	"  <arg type='d' name='priority' direction='in' />"
	"  <arg type='i' name='result' direction='out' />"
	" </method>"
	" <method name='pd_updated'>"
	"  <arg type='s' name='pkgname' direction='in' />"
	"  <arg type='s' name='filename' direction='in' />"
	"  <arg type='s' name='descfile' direction='in' />"
	"  <arg type='i' name='pd_w' direction='in' />"
	"  <arg type='i' name='pd_h' direction='in' />"
	"  <arg type='i' name='result' direction='out' />"
	" </method>"
	" <method name='created'>"
	"  <arg type='d' name='timestamp' direction='in' />"
	"  <arg type='s' name='pkgname' direction='in' />"
	"  <arg type='s' name='filename' direction='in' />"
	"  <arg type='s' name='content' direction='in' />"
	"  <arg type='i' name='lb_w' direction='in' />"
	"  <arg type='i' name='lb_h' direction='in' />"
	"  <arg type='i' name='pd_w' direction='in' />"
	"  <arg type='i' name='pd_h' direction='in' />"
	"  <arg type='s' name='cluster' direction='in' />"
	"  <arg type='s' name='category' direction='in' />"
	"  <arg type='s' name='lb_file' direction='in' />"
	"  <arg type='s' name='pd_file' direction='in' />"
	"  <arg type='i' name='auto_launch' direction='in' />"
	"  <arg type='d' name='priority' direction='in' />"
	"  <arg type='i' name='size_list' direction='in' />"
	"  <arg type='i' name='is_user' direction='in' />"
	"  <arg type='i' name='pinup_supported' direction='in' />"
	"  <arg type='i' name='text_lb' direction='in' />"
	"  <arg type='i' name='text_pd' direction='in' />"
	"  <arg type='d' name='period' direction='in' />"
	"  <arg type='i' name='result' direction='out' />"
	" </method>"
	"</interface>"
	"</node>",
};

struct cmd_item {
	char *funcname;
	GVariant *param;
	struct livebox *handler;
	void (*ret_cb)(struct livebox *handler, int ret, void *data);
	void *data;
};

GDBusProxy *dbus_get_proxy(void)
{
	return s_info.proxy;
}

static inline int send_acquire(void)
{
	GVariant *result;
	GError *err;

	err = NULL;
	result = g_dbus_proxy_call_sync(s_info.proxy, "acquire", g_variant_new("(i)", getpid()),
					G_DBUS_CALL_FLAGS_NO_AUTO_START, -1, NULL, &err);
	if (!result) {
		if (err) {
			ErrPrint("Error: %s\n", err->message);
			g_error_free(err);
		}

		ErrPrint("Failed to send 'acquire'\n");
		return -EIO;
	}

	g_variant_unref(result);
	return 0;
}

static inline int send_release(void)
{
	GVariant *result;
	GError *err;

	err = NULL;
	result = g_dbus_proxy_call_sync(s_info.proxy, "release", g_variant_new("(i)", getpid()),
					G_DBUS_CALL_FLAGS_NO_AUTO_START, -1, NULL, &err);

	if (!result) {
		if (err) {
			ErrPrint("Error: %s\n", err->message);
			g_error_free(err);
		}

		return -EIO;
	}

	g_variant_unref(result);
	return 0;
}

static void on_signal(GDBusProxy *proxy, gchar *sender, gchar *signame, GVariant *param, gpointer data)
{
	DbgPrint("Sender: %s\n", sender);
	DbgPrint("SigName: %s\n", signame);
}

static void done_cb(GDBusProxy *proxy, GAsyncResult *res, void *data)
{
	GVariant *result;
	GError *err;
	int r;
	struct cmd_item *item;

	item = data;

	err = NULL;
	result = g_dbus_proxy_call_finish(proxy, res, &err);
	if (!result) {
		if (err) {
			DbgPrint("Error: %s\n", err->message);
			g_error_free(err);
		}

		/*! \NOTE:
		 * Release resource even if
		 * we failed to finish the method call
		 */
		goto out;
	}

	g_variant_get(result, "(i)", &r);
	g_variant_unref(result);

	DbgPrint("%s Returns: %d\n", item->funcname, r);
	if (item->ret_cb)
		item->ret_cb(item->handler, r, item->data);

out:
	lb_unref(item->handler);
	/* Decreate the item->param's refernece counter now. */
	g_variant_unref(item->param);
	free(item->funcname);
	free(item);
}

static gboolean dbus_reconnect_cb(gpointer user_data)
{
	dbus_init();
	s_info.recon_timer = 0;
	return FALSE;
}

static gboolean cmd_consumer(gpointer user_data)
{
	struct dlist *l;
	struct cmd_item *item;

	if (!s_info.proxy) {
		DbgPrint("Proxy is not valid yet\n");
		s_info.cmd_timer = 0;
		return FALSE;
	}

	l = dlist_nth(s_info.cmd_list, 0);
	if (l) {
		item = dlist_data(l);

		/*!
		 * \NOTE:
		 * Item will be deleted in the "done_cb"
		 *
		 * item->param be release by the g_dbus_proxy_call
		 * so to use it again from the done_cb function,
		 * increate the reference counter of the item->param
		 */
		g_dbus_proxy_call(s_info.proxy,
			item->funcname,
			g_variant_ref(item->param),
			G_DBUS_CALL_FLAGS_NO_AUTO_START,
			-1, NULL, (GAsyncReadyCallback)done_cb, item);

		s_info.cmd_list = dlist_remove(s_info.cmd_list, l);
	}

	if (!s_info.cmd_list) {
		s_info.cmd_timer = 0;
		return FALSE;
	}

	return TRUE;
}

static void method_fault_package(GDBusMethodInvocation *inv, GVariant *param)
{
	const char *pkgname;
	const char *filename;
	const char *function;

	g_variant_get(param, "(&s&s&s)", &pkgname, &filename, &function);

	lb_invoke_fault_handler("deactivated", pkgname, filename, function);

	g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", 0));
}

static void method_pd_updated(GDBusMethodInvocation *inv, GVariant *param)
{
	const char *pkgname;
	const char *filename;
	const char *descfile;
	int ret;
	struct livebox *handler;
	int pd_w;
	int pd_h;

	g_variant_get(param, "(&s&s&sii)",
			&pkgname, &filename, &descfile, &pd_w, &pd_h);

	handler = lb_find_livebox(pkgname, filename);
	if (!handler) {
		ret = -ENOENT;
		goto out;
	}

	if (handler->deleted != NOT_DELETED) {
		/*!
		 * \note
		 * This handler is already deleted by the user.
		 * So don't try to notice anything about this anymore.
		 * Just ignore all events.
		 */
		ret = 0;
		goto out;
	}

	DbgPrint("PD is updated [%dx%d]\n", pd_w, pd_h);
	lb_set_pdsize(handler, pd_w, pd_h);

	if (lb_text_pd(handler)) {
		ret = parse_desc(handler, filename, 1);
	} else {
		if (lb_get_pd_fb(handler)) {
			lb_update_pd_fb(handler, pd_w, pd_h);
			fb_sync(lb_get_pd_fb(handler));
		}

		lb_invoke_event_handler(handler, "pd,updated");
		ret = 0;
	}
out:
	g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", ret));
}

static void method_lb_updated(GDBusMethodInvocation *inv, GVariant *param)
{
	const char *pkgname;
	const char *filename;
	int ret;
	struct livebox *handler;
	int lb_w;
	int lb_h;
	double priority;

	g_variant_get(param, "(&s&siid)",
			&pkgname, &filename,
			&lb_w, &lb_h, &priority);

	handler = lb_find_livebox(pkgname, filename);
	if (!handler) {
		ret = -ENOENT;
		goto out;
	}

	if (handler->deleted != NOT_DELETED) {
		/*!
		 * \note
		 * Already deleted by the user.
		 * Don't try to notice anything with this, Just ignore all events
		 * Beacuse the user doesn't wants know about this anymore
		 */
		ret = 0;
		goto out;
	}

	lb_set_priority(handler, priority);
	lb_set_size(handler, lb_w, lb_h);

	if (lb_text_lb(handler)) {
		ret = parse_desc(handler, filename, 0);
	} else {
		if (lb_get_lb_fb(handler)) {
			lb_update_lb_fb(handler, lb_w, lb_h);
			fb_sync(lb_get_lb_fb(handler));
		}

		lb_invoke_event_handler(handler, "lb,updated");
		ret = 0;
	}

out:
	g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", ret));
}

static void method_created(GDBusMethodInvocation *inv, GVariant *param)
{
	struct livebox *handler;
	const char *pkgname;
	const char *filename;
	int lb_w;
	int lb_h;
	int pd_w;
	int pd_h;
	const char *content;
	const char *cluster;
	const char *category;
	const char *lb_fname;
	const char *pd_fname;
	double timestamp;
	int auto_launch;
	int ret;
	double priority;
	int size_list;
	int user;
	int pinup_supported;
	int text_lb;
	int text_pd;
	double period;

	g_variant_get(param, "(d&s&s&siiii&s&s&s&sidiiiiid)",
			&timestamp,
			&pkgname, &filename, &content,
			&lb_w, &lb_h, &pd_w, &pd_h,
			&cluster, &category, &lb_fname, &pd_fname,
			&auto_launch, &priority, &size_list, &user, &pinup_supported,
			&text_lb, &text_pd, &period);

	DbgPrint("[%lf] pkgname: %s, filename: %s, content: %s, "
		"pd_w: %d, pd_h: %d, lb_w: %d, lb_h: %d, "
		"cluster: %s, category: %s, lb_fname: %s, pd_fname: %s"
		"auto_launch: %d, priority: %lf, size_list: %d, user: %d, pinup: %d"
		"text_lb: %d, text_pd: %d, period: %lf\n",
		timestamp, pkgname, filename, content,
		pd_w, pd_h, lb_w, lb_h,
		cluster, category, lb_fname, pd_fname,
		auto_launch, priority, size_list, user, pinup_supported,
		text_lb, text_pd, period);

	handler = lb_find_livebox_by_timestamp(timestamp);
	if (!handler) {
		DbgPrint("create a new livebox instance\n");
		handler = lb_new_livebox(pkgname, filename);
		if (!handler) {
			ErrPrint("Failed to create a new livebox\n");
			ret = -EFAULT;
			goto out;
		}
	} else {
		lb_set_filename(handler, filename);

		if (handler->deleted != NOT_DELETED) {
			/*!
			 * \note
			 * before get this created event,
			 * user delete this already.
			 * So user doesn't wants to know about this anymore
			 * just ignore created events
			 */

			if (handler->deleted == DELETE_ALL)
				lb_send_delete(handler);

			/*!
			 * \note
			 * This will make the method_delete function could not 
			 * find the handler object if the handler->deleted == DELETE_ALL
			 * So the method_delete function will return -ENOENT
			 */
			lb_unref(handler);
			ret = 0;
			goto out;
		}
	}

	lb_set_size(handler, lb_w, lb_h);
	lb_set_lb_fb(handler, lb_fname);

	lb_set_pdsize(handler, pd_w, pd_h);
	lb_set_pd_fb(handler, pd_fname);

	lb_set_priority(handler, priority);

	lb_set_size_list(handler, size_list);
	lb_set_group(handler, cluster, category);
	lb_set_content(handler, content);
	lb_set_user(handler, user);

	lb_set_auto_launch(handler, auto_launch);
	lb_set_pinup(handler, pinup_supported);

	lb_set_period(handler, period);

	lb_invoke_event_handler(handler, "lb,created");

	ret = 0;
out:
	g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", ret));
}

static void method_deleted(GDBusMethodInvocation *inv, GVariant *param)
{
	const char *pkgname;
	const char *filename;
	struct livebox *handler;
	int ret;

	g_variant_get(param, "(&s&s)", &pkgname, &filename);

	handler = lb_find_livebox(pkgname, filename);
	if (!handler) {
		/*!
		 * \note
		 * This can be happens only if the user delete a livebox
		 * right after create it before receive created event.
		 */
		ret = -ENOENT;
		goto out;
	}

	lb_invoke_event_handler(handler, "lb,deleted");

	/* Just try to delete it, if a user didn't remove it from the live box list */
	lb_unref(handler);

	ret = 0;
out:
	g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", ret));
}

static void method_handler(GDBusConnection *conn,
		const gchar *sender,
		const gchar *object_path,
		const gchar *iface_name,
		const gchar *method,
		GVariant *param,
		GDBusMethodInvocation *invocation,
		gpointer user_data)
{
	register int i;
	struct method_table {
		const char *name;
		void (*method)(GDBusMethodInvocation *inv, GVariant *param);
	} method_table[] = {
		{
			.name = "created",
			.method = method_created,
		},
		{
			.name = "deleted",
			.method = method_deleted,
		},
		{
			.name = "lb_updated",
			.method = method_lb_updated,
		},
		{
			.name = "pd_updated",
			.method = method_pd_updated,
		},
		{
			.name = "fault_package",
			.method = method_fault_package,
		},
		{
			.name = NULL,
			.method = NULL,
		},
	};

	for (i = 0; method_table[i].name; i++) {
		if (!g_strcmp0(method, method_table[i].name)) {
			if (!method_table[i].method) {
				DbgPrint("Method %s is not available\n", method_table[i].name);
				break;
			}

			method_table[i].method(invocation, param);
			break;
		}
	}

	DbgPrint("Method[%s] is processed\n", method);
}

static const GDBusInterfaceVTable iface_vtable = {
	method_handler,
	NULL,
	NULL,
};

static inline void register_dbus_object(void)
{
	GError *err;
	GDBusConnection *conn;

	conn = g_dbus_proxy_get_connection(s_info.proxy);
	if (!conn)
		goto errout;

	err = NULL;
	s_info.node_info = g_dbus_node_info_new_for_xml(s_info.xml_data, &err);
	if (!s_info.node_info) {
		if (err) {
			ErrPrint("error - %s\n", err->message);
			g_error_free(err);
		}
		goto errout;
	}

	err = NULL;
	s_info.reg_id = g_dbus_connection_register_object(conn,
						OBJECT_PATH,
						s_info.node_info->interfaces[0],
						&iface_vtable,
						NULL, NULL,
						&err);
	if (s_info.reg_id <= 0) {
		if (err) {
			DbgPrint("Register: %s\n", err->message);
			g_error_free(err);
		}

		goto errout;
	}

	DbgPrint("Registered: %d\n", s_info.reg_id);
	return;

errout:
	g_object_unref(s_info.proxy);
	s_info.proxy = NULL;
	return;
}

static void got_proxy_cb(GObject *obj, GAsyncResult *res, gpointer user_data)
{
	GError *err;

	err = NULL;
	s_info.proxy = g_dbus_proxy_new_for_bus_finish(res, &err);
	if (!s_info.proxy) {
		if (err) {
			ErrPrint("Error: %s\n", err->message);
			g_error_free(err);
		}

		if (!s_info.recon_timer)
			s_info.recon_timer = g_timeout_add(1000, dbus_reconnect_cb, NULL);
		return;
	}

	g_signal_connect(s_info.proxy, "g-signal", G_CALLBACK(on_signal), NULL);
	register_dbus_object();
	send_acquire();

	if (s_info.cmd_list && !s_info.cmd_timer) {
		DbgPrint("10ms timer is adding\n");
		s_info.cmd_timer = g_timeout_add(10, cmd_consumer, NULL);
		if (!s_info.cmd_timer)
			ErrPrint("Failed to add timer\n");
	}
}

int dbus_sync_command(const char *funcname, GVariant *param)
{
	GVariant *result;
	GError *err;
	int ret;

	err = NULL;
	result = g_dbus_proxy_call_sync(s_info.proxy, funcname, param,
					G_DBUS_CALL_FLAGS_NO_AUTO_START, -1, NULL, &err);
	if (!result) {
		if (err) {
			ErrPrint("funcname: %s, Error: %s\n", funcname, err->message);
			g_error_free(err);
		}

		return -EIO;
	}

	g_variant_get(result, "(i)", &ret);
	g_variant_unref(result);

	return ret;
}

int dbus_push_command(struct livebox *handler, const char *funcname, GVariant *param, void (*ret_cb)(struct livebox *handler, int ret, void *data), void *data)
{
	struct cmd_item *item;

	item = malloc(sizeof(*item));
	if (!item) {
		ErrPrint("Failed to allocate mem for cmd_item\n");
		return -ENOMEM;
	}

	item->funcname = strdup(funcname);
	if (!item->funcname) {
		ErrPrint("Failed to allocate mem for funcname - %s\n", funcname);
		free(item);
		return -ENOMEM;
	}

	lb_ref(handler);

	item->param = param;
	item->handler = handler;
	item->ret_cb = ret_cb;
	item->data = data;

	s_info.cmd_list = dlist_append(s_info.cmd_list, item);

	if (!s_info.cmd_timer && s_info.proxy) {
		DbgPrint("10ms timer is adding\n");
		s_info.cmd_timer = g_timeout_add(10, cmd_consumer, NULL);
		if (!s_info.cmd_timer)
			ErrPrint("Failed to add timer\n");
	}

	return 0;
}

int dbus_init(void)
{
	g_dbus_proxy_new_for_bus(BUS_TYPE,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL,
			SERVICE_NAME,
			OBJECT_PATH,
			SERVICE_INTERFACE,
			NULL,
			got_proxy_cb, NULL);
					
	return 0;
}

int dbus_fini(void)
{
	if (s_info.proxy) {
		GDBusConnection *conn;

		send_release();

		conn = g_dbus_proxy_get_connection(s_info.proxy);
		if (conn) {
			/* FIXME: Do I really need to do this? */
			g_dbus_connection_unregister_object(conn, s_info.reg_id);
			s_info.reg_id = 0;
		}

		g_object_unref(s_info.proxy);
		s_info.proxy = NULL;
	}

	if (s_info.node_info) {
		g_dbus_node_info_unref(s_info.node_info);
		s_info.node_info = NULL;
	}

	return 0;
}

/* End of a file */
