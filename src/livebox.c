#include <stdio.h>
#include <errno.h>
#include <stdlib.h> /* malloc */
#include <string.h> /* strdup */

#include <aul.h>
#include <dlog.h>

#include <com-core_packet.h>
#include <packet.h>

#include "debug.h"
#include "fb.h"
#include "livebox.h"
#include "livebox_internal.h"
#include "dlist.h"
#include "util.h"
#include "master_rpc.h"
#include "client.h"

#define EAPI __attribute__((visibility("default")))
#define EVENT_INTERVAL	0.05f

#if defined(FLOG)
FILE *__file_log_fp;
#endif

static struct info {
	struct dlist *livebox_list;
	struct dlist *event_list;
	struct dlist *fault_list;
} s_info = {
	.livebox_list = NULL,
	.event_list = NULL,
	.fault_list = NULL,
};

struct event_info {
	int (*handler)(struct livebox *handler, const char *event, void *data);
	void *user_data;
};

struct fault_info {
	int (*handler)(const char *event, const char *pkgname, const char *filename, const char *func, void *data);
	void *user_data;
};

/*!
 * \note
 * If the event occurs too fast to handle them correctly,
 * just ignore them.
 */
static int update_event_timestamp(struct livebox *handler)
{
	double now;
	double interval;

	now = util_timestamp();
	interval = now - handler->event_timestamp;

	if (interval < EVENT_INTERVAL)
		return -EBUSY;

	handler->event_timestamp = now;
	return 0;
}

static void mouse_event_cb(struct livebox *handler, const struct packet *packet, void *data)
{
	int ret;

	if (!packet) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	if (packet_get(packet, "i", &ret) != 1) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	if (ret < 0)
		lb_invoke_event_handler(handler, "event,ingored");
}

static void resize_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;

	if (!result) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	if (packet_get(result, "i", &ret) != 1) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	if (ret < 0)
		lb_invoke_event_handler(handler, "event,ignored");
}

static void clicked_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;

	if (!result) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	if (packet_get(result, "i", &ret) != 1) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	if (ret < 0)
		lb_invoke_event_handler(handler, "event,ignored");
		
}

static void text_signal_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;

	if (!result) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	if (packet_get(result, "i", &ret) != 1) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	if (ret < 0)
		lb_invoke_event_handler(handler, "event,ignored");

	return;
}

static void set_group_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;

	if (!result) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	if (packet_get(result, "i", &ret) != 1) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	if (ret < 0)
		lb_invoke_event_handler(handler, "event,ignored");

	return;
}

static void period_ret_cb(struct livebox *handler, const struct packet *result, void *data)
{
	double *period;
	int ret;

	if (!result) {
		lb_invoke_event_handler(handler, "event,ignored");
		free(data);
		return;
	}

	if (packet_get(result, "i", &ret) != 1) {
		lb_invoke_event_handler(handler, "event,ignored");
		free(data);
		return;
	}

	period = (double *)data;

	if (ret < 0)
		lb_invoke_event_handler(handler, "event,ignored");
	else if (ret == 0)
		lb_set_period(handler, *period);
	else
		ErrPrint("Unknown returns %d\n", ret);

	free(data);
}

static void del_ret_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;

	if (!result) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	if (packet_get(result, "i", &ret) != 1) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	if (ret < 0) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}
}

static void new_ret_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;

	if (!result) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	if (packet_get(result, "i", &ret) != 1) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	if (ret >= 0)
		return;

	/*!
	 * \note
	 * It means the current instance is not created,
	 * so user has to know about this.
	 * notice it to user using "deleted" event.
	 */
	lb_invoke_event_handler(handler, "lb,deleted");
	lb_unref(handler);
}

static void pd_created_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;

	if (!result) {
		lb_invoke_event_handler(handler, "pd,create,failed");
		return;
	}

	if (packet_get(result, "i", &ret) != 1) {
		lb_invoke_event_handler(handler, "pd,create,failed");
		return;
	}

	if (ret < 0) {
		DbgPrint("Livebox returns %d\n", ret);
		lb_invoke_event_handler(handler, "pd,create,failed");
		return;
	}

	if (!handler->pd.data.fb) {
		DbgPrint("Failed to create a PD (FB is not valid)\n");
		lb_invoke_event_handler(handler, "pd,create,failed");
		return;
	}

	ret = fb_create_buffer(handler->pd.data.fb);
	if (ret < 0)
		lb_invoke_event_handler(handler, "pd,create,failed");
	else
		lb_invoke_event_handler(handler, "pd,created");
}

