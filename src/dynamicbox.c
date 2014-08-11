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

#include "dynamicbox.h"
#include "livebox.h"


#define EAPI __attribute__((visibility("default")))

EAPI int dynamicbox_init_with_options(void *disp, int prevent_overwrite, double event_filter, int use_thread)
{
	return livebox_init_with_options(disp, prevent_overwrite, event_filter, use_thread);
}

EAPI int dynamicbox_init(void *disp)
{
	return livebox_init(disp);
}

EAPI int dynamicbox_fini(void)
{
	return livebox_fini();
}

EAPI struct dynamicbox *dynamicbox_add(const char *pkgname, const char *content, const char *cluster, const char *category, double period, int type, dynamicbox_ret_cb_t cb, void *data)
{
	return (struct dynamicbox*)livebox_add(pkgname, content, cluster, category, period, type, (ret_cb_t)cb, data);
}

EAPI double dynamicbox_period(struct dynamicbox *handler)
{
	return livebox_period((struct livebox*)handler);
}

EAPI int dynamicbox_set_period(struct dynamicbox *handler, double period, dynamicbox_ret_cb_t cb, void *data)
{
	return livebox_set_period((struct livebox*)handler, period, (ret_cb_t)cb, data);
}

EAPI int dynamicbox_del(struct dynamicbox *handler, int type, dynamicbox_ret_cb_t cb, void *data)
{
	return livebox_del(((struct livebox*) handler), type, (ret_cb_t)cb, data);
}



EAPI int dynamicbox_set_fault_handler(int (*dbox_cb)(enum dynamicbox_fault_type, const char *, const char *, const char *, void *), void *data)
{
	typedef int (*set_fault_handler_cb)(enum livebox_fault_type, const char *, const char *, const char *, void *);

	return livebox_set_fault_handler((set_fault_handler_cb)dbox_cb, data);
}

EAPI void *dynamicbox_unset_fault_handler(int (*dbox_cb)(enum dynamicbox_fault_type, const char *, const char *, const char *, void *))
{
	typedef int (*cb)(enum livebox_fault_type, const char *, const char *, const char *, void *);

	return livebox_unset_fault_handler((cb)dbox_cb);
}

EAPI int dynamicbox_set_event_handler(int (*dbox_cb)(struct dynamicbox *, enum dynamicbox_event_type, void *), void *data)
{
	typedef int (*cb)(struct livebox *, enum livebox_event_type, void *);

	return livebox_set_event_handler((cb)dbox_cb, data);
}

EAPI void *dynamicbox_unset_event_handler(int (*dbox_cb)(struct dynamicbox *, enum dynamicbox_event_type, void *))
{
	typedef int (*cb)(struct livebox *, enum livebox_event_type, void *);

	return livebox_unset_event_handler((cb)dbox_cb);
}

EAPI int dynamicbox_set_update_mode(struct dynamicbox *handler, int active_update, dynamicbox_ret_cb_t cb, void *data)
{
	return livebox_set_update_mode((struct livebox*)handler, active_update, (ret_cb_t)cb, data);
}

EAPI int dynamicbox_is_active_update(struct dynamicbox *handler)
{
	return livebox_is_active_update((struct livebox*)handler);
}

EAPI int dynamicbox_resize(struct dynamicbox *handler, int type, dynamicbox_ret_cb_t cb, void *data)
{
	return livebox_resize((struct livebox*)handler, type, (ret_cb_t)cb, data);
}

EAPI int dynamicbox_click(struct dynamicbox *handler, double x, double y)
{
	return livebox_click((struct livebox*)handler, x, y);
}

EAPI int dynamicbox_has_pd(struct dynamicbox *handler)
{
	return livebox_has_pd((struct livebox*)handler);
}

EAPI int dynamicbox_pd_is_created(struct dynamicbox *handler)
{
	return livebox_pd_is_created((struct livebox*)handler);
}

EAPI int dynamicbox_create_pd(struct dynamicbox *handler, dynamicbox_ret_cb_t cb, void *data)
{
	return livebox_create_pd((struct livebox*)handler, (ret_cb_t)cb, data);
}

EAPI int dynamicbox_create_pd_with_position(struct dynamicbox *handler, double x, double y, dynamicbox_ret_cb_t cb, void *data)
{
	return livebox_create_pd_with_position((struct livebox*)handler, x, y, (ret_cb_t)cb, data);
}

EAPI int dynamicbox_move_pd(struct dynamicbox *handler, double x, double y)
{
	return livebox_move_pd((struct livebox*)handler, x, y);
}

EAPI int dynamicbox_activate(const char *pkgname, dynamicbox_ret_cb_t cb, void *data)
{
	return livebox_activate(pkgname, (ret_cb_t)cb, data);
}

EAPI int dynamicbox_destroy_pd(struct dynamicbox *handler, dynamicbox_ret_cb_t cb, void *data)
{
	return livebox_destroy_pd((struct livebox*)handler, (ret_cb_t)cb, data);
}

