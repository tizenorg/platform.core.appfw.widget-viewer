extern int lb_set_group(struct livebox *handler, const char *cluster, const char *category);
extern void lb_set_size(struct livebox *handler, int w, int h);
extern void lb_set_pdsize(struct livebox *handler, int w, int h);
extern void lb_invoke_event_handler(struct livebox *handler, const char *event);
extern void lb_invoke_fault_handler(const char *event, const char *pkgname, const char *filename, const char *function);
extern int lb_set_content(struct livebox *handler, const char *content);
extern void lb_set_auto_launch(struct livebox *handler, int auto_launch);
extern struct livebox *lb_find_livebox(const char *pkgname, const char *filename);
extern struct livebox *lb_new_livebox(const char *pkgname, const char *filename, double timestamp);
extern struct livebox *lb_find_livebox_by_timestamp(double timestamp);
extern void lb_set_filename(struct livebox *handler, const char *filename);
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
extern int lb_send_delete(struct livebox *handler);

struct livebox {
	int refcnt;
	enum {
		CREATE = 0xBEEFbeef,
		DELETE = 0xDEADdead, /* Delete only for this client */
	} state;

	char *cluster;
	char *category;

	char *pkgname;
	char *filename;
	char *content;

	double timestamp;
	double event_timestamp;

	int is_user;

	struct {
		enum {
			LB_FILE,
			LB_FB,
			LB_TEXT,
		} type;

		union {
			struct fb_info *fb;
			struct livebox_script_operators ops;
		} data;

		int size_list;

		int width;
		int height;
		double priority;

		int auto_launch;
		double period;
		int is_pinned_up;
		int pinup_supported;
	} lb;

	struct {
		enum {
			PD_FILE,
			PD_FB,
			PD_TEXT,
		} type;

		union {
			struct fb_info *fb;
			struct livebox_script_operators ops;
		} data;

		int width;
		int height;
	} pd;

	int nr_of_sizes;

	void *data;

	ret_cb_t created_cb;
	void *created_cbdata;
	int is_created;

	ret_cb_t deleted_cb;
	void *deleted_cbdata;
};

/* End of a file */