static void activated_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;
	char *pkgname = data;

	if (!result) {
		lb_invoke_fault_handler("activation,failed", pkgname, NULL, NULL);
		free(pkgname);
		return;
	}

	if (packet_get(result, "i", &ret) != 1) {
		lb_invoke_fault_handler("activation,failed", pkgname, NULL, NULL);
		free(pkgname);
		return;
	}

	if (ret == 0)
		lb_invoke_fault_handler("activated", pkgname, NULL, NULL);
	else if (ret == -EINVAL)
		lb_invoke_fault_handler("invalid,request", pkgname, NULL, NULL);
	else
		lb_invoke_fault_handler("activation,failed", pkgname, NULL, NULL);

	free(pkgname);
}

static void pd_destroy_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;

	fb_destroy_buffer(handler->pd.data.fb);

	if (!result) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	if (packet_get(result, "i", &ret) != 1) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	if (ret < 0) {
		DbgPrint("PD destroy returns %d\n", ret);
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	lb_invoke_event_handler(handler, "pd,deleted");
}

static void pinup_done_cb(struct livebox *handler, const struct packet *result, void *data)
{
	int ret;

	if (!result) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	if (packet_get(result, "i", &ret) != 1) {
		lb_invoke_event_handler(handler, "event,ignored");
		return;
	}

	if (ret != 0) {
		ErrPrint("Pinup is not changed: %s\n", strerror(ret));
		lb_invoke_event_handler(handler, "pinup,failed");
	} else {
		handler->lb.is_pinned_up = (int)data;
		lb_invoke_event_handler(handler, "pinup,changed");
	}
}

static int send_mouse_event(struct livebox *handler, const char *event, double x, double y, int w, int h)
{
	struct packet *packet;
	double timestamp;

	timestamp = util_timestamp();
	packet = packet_create(event, "ssiiddd", handler->pkgname, handler->filename, w, h,
						timestamp, x, y);
	if (!packet) {
		ErrPrint("Failed to build param\n");
		return -EFAULT;
	}

	return master_rpc_async_request(handler, packet, 0, mouse_event_cb, NULL);
}

EAPI int livebox_init(void)
{
#if defined(FLOG)
	char filename[BUFSIZ];
	snprintf(filename, sizeof(filename), "/tmp/%d.box.log", getpid());
	__file_log_fp = fopen(filename, "w+t");
	if (!__file_log_fp)
		__file_log_fp = fdopen(1, "w+t");
#endif

	client_init();
	return 0;
}

EAPI int livebox_fini(void)
{
	client_fini();
	return 0;
}

EAPI struct livebox *livebox_add(const char *pkgname, const char *content, const char *cluster, const char *category, double period)
{
	struct livebox *handler;
	struct packet *packet;
	int ret;

	if (!pkgname || !cluster || !category)
		return NULL;

	handler = calloc(1, sizeof(*handler));
	if (!handler) {
		ErrPrint("Error: %s\n", strerror(errno));
		return NULL;
	}

	handler->pkgname = strdup(pkgname);
	if (!handler->pkgname) {
		ErrPrint("Error: %s\n", strerror(errno));
		free(handler);
		return NULL;
	}

	if (content) {
		handler->content = strdup(content);
		if (!handler->content) {
			ErrPrint("Error: %s\n", strerror(errno));
			free(handler->pkgname);
			free(handler);
			return NULL;
		}
	}

	handler->cluster = strdup(cluster);
	if (!handler->cluster) {
		ErrPrint("Error: %s\n", strerror(errno));
		free(handler->content);
		free(handler->pkgname);
		free(handler);
		return NULL;
	}

	handler->category = strdup(category);
	if (!handler->category) {
		ErrPrint("Error: %s\n", strerror(errno));
		free(handler->cluster);
		free(handler->content);
		free(handler->pkgname);
		free(handler);
		return NULL;
	}

	/* Data provider will set this */
	handler->lb.type = LB_FILE;
	handler->pd.type = PD_FB;
	handler->lb.period = period;

	/* Cluster infomration is not determined yet */
	handler->nr_of_sizes = 0x01;

	handler->timestamp = util_timestamp();
	handler->is_user = 1;

	s_info.livebox_list = dlist_append(s_info.livebox_list, handler);

	packet = packet_create("new", "dssssd", handler->timestamp, pkgname, content, cluster, category, period);
	if (!packet) {
		free(handler->category);
		free(handler->cluster);
		free(handler->content);
		free(handler->pkgname);
		free(handler);
		return NULL;
	}

	ret = master_rpc_async_request(handler, packet, 0, new_ret_cb, NULL);
	if (ret < 0) {
		free(handler->category);
		free(handler->cluster);
		free(handler->content);
		free(handler->pkgname);
		free(handler);
		return NULL;
	}

	handler->state = CREATE;
	return lb_ref(handler);
}

