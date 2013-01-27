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

extern int lb_set_group(struct livebox *handler, const char *cluster, const char *category);
extern void lb_set_size(struct livebox *handler, int w, int h);
extern void lb_set_pdsize(struct livebox *handler, int w, int h);
extern void lb_invoke_event_handler(struct livebox *handler, enum livebox_event_type event);
extern void lb_invoke_fault_handler(enum livebox_fault_type type, const char *pkgname, const char *filename, const char *function);
extern int lb_set_content(struct livebox *handler, const char *content);
extern int lb_set_title(struct livebox *handler, const char *title);
extern void lb_set_auto_launch(struct livebox *handler, const char *auto_launch);
extern struct livebox *lb_find_livebox(const char *pkgname, const char *filename);
extern struct livebox *lb_new_livebox(const char *pkgname, const char *filename, double timestamp);
extern struct livebox *lb_find_livebox_by_timestamp(double timestamp);
extern void lb_set_id(struct livebox *handler, const char *id);
extern void lb_set_size_list(struct livebox *handler, int size_list);
extern void lb_set_priority(struct livebox *handler, double priority);
extern int lb_set_lb_fb(struct livebox *handler, const char *filename);
extern int lb_set_pd_fb(struct livebox *handler, const char *filename);
extern struct fb_info *lb_get_pd_fb(struct livebox *handler);
extern struct fb_info *lb_get_lb_fb(struct livebox *handler);
extern void lb_set_user(struct livebox *handler, int user);
extern void lb_set_pinup(struct livebox *handler, int pinup);
extern void lb_set_text_lb(struct livebox *handler);
extern void lb_set_text_pd(struct livebox *handler);
extern int lb_text_lb(struct livebox *handler);
extern int lb_text_pd(struct livebox *handler);
extern void lb_set_period(struct livebox *handler, double period);
extern struct livebox *lb_ref(struct livebox *handler);
extern struct livebox *lb_unref(struct livebox *handler);
extern int lb_send_delete(struct livebox *handler, ret_cb_t cb, void *data);
extern int lb_delete_all(void);

enum lb_type { /*!< Must have to be sync with data-provider-master */
	_LB_TYPE_NONE = 0x0,
	_LB_TYPE_SCRIPT,
	_LB_TYPE_FILE,
	_LB_TYPE_TEXT,
	_LB_TYPE_BUFFER,
};

enum pd_type { /*!< Must have to be sync with data-provider-master */
	_PD_TYPE_NONE = 0x0,
	_PD_TYPE_SCRIPT,
	_PD_TYPE_TEXT,
	_PD_TYPE_BUFFER,
};

struct livebox {
	int refcnt;
	enum {
		CREATE = 0xBEEFbeef,
		DELETE = 0xDEADdead, /* Delete only for this client */
		DESTROYED = 0x00DEAD00,
	} state;

	char *cluster;
	char *category;

	char *pkgname;
	char *id;
	char *content;
	char *title;
	char *filename;

	double timestamp;

	enum livebox_visible_state visible;

	int is_user;
	int is_pd_created;
	int is_pinned_up;

	struct {
		enum lb_type type;
		union {
			struct fb_info *fb;
			struct livebox_script_operators ops;
		} data;

		int size_list;

		int width;
		int height;
		double priority;

		char *auto_launch;
		double period;
		int pinup_supported;
		int mouse_event;

		/* For the filtering event */
		double x;
		double y;
	} lb;

	struct {
		enum pd_type type;
		union {
			struct fb_info *fb;
			struct livebox_script_operators ops;
		} data;

		int width;
		int height;

		/* For the filtering event */
		double x;
		double y;
	} pd;

	int nr_of_sizes;

	void *data;

	ret_cb_t created_cb;
	void *created_cbdata;

	ret_cb_t deleted_cb;
	void *deleted_cbdata;

	ret_cb_t pinup_cb;
	void *pinup_cbdata;

	ret_cb_t group_changed_cb;
	void *group_cbdata;

	ret_cb_t period_changed_cb;
	void *period_cbdata;

	ret_cb_t pd_created_cb;
	void *pd_created_cbdata;

	ret_cb_t pd_destroyed_cb;
	void *pd_destroyed_cbdata;
};

/* End of a file */
