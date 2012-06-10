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
#include "livebox_internal.h"
#include "dbus.h"
#include "desc_parser.h"
#include "master_rpc.h"

static struct info {
	GDBusNodeInfo *node_info;
	GDBusProxy *proxy;
	guint recon_timer;
	guint reg_id;
	guint reacquire_timer;
	const gchar *xml_data;
} s_info = {
	.proxy = NULL,
	.recon_timer = 0,
	.reacquire_timer = 0,
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
	"  <arg type='d' name='timestamp' direction='in' />"
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

GDBusProxy *dbus_proxy(void)
{
	return s_info.proxy;
}

static void method_fault_package(GDBusMethodInvocation *inv, GVariant *param)
{
	const char *pkgname;
	const char *filename;
	const char *function;
	char *_pkgname;
	char *_filename;
	char *_function;

	g_variant_get(param, "(&s&s&s)", &pkgname, &filename, &function);

	_pkgname = strdup(pkgname);
	_filename = strdup(filename);
	_function = strdup(function);

	DbgPrint("%s(%s) is deactivated\n", pkgname, filename);
	g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", 0));

	lb_invoke_fault_handler("deactivated", _pkgname, _filename, _function);
	free(_pkgname);
	free(_filename);
	free(_function);
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
		g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", -ENOENT));
		return;
	}

	if (handler->state == DELETE) {
		/*!
		 * \note
		 * This handler is already deleted by the user.
		 * So don't try to notice anything about this anymore.
		 * Just ignore all events.
		 */
		g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", 0));
		return;
	}

	lb_set_pdsize(handler, pd_w, pd_h);

	if (lb_text_pd(handler)) {
		ret = parse_desc(handler, filename, 1);
		g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", ret));
	} else {
		if (lb_get_pd_fb(handler)) {
			lb_update_pd_fb(handler, pd_w, pd_h);
			/*!
			 * \note
			 * After lb_update_pd_fb function,
			 * The return value of lb_get_pd_fb function can be change,
			 * So call lb_get_pd_fb again to get newly allocated pd buffer
			 */
			ret = fb_sync(lb_get_pd_fb(handler));
			if (ret < 0) {
				g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", ret));
				return;
			}
		}

		g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", 0));
		lb_invoke_event_handler(handler, "pd,updated");
	}
}

static void method_lb_updated(GDBusMethodInvocation *inv, GVariant *param)
{
	const char *pkgname;
	const char *filename;
	struct livebox *handler;
	int lb_w;
	int lb_h;
	double priority;

	g_variant_get(param, "(&s&siid)",
			&pkgname, &filename,
			&lb_w, &lb_h, &priority);

	handler = lb_find_livebox(pkgname, filename);
	if (!handler) {
		g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", -ENOENT));
		return;
	}

	if (handler->state == DELETE) {
		/*!
		 * \note
		 * Already deleted by the user.
		 * Don't try to notice anything with this, Just ignore all events
		 * Beacuse the user doesn't wants know about this anymore
		 */
		g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", 0));
		return;
	}

	lb_set_priority(handler, priority);

	if (lb_text_lb(handler)) {
		int ret;
		lb_set_size(handler, lb_w, lb_h);
		ret = parse_desc(handler, filename, 0);
		g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", ret));
	} else {
		if (lb_get_lb_fb(handler)) {
			int ret;
			lb_update_lb_fb(handler, lb_w, lb_h);
			ret = fb_sync(lb_get_lb_fb(handler));
			if (ret < 0) {
				g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", ret));
				return;
			}
		} else {
			lb_set_size(handler, lb_w, lb_h);
		}

		g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", 0));

		lb_invoke_event_handler(handler, "lb,updated");
	}
}