EAPI double livebox_period(struct livebox *handler)
{
	if (!handler || handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return 0.0f;
	}

	return handler->lb.period;
}

EAPI int livebox_set_period(struct livebox *handler, double period)
{
	struct packet *packet;
	double *period_heap;

	if (!handler || !handler->filename || handler->state == DELETE) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	if (handler->lb.period == period)
		return 0;

	period_heap = malloc(sizeof(*period_heap));
	if (!period_heap)
		return -ENOMEM;

	packet = packet_create("set_period", "ssd", handler->pkgname, handler->filename, period);
	if (!packet) {
		free(period_heap);
		return -EFAULT;
	}

	*period_heap = period;
	return master_rpc_async_request(handler, packet, 0, period_ret_cb, (void *)period_heap);
}

EAPI int livebox_del(struct livebox *handler, int unused)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (handler->state == DELETE) {
		ErrPrint("Handler is already deleted\n");
		return -EINVAL;
	}

	handler->state = DELETE;

	if (!handler->filename) {
		/*!
		 * \note
		 * The filename is not determined yet.
		 * It means a user didn't receive created event yet.
		 * Then just stop to delete procedure from here.
		 * Because the "created" event handler will release this.
		 */
		return 0;
	}

	return lb_send_delete(handler);
}

EAPI int livebox_fault_handler_set(int (*cb)(const char *, const char *, const char *, const char *, void *), void *data)
{
	struct fault_info *info;

	if (!cb)
		return -EINVAL;

	info = malloc(sizeof(*info));
	if (!info) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return -ENOMEM;
	}

	info->handler = cb;
	info->user_data = data;

	s_info.fault_list = dlist_append(s_info.fault_list, info);
	return 0;
}

EAPI void *livebox_fault_handler_unset(int (*cb)(const char *, const char *, const char *, const char *, void *))
{
	struct fault_info *info;
	struct dlist *l;

	dlist_foreach(s_info.fault_list, l, info) {
		if (info->handler == cb) {
			void *data;
			s_info.fault_list = dlist_remove(s_info.fault_list, l);
			data = info->user_data;
			free(info);

			return data;
		}
	}

	return NULL;
}

EAPI int livebox_event_handler_set(int (*cb)(struct livebox *, const char *, void *), void *data)
{
	struct event_info *info;

	if (!cb) {
		ErrPrint("Invalid argument cb is nil\n");
		return -EINVAL;
	}

	info = malloc(sizeof(*info));
	if (!info) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return -ENOMEM;
	}

	info->handler = cb;
	info->user_data = data;

	s_info.event_list = dlist_append(s_info.event_list, info);
	return 0;
}

EAPI void *livebox_event_handler_unset(int (*cb)(struct livebox *, const char *, void *))
{
	struct event_info *info;
	struct dlist *l;

	dlist_foreach(s_info.event_list, l, info) {
		if (info->handler == cb) {
			void *data;

			s_info.event_list = dlist_remove(s_info.event_list, l);
			data = info->user_data;
			free(info);

			return data;
		}
	}

	return NULL;
}

