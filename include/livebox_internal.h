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

extern void lb_invoke_event_handler(struct livebox *handler, enum livebox_event_type event);
extern void lb_invoke_fault_handler(enum livebox_fault_type type, const char *pkgname, const char *filename, const char *function);

extern struct livebox_common *lb_find_common_handle(const char *pkgname, const char *filename);
extern struct livebox *lb_new_livebox(const char *pkgname, const char *id, double timestamp, const char *cluster, const char *category);
extern struct livebox_common *lb_find_common_handle_by_timestamp(double timestamp);

extern int lb_set_group(struct livebox_common *common, const char *cluster, const char *category);
extern void lb_set_size(struct livebox_common *common, int w, int h);
extern void lb_set_pdsize(struct livebox_common *common, int w, int h);
extern void lb_set_default_pdsize(struct livebox_common *common, int w, int h);
extern int lb_set_content(struct livebox_common *common, const char *content);
extern int lb_set_title(struct livebox_common *handler, const char *title);
extern void lb_set_auto_launch(struct livebox_common *handler, const char *auto_launch);
extern void lb_set_id(struct livebox_common *handler, const char *id);
extern void lb_set_size_list(struct livebox_common *handler, int size_list);
extern void lb_set_priority(struct livebox_common *handler, double priority);
extern int lb_set_lb_fb(struct livebox_common *handler, const char *filename);
extern int lb_set_pd_fb(struct livebox_common *handler, const char *filename);
extern struct fb_info *lb_get_pd_fb(struct livebox_common *handler);
extern struct fb_info *lb_get_lb_fb(struct livebox_common *handler);
extern void lb_set_user(struct livebox_common *handler, int user);
extern void lb_set_pinup(struct livebox_common *handler, int pinup);
extern void lb_set_text_lb(struct livebox_common *handler);
extern void lb_set_text_pd(struct livebox_common *handler);
extern int lb_text_lb(struct livebox_common *handler);
extern int lb_text_pd(struct livebox_common *handler);
extern void lb_set_period(struct livebox_common *handler, double period);
extern void lb_set_update_mode(struct livebox_common *handler, int active_mode);
extern void lb_set_filename(struct livebox_common *handler, const char *filename);
extern void lb_set_alt_info(struct livebox_common *handler, const char *icon, const char *name);
extern int lb_destroy_lock_file(struct livebox_common *common, int is_pd);
extern int lb_create_lock_file(struct livebox_common *common, int is_pd);
extern int lb_destroy_common_handle(struct livebox_common *common);
extern struct livebox_common *lb_create_common_handle(struct livebox *handle, const char *pkgname, const char *cluster, const char *category);
extern int lb_sync_pd_fb(struct livebox_common *common);
extern int lb_sync_lb_fb(struct livebox_common *common);
extern int lb_common_unref(struct livebox_common *common, struct livebox *handle);
extern int lb_common_ref(struct livebox_common *common, struct livebox *handle);

extern struct livebox *lb_ref(struct livebox *handler);
extern struct livebox *lb_unref(struct livebox *handler, int destroy_common);
extern int lb_send_delete(struct livebox *handler, int type, ret_cb_t cb, void *data);
extern int lb_delete_all(void);

enum lb_type { /*!< Must have to be sync with data-provider-master */
	_LB_TYPE_NONE = 0x0,
	_LB_TYPE_SCRIPT,
	_LB_TYPE_FILE,
	_LB_TYPE_TEXT,
	_LB_TYPE_BUFFER,
	_LB_TYPE_ELEMENTARY
};

enum pd_type { /*!< Must have to be sync with data-provider-master */
	_PD_TYPE_NONE = 0x0,
	_PD_TYPE_SCRIPT,
	_PD_TYPE_TEXT,
	_PD_TYPE_BUFFER,
	_PD_TYPE_ELEMENTARY
};

enum livebox_state {
	CREATE = 0xBEEFbeef,
	DELETE = 0xDEADdead, /* Delete only for this client */
	DESTROYED = 0x00DEAD00
};

struct livebox_common {
	enum livebox_state state;

	struct dlist *livebox_list;
	int refcnt;

	char *cluster;
	char *category;

	char *pkgname;
	char *id;

	char *content;
	char *title;
	char *filename;

	double timestamp;

	struct alt_info {
		char *icon;
		char *name;
	} alt;

	enum livebox_delete_type delete_type;

	int is_user;
	int is_pd_created;
	int is_pinned_up;
	int is_active_update;
	enum livebox_visible_state visible;

	struct {
		enum lb_type type;
		struct fb_info *fb;

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
		char *lock;
		int lock_fd;
	} lb;

	struct {
		enum pd_type type;
		struct fb_info *fb;

		int width;
		int height;

		int default_width;
		int default_height;

		/* For the filtering event */
		double x;
		double y;
		char *lock;
		int lock_fd;
	} pd;

	int nr_of_sizes;

	struct requested_flag {
		unsigned int created:1;
		unsigned int deleted:1;
		unsigned int pinup:1;
		unsigned int group_changed:1;
		unsigned int period_changed:1;
		unsigned int size_changed:1;
		unsigned int pd_created:1;
		unsigned int pd_destroyed:1;
		unsigned int update_mode:1;
		unsigned int access_event:1;
		unsigned int key_event:1;

		/*!
		 * \note
		 * Reserved
		 */
		unsigned int reserved:21;
	} request;
};

struct job_item {
	struct livebox *handle;
	ret_cb_t cb;
	int ret;
	void *data;
};

struct livebox {
	enum livebox_state state;

	int refcnt;
	int paused_updating;

	enum livebox_visible_state visible;
	struct livebox_common *common;

	void *data;

	struct callback_table {
		struct livebox_script_operators lb_ops;
		struct livebox_script_operators pd_ops;

		struct created {
			ret_cb_t cb;
			void *data;
		} created;

		struct deleted {
			ret_cb_t cb;
			void *data;
		} deleted;

		struct pinup {
			ret_cb_t cb;
			void *data;
		} pinup;

		struct group_changed {
			ret_cb_t cb;
			void *data;
		} group_changed;

		struct period_changed {
			ret_cb_t cb;
			void *data;
		} period_changed;

		struct size_changed {
			ret_cb_t cb;
			void *data;
		} size_changed;

		struct pd_created {
			ret_cb_t cb;
			void *data;
		} pd_created;

		struct pd_destroyed {
			ret_cb_t cb;
			void *data;
		} pd_destroyed;

		struct update_mode {
			ret_cb_t cb;
			void *data;
		} update_mode;

		struct access_event {
			ret_cb_t cb;
			void *data;
		} access_event;

		struct key_event {
			ret_cb_t cb;
			void *data;
		} key_event;
	} cbs;
};

/* End of a file */