static void method_created(GDBusMethodInvocation *inv, GVariant *param)
{
	struct livebox *handler;

	int lb_w;
	int lb_h;
	int pd_w;
	int pd_h;
	const char *pkgname;
	const char *filename;

	const char *content;
	const char *cluster;
	const char *category;
	const char *lb_fname;
	const char *pd_fname;

	char *_content;
	char *_cluster;
	char *_category;
	char *_lb_fname;
	char *_pd_fname;

	double timestamp;
	int auto_launch;
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
		"cluster: %s, category: %s, lb_fname: \"%s\", pd_fname: \"%s\", "
		"auto_launch: %d, priority: %lf, size_list: %d, user: %d, pinup: %d, "
		"text_lb: %d, text_pd: %d, period: %lf\n",
		timestamp, pkgname, filename, content,
		pd_w, pd_h, lb_w, lb_h,
		cluster, category, lb_fname, pd_fname,
		auto_launch, priority, size_list, user, pinup_supported,
		text_lb, text_pd, period);

	handler = lb_find_livebox_by_timestamp(timestamp);
	if (!handler) {
		handler = lb_new_livebox(pkgname, filename, timestamp);
		if (!handler) {
			ErrPrint("Failed to create a new livebox\n");
			g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", -EFAULT));
			return;
		}
	} else {
		lb_set_filename(handler, filename);

		if (handler->state == DELETE) {
			lb_send_delete(handler);
			g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", 0));
			return;
		}
	}

	_lb_fname = strdup(lb_fname);
	_pd_fname = strdup(pd_fname);
	_cluster = strdup(cluster);
	_category = strdup(category);
	_content = strdup(content);
	g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", 0));

	lb_set_size(handler, lb_w, lb_h);
	lb_set_lb_fb(handler, _lb_fname);
	free(_lb_fname);

	lb_set_pdsize(handler, pd_w, pd_h);
	lb_set_pd_fb(handler, _pd_fname);
	free(_pd_fname);

	lb_set_priority(handler, priority);

	lb_set_size_list(handler, size_list);
	lb_set_group(handler, _cluster, _category);
	free(_cluster);
	free(_category);

	lb_set_content(handler, _content);
	free(_content);

	lb_set_user(handler, user);

	lb_set_auto_launch(handler, auto_launch);
	lb_set_pinup(handler, pinup_supported);

	lb_set_period(handler, period);

	lb_invoke_event_handler(handler, "lb,created");
}

static void method_deleted(GDBusMethodInvocation *inv, GVariant *param)
{
	const char *pkgname;
	const char *filename;
	double timestamp;
	struct livebox *handler;

	g_variant_get(param, "(&s&sd)", &pkgname, &filename, &timestamp);

	handler = lb_find_livebox_by_timestamp(timestamp);
	if (!handler) {
		/*!
		 * \note
		 * This can be happens only if the user delete a livebox
		 * right after create it before receive created event.
		 */
		g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", -ENOENT));
		return;
	}

	DbgPrint("[%p] %s(%s) is deleted\n", handler, pkgname, filename);
	g_dbus_method_invocation_return_value(inv, g_variant_new("(i)", 0));

	lb_invoke_event_handler(handler, "lb,deleted");
	/* Just try to delete it, if a user didn't remove it from the live box list */
	lb_unref(handler);
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
				ErrPrint("Method %s is not available\n", method_table[i].name);
				break;
			}

			method_table[i].method(invocation, param);
			break;
		}
	}
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
			ErrPrint("Register failed: %s\n", err->message);
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

static inline void unregister_dbus_object(void)
{
	GDBusConnection *conn;

	conn = g_dbus_proxy_get_connection(s_info.proxy);
	if (!conn)
		return;

	/* FIXME: Do I really need to do this? */
	g_dbus_connection_unregister_object(conn, s_info.reg_id);
	s_info.reg_id = 0;

	if (s_info.node_info) {
		g_dbus_node_info_unref(s_info.node_info);
		s_info.node_info = NULL;
	}

}

static void on_signal(GDBusProxy *proxy, gchar *sender, gchar *signame, GVariant *param, gpointer data)
{
	DbgPrint("Sender: %s\n", sender);
	DbgPrint("SigName: %s\n", signame);
}

static gboolean dbus_reconnect_cb(gpointer user_data)
{
	dbus_init();
	s_info.recon_timer = 0;
	return FALSE;
}

static void got_proxy_cb(GObject *obj, GAsyncResult *res, gpointer user_data)
{
	GVariant *param;
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

	param = g_variant_new("(i)", getpid());
	if (!param) {
		unregister_dbus_object();
		g_object_unref(s_info.proxy);
		s_info.proxy = NULL;
		return;
	}

	master_rpc_sync_request(NULL, "acquire", param);
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
	GVariant *param;

	if (!s_info.proxy)
		return 0;

	param = g_variant_new("(i)", getpid());
	if (param)
		master_rpc_sync_request(NULL, "release", param);

	unregister_dbus_object();
	g_object_unref(s_info.proxy);
	s_info.proxy = NULL;

	return 0;
}

/* End of a file */