EAPI int livebox_resize(struct livebox *handler, int w, int h)
{
	struct packet *packet;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	packet = packet_create("resize", "ssii", handler->pkgname, handler->filename, w, h);
	if (!packet) {
		ErrPrint("Failed to build param\n");
		return -EFAULT;
	}

	return master_rpc_async_request(handler, packet, 0, resize_cb, NULL);
}

EAPI int livebox_click(struct livebox *handler, double x, double y)
{
	struct packet *packet;
	double timestamp;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	if (handler->lb.auto_launch)
		if (aul_launch_app(handler->pkgname, NULL) < 0)
			ErrPrint("Failed to launch app %s\n", handler->pkgname);

	timestamp = util_timestamp();
	packet = packet_create("clicked", "sssddd", handler->pkgname, handler->filename, "clicked", timestamp, x, y);
	if (!packet) {
		ErrPrint("Failed to build param\n");
		return -EFAULT;
	}

	return master_rpc_async_request(handler, packet, 0, clicked_cb, NULL);
}

EAPI int livebox_has_pd(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	return !!handler->pd.data.fb;
}

EAPI int livebox_pd_is_created(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (!handler->pd.data.fb || handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	return fb_is_created(handler->pd.data.fb);
}

EAPI int livebox_create_pd(struct livebox *handler)
{
	struct packet *packet;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (!handler->pd.data.fb || handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	if (fb_is_created(handler->pd.data.fb) == 1) {
		DbgPrint("PD already created\n");
		return 0;
	}

	packet = packet_create("create_pd", "ss", handler->pkgname, handler->filename);
	if (!packet) {
		ErrPrint("Failed to build param\n");
		return -EFAULT;
	}

	return master_rpc_async_request(handler, packet, 0, pd_created_cb, NULL);
}

EAPI int livebox_activate(const char *pkgname)
{
	struct packet *packet;
	char *str;

	if (!pkgname)
		return -EINVAL;

	packet = packet_create("activate_package", "s", pkgname);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return -EFAULT;
	}

	str = strdup(pkgname);
	if (!str) {
		ErrPrint("Heap: %s\n", strerror(errno));
		packet_destroy(packet);
		return -ENOMEM;
	}

	return master_rpc_async_request(NULL, packet, 0, activated_cb, str);
}

EAPI int livebox_destroy_pd(struct livebox *handler)
{
	struct packet *packet;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (!handler->pd.data.fb || handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	if (fb_is_created(handler->pd.data.fb) != 1) {
		ErrPrint("PD is not created\n");
		return -EINVAL;
	}

	packet = packet_create("destroy_pd", "ss", handler->pkgname, handler->filename);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return -EFAULT;
	}

	return master_rpc_async_request(handler, packet, 0, pd_destroy_cb, NULL);
}

EAPI int livebox_pd_mouse_down(struct livebox *handler, double x, double y)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (!handler->pd.data.fb || handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	return send_mouse_event(handler, "pd_mouse_down", x, y, handler->pd.width, handler->pd.height);
}

EAPI int livebox_pd_mouse_up(struct livebox *handler, double x, double y)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (!handler->pd.data.fb || handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	return send_mouse_event(handler, "pd_mouse_up", x, y, handler->pd.width, handler->pd.height);
}

EAPI int livebox_pd_mouse_move(struct livebox *handler, double x, double y)
{
	int ret;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (!handler->pd.data.fb || handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	ret = update_event_timestamp(handler);
	if (ret < 0)
		return ret;

	return send_mouse_event(handler, "pd_mouse_move", x, y, handler->pd.width, handler->pd.height);
}

EAPI int livebox_livebox_mouse_down(struct livebox *handler, double x, double y)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (!handler->lb.data.fb || handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	return send_mouse_event(handler, "lb_mouse_down", x, y, handler->lb.width, handler->lb.height);
}

EAPI int livebox_livebox_mouse_up(struct livebox *handler, double x, double y)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (!handler->lb.data.fb || handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	return send_mouse_event(handler, "lb_mouse_up", x, y, handler->lb.width, handler->lb.height);
}

EAPI int livebox_livebox_mouse_move(struct livebox *handler, double x, double y)
{
	int ret;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (!handler->lb.data.fb || handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	ret = update_event_timestamp(handler);
	if (ret < 0)
		return ret;

	return send_mouse_event(handler, "lb_mouse_move", x, y, handler->lb.width, handler->lb.height);
}

EAPI const char *livebox_filename(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return NULL;
	}

	if (handler->state == DELETE) {
		ErrPrint("Handler is not valid\n");
		return NULL;
	}

	return handler->filename;
}

EAPI int livebox_get_pdsize(struct livebox *handler, int *w, int *h)
{
	int _w;
	int _h;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	if (!w)
		w = &_w;
	if (!h)
		h = &_h;

	*w = handler->pd.width;
	*h = handler->pd.height;
	return 0;
}

EAPI int livebox_get_size(struct livebox *handler, int *w, int *h)
{
	int _w;
	int _h;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	if (!w)
		w = &_w;
	if (!h)
		h = &_h;

	*w = handler->lb.width;
	*h = handler->lb.height;
	return 0;
}

EAPI int livebox_set_group(struct livebox *handler, const char *cluster, const char *category)
{
	struct packet *packet;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (!cluster || !category || handler->state == DELETE || !handler->filename) {
		ErrPrint("Invalid argument\n");
		return -EINVAL;
	}

	packet = packet_create("change_group", "ssss", handler->pkgname, handler->filename, cluster, category);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return -EFAULT;
	}

	return master_rpc_async_request(handler, packet, 0, set_group_cb, NULL);
}

EAPI int livebox_get_group(struct livebox *handler, char ** const cluster, char ** const category)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (!cluster || !category || handler->state == DELETE || !handler->filename) {
		ErrPrint("Invalid argument\n");
		return -EINVAL;
	}

	*cluster = handler->cluster;
	*category = handler->category;
	return 0;
}