EAPI int dynamicbox_access_event(struct dynamicbox *handler, enum dynamicbox_access_event_type type, double x, double y, dynamicbox_ret_cb_t cb, void *data)
{
	return livebox_access_event((struct livebox*)handler, type, x, y, (ret_cb_t)cb, data);
}

EAPI int dynamicbox_mouse_event(struct dynamicbox *handler, enum dynamicbox_content_event_type type, double x, double y)
{
	return livebox_mouse_event((struct livebox*)handler, type, x, y);
}

EAPI int dynamicbox_key_event(struct dynamicbox *handler, enum dynamicbox_content_event_type type, unsigned int keycode, dynamicbox_ret_cb_t cb, void *data)
{
	return livebox_key_event((struct livebox*) handler, type, keycode, (ret_cb_t)cb, data);
}

EAPI const char *dynamicbox_filename(struct dynamicbox *handler)
{
	return livebox_filename((struct livebox*)handler);
}

EAPI int dynamicbox_get_pdsize(struct dynamicbox *handler, int *w, int *h)
{
	return livebox_get_pdsize((struct livebox*) handler, w, h);
}

EAPI int dynamicbox_size(struct dynamicbox *handler)
{
	return livebox_size((struct livebox*) handler);
}

EAPI int dynamicbox_set_group(struct dynamicbox *handler, const char *cluster, const char *category, dynamicbox_ret_cb_t cb, void *data)
{
	return livebox_set_group((struct livebox*) handler, cluster, category, (ret_cb_t)cb, data);
}

EAPI int dynamicbox_get_group(struct dynamicbox *handler, const char **cluster, const char **category)
{
	return livebox_get_group((struct livebox*) handler, cluster, category);
}

EAPI int dynamicbox_get_supported_sizes(struct dynamicbox *handler, int *cnt, int *size_list)
{
	return livebox_get_supported_sizes((struct livebox*) handler, cnt, size_list);
}

EAPI const char *dynamicbox_pkgname(struct dynamicbox *handler)
{
	return livebox_pkgname((struct livebox*) handler);
}

EAPI double dynamicbox_priority(struct dynamicbox *handler)
{
	return livebox_priority((struct livebox*) handler);
}

#if 0 // those are unused functions
EAPI int dynamicbox_delete_cluster(const char *cluster, dynamicbox_ret_cb_t cb, void *data)
{
	return livebox_delete_cluster(cluster, (ret_cb_t)cb, data);
}

EAPI int dynamicbox_delete_category(const char *cluster, const char *category, dynamicbox_ret_cb_t cb, void *data)
{
	return livebox_delete_category(cluster, category, (ret_cb_t)cb, data);
}
#endif

EAPI enum dynamicbox_dbox_type dynamicbox_dbox_type(struct dynamicbox *handler)
{
	return livebox_lb_type((struct livebox*) handler);
}

EAPI enum dynamicbox_pd_type dynamicbox_pd_type(struct dynamicbox *handler)
{
	return livebox_pd_type((struct livebox*) handler);
}

EAPI int dynamicbox_set_pd_text_handler(struct dynamicbox *handler, struct dynamicbox_script_operators *ops)
{
	return livebox_set_pd_text_handler((struct livebox*) handler, (struct livebox_script_operators*) ops);
}

EAPI int dynamicbox_set_text_handler(struct dynamicbox *handler, struct dynamicbox_script_operators *ops)
{
	return livebox_set_text_handler((struct livebox*) handler, (struct livebox_script_operators*) ops);
}

EAPI int dynamicbox_acquire_dbox_pixmap(struct dynamicbox *handler, dynamicbox_ret_cb_t cb, void *data)
{
	return livebox_acquire_lb_pixmap((struct livebox*) handler, (ret_cb_t)cb, data);
}

EAPI int dynamicbox_release_dbox_pixmap(struct dynamicbox *handler, int pixmap)
{
	return livebox_release_lb_pixmap((struct livebox*) handler, pixmap);
}

EAPI int dynamicbox_acquire_pd_pixmap(struct dynamicbox *handler, dynamicbox_ret_cb_t cb, void *data)
{
	return livebox_acquire_pd_pixmap((struct livebox*) handler, (ret_cb_t)cb, data);
}

EAPI int dynamicbox_pd_pixmap(const struct dynamicbox *handler)
{
	return livebox_pd_pixmap((const struct livebox*) handler);
}

EAPI int dynamicbox_dbox_pixmap(const struct dynamicbox *handler)
{
	return livebox_lb_pixmap((const struct livebox*) handler);
}

EAPI int dynamicbox_release_pd_pixmap(struct dynamicbox *handler, int pixmap)
{
	return livebox_release_pd_pixmap((struct livebox*) handler, pixmap);
}

EAPI void *dynamicbox_acquire_fb(struct dynamicbox *handler)
{
	return livebox_acquire_fb((struct livebox*) handler);
}

EAPI int dynamicbox_release_fb(void *buffer)
{
	return livebox_release_fb(buffer);
}

EAPI int dynamicbox_fb_refcnt(void *buffer)
{
	return livebox_fb_refcnt(buffer);
}

EAPI void *dynamicbox_acquire_pdfb(struct dynamicbox *handler)
{
	return livebox_acquire_pdfb((struct livebox*) handler);
}