EAPI int livebox_get_supported_sizes(struct livebox *handler, int *cnt, int *w, int *h)
{
	register int i;
	register int j;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (!cnt || handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	for (j = i = 0; i < NR_OF_SIZE_LIST; i++) {
		if (handler->lb.size_list & (0x01 << i)) {
			if (j == *cnt)
				break;

			if (w)
				w[j] = SIZE_LIST[i].w;
			if (h)
				h[j] = SIZE_LIST[i].h;
			j++;
		}
	}

	*cnt = j;
	return 0;
}

EAPI const char *livebox_pkgname(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return NULL;
	}

	if (handler->state == DELETE) {
		ErrPrint("Handler is not valid\n");
		return NULL;
	}

	return handler->pkgname;
}

EAPI double livebox_priority(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return 0.0f;
	}

	if (handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid (%p)\n", handler);
		return -1.0f;
	}

	return handler->lb.priority;
}

EAPI int livebox_delete_cluster(const char *cluster)
{
	ErrPrint("Not implemented yet\n");
	return -ENOSYS;
}

EAPI int livebox_delete_category(const char *cluster, const char *category)
{
	ErrPrint("Not implemented yet\n");
	return -ENOSYS;
}

EAPI int livebox_is_file(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	return handler->lb.type == LB_FILE;
}

EAPI int livebox_is_text(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	return handler->lb.type == LB_TEXT;
}

EAPI int livebox_pd_is_text(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	return handler->pd.type == PD_TEXT;
}

EAPI int livebox_pd_set_text_handler(struct livebox *handler, struct livebox_script_operators *ops)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (handler->state == DELETE) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	memcpy(&handler->pd.data.ops, ops, sizeof(*ops));
	return 0;
}

EAPI int livebox_set_text_handler(struct livebox *handler, struct livebox_script_operators *ops)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (handler->state == DELETE) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	memcpy(&handler->lb.data.ops, ops, sizeof(*ops));
	return 0;
}

EAPI void *livebox_acquire_fb(struct livebox *handler)
{
	void *ptr;
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return NULL;
	}

	if (handler->state == DELETE || !handler->filename || handler->lb.type != LB_FB) {
		ErrPrint("Handler is not valid\n");
		return NULL;
	}

	ptr = fb_acquire_buffer(handler->lb.data.fb);
	return ptr;
}

EAPI int livebox_release_fb(void *buffer)
{
	return fb_release_buffer(buffer);
}

EAPI int livebox_fb_refcnt(void *buffer)
{
	return fb_refcnt(buffer);
}

EAPI void *livebox_acquire_pdfb(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return NULL;
	}

	if (handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return NULL;
	}

	return fb_acquire_buffer(handler->pd.data.fb);
}

EAPI int livebox_release_pdfb(void *buffer)
{
	return fb_release_buffer(buffer);
}

EAPI int livebox_pdfb_refcnt(void *buffer)
{
	return fb_refcnt(buffer);
}

EAPI int livebox_pdfb_bufsz(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	return fb_size(handler->pd.data.fb);
}

EAPI int livebox_lbfb_bufsz(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	return fb_size(handler->lb.data.fb);
}

EAPI int livebox_is_user(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (handler->state == DELETE) {
		ErrPrint("Handler is invalid\n");
		return -EINVAL;
	}

	return handler->is_user;
}

EAPI int livebox_set_pinup(struct livebox *handler, int flag)
{
	struct packet *packet;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	if (handler->lb.is_pinned_up == flag)
		return 0;

	packet = packet_create("pinup_changed", "ssi", handler->pkgname, handler->filename, flag);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return -EFAULT;
	}

	return master_rpc_async_request(handler, packet, 0, pinup_done_cb, (void *)flag);
}

EAPI int livebox_pinup(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (handler->state == DELETE || !handler->filename)
		return -EINVAL;

	return handler->lb.is_pinned_up;
}

EAPI int livebox_has_pinup(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (handler->state == DELETE || !handler->filename)
		return -EINVAL;

	return handler->lb.pinup_supported;
}

EAPI int livebox_set_data(struct livebox *handler, void *data)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if (handler->state == DELETE)
		return -EINVAL;

	handler->data = data;
	return 0;
}

EAPI void *livebox_get_data(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return NULL;
	}

	if (handler->state == DELETE)
		return NULL;

	return handler->data;
}

EAPI int livebox_is_exists(const char *pkgname)
{
	struct packet *packet;
	int ret;

	packet = packet_create("livebox_is_exists", "s", pkgname);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return 0;
	}
	ret = master_rpc_sync_request(packet);
	packet_destroy(packet);
	return ret == 0;
}

EAPI const char *livebox_content(struct livebox *handler)
{
	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return NULL;
	}

	if (handler->state == DELETE)
		return NULL;

	return handler->content;
}

EAPI int livebox_text_emit_signal(struct livebox *handler, const char *emission, const char *source, double sx, double sy, double ex, double ey)
{
	struct packet *packet;

	if (!handler) {
		ErrPrint("Handler is NIL\n");
		return -EINVAL;
	}

	if ((handler->lb.type != LB_TEXT && !handler->pd.type != PD_TEXT) || handler->state == DELETE || !handler->filename) {
		ErrPrint("Handler is not valid\n");
		return -EINVAL;
	}

	if (!emission)
		emission = "";

	if (!source)
		source = "";

	packet = packet_create("text_signal", "ssssdddd",
				handler->pkgname, handler->filename, emission, source, sx, sy, ex, ey);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return -EFAULT;
	}

	return master_rpc_async_request(handler, packet, 0, text_signal_cb, NULL);
}

int lb_set_group(struct livebox *handler, const char *cluster, const char *category)
{
	void *pc = NULL;
	void *ps = NULL;

	if (cluster) {
		pc = strdup(cluster);
		if (!pc) {
			ErrPrint("Heap: %s\n", strerror(errno));
			return -ENOMEM;
		}
	}

	if (category) {
		ps = strdup(category);
		if (!ps) {
			ErrPrint("Heap: %s\n", strerror(errno));
			free(pc);
			return -ENOMEM;
		}
	}

	if (handler->cluster)
		free(handler->cluster);

	if (handler->category)
		free(handler->category);

	handler->cluster = pc;
	handler->category = ps;

	return 0;
}

void lb_set_size(struct livebox *handler, int w, int h)
{
	handler->lb.width = w;
	handler->lb.height = h;
}

void lb_set_pdsize(struct livebox *handler, int w, int h)
{
	handler->pd.width = w;
	handler->pd.height = h;
}