EAPI int dynamicbox_release_pdfb(void *buffer)
{
	return livebox_release_pdfb(buffer);
}

EAPI int dynamicbox_pdfb_refcnt(void *buffer)
{
	return livebox_pdfb_refcnt(buffer);
}

EAPI int dynamicbox_pdfb_bufsz(struct dynamicbox *handler)
{
	return livebox_pdfb_bufsz((struct livebox*) handler);
}

EAPI int dynamicbox_lbfb_bufsz(struct dynamicbox *handler)
{
	return livebox_lbfb_bufsz((struct livebox*) handler);
}

EAPI int dynamicbox_is_user(struct dynamicbox *handler)
{
	return livebox_is_user((struct livebox*) handler);
}

EAPI int dynamicbox_set_pinup(struct dynamicbox *handler, int flag, dynamicbox_ret_cb_t cb, void *data)
{
	return livebox_set_pinup((struct livebox*) handler, flag, (ret_cb_t)cb, data);
}

EAPI int dynamicbox_is_pinned_up(struct dynamicbox *handler)
{
	return livebox_is_pinned_up((struct livebox*) handler);
}

EAPI int dynamicbox_has_pinup(struct dynamicbox *handler)
{
	return livebox_has_pinup((struct livebox*) handler);
}

EAPI int dynamicbox_set_data(struct dynamicbox *handler, void *data)
{
	return livebox_set_data((struct livebox*) handler, data);
}

EAPI void *dynamicbox_get_data(struct dynamicbox *handler)
{
	return livebox_get_data((struct livebox*) handler);
}

EAPI int dynamicbox_is_exists(const char *pkgname)
{
	return livebox_is_exists(pkgname);
}

EAPI const char *dynamicbox_content(struct dynamicbox *handler)
{
	return livebox_content((struct livebox*) handler);
}

EAPI const char *dynamicbox_category_title(struct dynamicbox *handler)
{
	return livebox_category_title((struct livebox*) handler);
}

EAPI int dynamicbox_emit_text_signal(struct dynamicbox *handler, const char *emission, const char *source, double sx, double sy, double ex, double ey, dynamicbox_ret_cb_t cb, void *data)
{
	return livebox_emit_text_signal((struct livebox*) handler, emission, source, sx, sy, ex, ey, (ret_cb_t)cb, data);
}

EAPI int dynamicbox_subscribe_group(const char *cluster, const char *category)
{
	return livebox_subscribe_group(cluster, category);
}

EAPI int dynamicbox_unsubscribe_group(const char *cluster, const char *category)
{
	return livebox_unsubscribe_group(cluster, category);
}

EAPI int dynamicbox_refresh(struct dynamicbox *handler, int force)
{
	return livebox_refresh((struct livebox*) handler, force);
}

EAPI int dynamicbox_refresh_group(const char *cluster, const char *category, int force)
{
	return livebox_refresh_group(cluster, category, force);
}

EAPI int dynamicbox_set_visibility(struct dynamicbox *handler, enum dynamicbox_visible_state state)
{
	return livebox_set_visibility((struct livebox*) handler, state);
}

EAPI enum dynamicbox_visible_state dynamicbox_visibility(struct dynamicbox *handler)
{
	return livebox_visibility((struct livebox*) handler);
}

EAPI int dynamicbox_client_paused(void)
{
	return livebox_client_paused();
}

EAPI int dynamicbox_client_resumed(void)
{
	return livebox_client_resumed();
}

EAPI int dynamicbox_sync_dbox_fb(struct dynamicbox *handler)
{
	return livebox_sync_lb_fb((struct livebox*) handler);
}

EAPI int dynamicbox_sync_pd_fb(struct dynamicbox *handler)
{
	return livebox_sync_pd_fb((struct livebox*) handler);
}

EAPI const char *dynamicbox_alt_icon(struct dynamicbox *handler)
{
	return livebox_alt_icon((struct livebox*) handler);
}

EAPI const char *dynamicbox_alt_name(struct dynamicbox *handler)
{
	return livebox_alt_name((struct livebox*) handler);
}

EAPI int dynamicbox_acquire_fb_lock(struct dynamicbox *handler, int is_pd)
{
	return livebox_acquire_fb_lock((struct livebox*) handler, is_pd);
}

EAPI int dynamicbox_release_fb_lock(struct dynamicbox *handler, int is_pd)
{
	return livebox_release_fb_lock((struct livebox*) handler, is_pd);
}

EAPI int dynamicbox_set_option(enum dynamicbox_option_type option, int state)
{
	return livebox_set_option(option, state);
}

EAPI int dynamicbox_option(enum dynamicbox_option_type option)
{
	return livebox_option(option);
}

EAPI int dynamicbox_set_auto_launch_handler(int (*dbox_launch_handler)(struct dynamicbox *handler, const char *appid, void *data), void *data)
{
	typedef int (*launch_handler)(struct livebox *handler, const char *appid, void *data);

	return livebox_set_auto_launch_handler((launch_handler)dbox_launch_handler, data);
}

/* End of a file */