void lb_invoke_fault_handler(const char *event, const char *pkgname, const char *file, const char *func)
{
	struct dlist *l;
	struct dlist *n;
	struct fault_info *info;

	dlist_foreach_safe(s_info.fault_list, l, n, info) {
		if (info->handler(event, pkgname, file, func, info->user_data) == EXIT_FAILURE)
			s_info.fault_list = dlist_remove(s_info.fault_list, l);
	}
}

void lb_invoke_event_handler(struct livebox *handler, const char *event)
{
	struct dlist *l;
	struct dlist *n;
	struct event_info *info;

	{
		DbgPrint("Inovke %s for %s\n", event, handler->pkgname);
		if (handler->lb.type == LB_FB)
			DbgPrint("LB[FB] = %s\n", util_basename(fb_filename(handler->lb.data.fb)));
		else if (handler->lb.type == LB_TEXT)
			DbgPrint("LB[TEXT] = %s\n", util_basename(handler->filename));
		else if (handler->lb.type == LB_FILE)
			DbgPrint("LB[FILE] = %s\n", util_basename(handler->filename));

		if (handler->pd.type == PD_FB)
			DbgPrint("PD[FB] = %s\n", util_basename(fb_filename(handler->pd.data.fb)));
		else if (handler->pd.type == PD_TEXT)
			DbgPrint("PD[TEXT] = %s\n", util_basename(handler->filename));
	}

	dlist_foreach_safe(s_info.event_list, l, n, info) {
		if (info->handler(handler, event, info->user_data) == EXIT_FAILURE)
			s_info.event_list = dlist_remove(s_info.event_list, l);
	}
}

struct livebox *lb_find_livebox(const char *pkgname, const char *filename)
{
	struct dlist *l;
	struct livebox *handler;

	dlist_foreach(s_info.livebox_list, l, handler) {
		if (!handler->filename)
			continue;

		if (!strcmp(handler->pkgname, pkgname) && !strcmp(handler->filename, filename))
			return handler;
	}

	return NULL;
}

struct livebox *lb_find_livebox_by_timestamp(double timestamp)
{
	struct dlist *l;
	struct livebox *handler;

	dlist_foreach(s_info.livebox_list, l, handler) {
		if (handler->timestamp == timestamp)
			return handler;
	}

	return NULL;
}

struct livebox *lb_new_livebox(const char *pkgname, const char *filename, double timestamp)
{
	struct livebox *handler;

	handler = calloc(1, sizeof(*handler));
	if (!handler) {
		ErrPrint("Failed to create a new livebox\n");
		return NULL;
	}

	handler->pkgname = strdup(pkgname);
	if (!handler->pkgname) {
		ErrPrint("%s\n", strerror(errno));
		free(handler);
		return NULL;
	}

	handler->filename = strdup(filename);
	if (!handler->filename) {
		ErrPrint("%s\n", strerror(errno));
		free(handler->pkgname);
		free(handler);
		return NULL;
	}

	handler->timestamp = timestamp;
	handler->lb.type = LB_FILE;
	handler->pd.type = PD_FB;

	s_info.livebox_list = dlist_append(s_info.livebox_list, handler);
	lb_ref(handler);
	return handler;
}

int lb_set_content(struct livebox *handler, const char *content)
{
	if (handler->content) {
		free(handler->content);
		handler->content = NULL;
	}

	if (content) {
		handler->content = strdup(content);
		if (!handler->content) {
			ErrPrint("Heap: %s\n", strerror(errno));
			return -ENOMEM;
		}
	}

	return 0;
}

void lb_set_size_list(struct livebox *handler, int size_list)
{
	handler->lb.size_list = size_list;
}

void lb_set_auto_launch(struct livebox *handler, int auto_launch)
{
	handler->lb.auto_launch = auto_launch;
}

void lb_set_priority(struct livebox *handler, double priority)
{
	handler->lb.priority = priority;
}

void lb_set_filename(struct livebox *handler, const char *filename)
{
	if (handler->filename)
		free(handler->filename);

	handler->filename = strdup(filename);
	if (!handler->filename)
		ErrPrint("Error: %s\n", strerror(errno));
}

int lb_set_lb_fb(struct livebox *handler, const char *filename)
{
	struct fb_info *fb;

	if (!handler)
		return -EINVAL;

	fb = handler->lb.data.fb;
	if (fb && !strcmp(fb_filename(fb), filename)) {
		/* BUFFER is not changed, just update the content */
		DbgPrint("Update LB. buf-file same: %s\n", filename);
		return 0;
	}

	handler->lb.data.fb = NULL;

	DbgPrint("Update to new FBFILE: %s\n", filename);

	if (!filename || filename[0] == '\0') {
		if (fb)
			fb_destroy(fb);
		return 0;
	}

	handler->lb.data.fb = fb_create(filename, handler->lb.width, handler->lb.height);
	if (!handler->lb.data.fb) {
		ErrPrint("Faield to create a FB\n");
		if (fb)
			fb_destroy(fb);
		return -EFAULT;
	}

	if (fb_create_buffer(handler->lb.data.fb) < 0) {
		fb_destroy(handler->lb.data.fb);
		handler->lb.data.fb = NULL;
		ErrPrint("Failed to create frame buffer\n");

		if (fb)
			fb_destroy(fb);
		return -EFAULT;
	}

	handler->lb.type = LB_FB;
	if (fb)
		fb_destroy(fb);
	return 0;
}

int lb_set_pd_fb(struct livebox *handler, const char *filename)
{
	struct fb_info *fb;

	if (!handler)
		return -EINVAL;

	fb = handler->pd.data.fb;
	if (fb && !strcmp(fb_filename(fb), filename)) {
		DbgPrint("Update PD. buf-file same: %s\n", filename);
		/* BUFFER is not changed, just update the content */
		return -EEXIST;
	}
	handler->pd.data.fb = NULL;

	DbgPrint("Update to new FBFILE: %s\n", filename);

	if (!filename || filename[0] == '\0') {
		if (fb)
			fb_destroy(fb);
		return 0;
	}

	handler->pd.data.fb = fb_create(filename, handler->pd.width, handler->pd.height);
	if (!handler->pd.data.fb) {
		ErrPrint("Failed to create a FB\n");
		if (fb)
			fb_destroy(fb);
		return -EFAULT;
	}

	handler->pd.type = PD_FB;
	if (fb)
		fb_destroy(fb);
	return 0;
}

struct fb_info *lb_get_lb_fb(struct livebox *handler)
{
	return handler->lb.data.fb;
}

struct fb_info *lb_get_pd_fb(struct livebox *handler)
{
	return handler->pd.data.fb;
}

void lb_set_user(struct livebox *handler, int user)
{
	handler->is_user = user;
}

void lb_set_pinup(struct livebox *handler, int pinup_supported)
{
	handler->lb.pinup_supported = pinup_supported;
}

void lb_set_text_lb(struct livebox *handler)
{
	handler->lb.type = LB_TEXT;
}

void lb_set_text_pd(struct livebox *handler)
{
	handler->pd.type = PD_TEXT;
}

int lb_text_lb(struct livebox *handler)
{
	return handler->lb.type == LB_TEXT;
}

int lb_text_pd(struct livebox *handler)
{
	return handler->pd.type == PD_TEXT;
}

void lb_set_period(struct livebox *handler, double period)
{
	handler->lb.period = period;
}

struct livebox *lb_ref(struct livebox *handler)
{
	if (!handler)
		return NULL;

	handler->refcnt++;
	return handler;
}

struct livebox *lb_unref(struct livebox *handler)
{
	if (!handler)
		return NULL;

	handler->refcnt--;
	if (handler->refcnt > 0)
		return handler;

	dlist_remove_data(s_info.livebox_list, handler);

	free(handler->cluster);
	free(handler->category);
	free(handler->filename);
	free(handler->pkgname);

	if (handler->lb.data.fb) {
		fb_destroy(handler->lb.data.fb);
		handler->lb.data.fb = NULL;
	}

	if (handler->pd.data.fb) {
		fb_destroy(handler->pd.data.fb);
		handler->pd.data.fb = NULL;
	}

	free(handler);
	return NULL;
}

int lb_send_delete(struct livebox *handler)
{
	struct packet *packet;

	packet = packet_create("delete", "ss", handler->pkgname, handler->filename);
	if (!packet) {
		ErrPrint("Failed to build a param\n");
		return -EFAULT;
	}

	return master_rpc_async_request(handler, packet, 0, del_ret_cb, NULL);
}

/* End of a file */
