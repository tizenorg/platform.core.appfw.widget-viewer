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

struct cb_info {
    dynamicbox_ret_cb cb;
    void *data;
};

extern void dbox_invoke_event_handler(struct dynamicbox *handler, dynamicbox_event_type_e event);
extern void dbox_invoke_fault_handler(dynamicbox_fault_type_e type, const char *pkgname, const char *filename, const char *function);

extern struct dynamicbox_common *dbox_find_common_handle(const char *pkgname, const char *filename);
extern struct dynamicbox *dbox_new_dynamicbox(const char *pkgname, const char *id, double timestamp, const char *cluster, const char *category);
extern struct dynamicbox_common *dbox_find_common_handle_by_timestamp(double timestamp);

extern int dbox_set_group(struct dynamicbox_common *common, const char *cluster, const char *category);
extern void dbox_set_size(struct dynamicbox_common *common, int w, int h);
extern void dbox_set_gbarsize(struct dynamicbox_common *common, int w, int h);
extern void dbox_set_default_gbarsize(struct dynamicbox_common *common, int w, int h);
extern int dbox_set_content(struct dynamicbox_common *common, const char *content);
extern int dbox_set_title(struct dynamicbox_common *handler, const char *title);
extern void dbox_set_auto_launch(struct dynamicbox_common *handler, const char *auto_launch);
extern void dbox_set_id(struct dynamicbox_common *handler, const char *id);
extern void dbox_set_size_list(struct dynamicbox_common *handler, int size_list);
extern void dbox_set_priority(struct dynamicbox_common *handler, double priority);
extern int dbox_set_dbox_fb(struct dynamicbox_common *handler, const char *filename);
extern int dbox_set_gbar_fb(struct dynamicbox_common *handler, const char *filename);
extern struct fb_info *dbox_get_gbar_fb(struct dynamicbox_common *handler);
extern struct fb_info *dbox_get_dbox_fb(struct dynamicbox_common *handler);
extern void dbox_set_user(struct dynamicbox_common *handler, int user);
extern void dbox_set_pinup(struct dynamicbox_common *handler, int pinup);
extern void dbox_set_text_dbox(struct dynamicbox_common *handler);
extern void dbox_set_text_gbar(struct dynamicbox_common *handler);
extern int dbox_text_dbox(struct dynamicbox_common *handler);
extern int dbox_text_gbar(struct dynamicbox_common *handler);
extern void dbox_set_period(struct dynamicbox_common *handler, double period);
extern void dbox_set_update_mode(struct dynamicbox_common *handler, int active_mode);
extern void dbox_set_filename(struct dynamicbox_common *handler, const char *filename);
extern void dbox_unlink_filename(struct dynamicbox_common *common);
extern void dbox_set_alt_icon(struct dynamicbox_common *handler, const char *icon);
extern void dbox_set_alt_name(struct dynamicbox_common *handle, const char *name);
extern int dbox_destroy_common_handle(struct dynamicbox_common *common);
extern struct dynamicbox_common *dbox_create_common_handle(struct dynamicbox *handle, const char *pkgname, const char *cluster, const char *category);
extern int dbox_sync_gbar_fb(struct dynamicbox_common *common);
extern int dbox_sync_dbox_fb(struct dynamicbox_common *common);
extern int dbox_common_unref(struct dynamicbox_common *common, struct dynamicbox *handle);
extern int dbox_common_ref(struct dynamicbox_common *common, struct dynamicbox *handle);
extern struct dynamicbox_common *dbox_find_sharable_common_handle(const char *pkgname, const char *content, int w, int h, const char *cluster, const char *category);
extern struct dynamicbox *dbox_find_dbox_in_show(struct dynamicbox_common *common);
extern struct dynamicbox *dbox_get_dbox_nth(struct dynamicbox_common *common, int nth);
extern void *dbox_remove_event_handler(dynamicbox_event_handler_cb dbox_cb);
extern int dbox_add_event_handler(dynamicbox_event_handler_cb dbox_cb, void *data);
extern int dbox_add_fault_handler(dynamicbox_fault_handler_cb dbox_cb, void *data);
extern void *dbox_remove_fault_handler(dynamicbox_fault_handler_cb dbox_cb);
extern struct cb_info *dbox_create_cb_info(dynamicbox_ret_cb cb, void *data);
extern void dbox_destroy_cb_info(struct cb_info *info);

extern struct dynamicbox *dbox_ref(struct dynamicbox *handler);
extern struct dynamicbox *dbox_unref(struct dynamicbox *handler, int destroy_common);
extern int dbox_send_delete(struct dynamicbox *handler, int type, dynamicbox_ret_cb cb, void *data);
extern int dbox_delete_all(void);

typedef enum dynamicbox_state {
    DBOX_STATE_CREATE = 0xBEEFbeef,
    DBOX_STATE_DELETE = 0xDEADdead, /* Delete only for this client */
    DBOX_STATE_DESTROYED = 0x00DEAD00
} dynamicbox_state_e;

struct dynamicbox_common {
    dynamicbox_state_e state;

    struct dlist *dynamicbox_list;
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

    dynamicbox_delete_type_e delete_type;

    int is_user;
    int is_gbar_created;
    int is_pinned_up;
    int is_active_update;
    dynamicbox_visible_state_e visible;

    struct {
        dynamicbox_dbox_type_e type;
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

        /* For the extra buffer */
        unsigned int *extra_buffer;
        int last_extra_buffer_idx;

        /* Lock */
        dynamicbox_lock_info_t lock;

        /* For damaged region */
        struct dynamicbox_damage_region last_damage;
    } dbox;

    struct {
        dynamicbox_gbar_type_e type;
        struct fb_info *fb;

        int width;
        int height;

        int default_width;
        int default_height;

        /* For the filtering event */
        double x;
        double y;

        /* For the extra buffer */
        unsigned int *extra_buffer;
        int last_extra_buffer_idx;

        /* Lock */
        dynamicbox_lock_info_t lock;

        /* For damaged region */
        struct dynamicbox_damage_region last_damage;
    } gbar;

    int nr_of_sizes;

    struct requested_flag {
        unsigned int created:1;
        unsigned int deleted:1;
        unsigned int pinup:1;
        unsigned int group_changed:1;
        unsigned int period_changed:1;
        unsigned int size_changed:1;
        unsigned int gbar_created:1;
        unsigned int gbar_destroyed:1;
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
    struct dynamicbox *handle;
    dynamicbox_ret_cb cb;
    int ret;
    void *data;
};

struct dynamicbox {
    dynamicbox_state_e state;

    int refcnt;
    int paused_updating;

    dynamicbox_visible_state_e visible;
    struct dynamicbox_common *common;

    void *data;

    struct callback_table {
        struct dynamicbox_script_operators dbox_ops;
        struct dynamicbox_script_operators gbar_ops;

        struct created {
            dynamicbox_ret_cb cb;
            void *data;
        } created;

        struct deleted {
            dynamicbox_ret_cb cb;
            void *data;
        } deleted;

        struct pinup {
            dynamicbox_ret_cb cb;
            void *data;
        } pinup;

        struct group_changed {
            dynamicbox_ret_cb cb;
            void *data;
        } group_changed;

        struct period_changed {
            dynamicbox_ret_cb cb;
            void *data;
        } period_changed;

        struct size_changed {
            dynamicbox_ret_cb cb;
            void *data;
        } size_changed;

        struct gbar_created {
            dynamicbox_ret_cb cb;
            void *data;
        } gbar_created;

        struct gbar_destroyed {
            dynamicbox_ret_cb cb;
            void *data;
        } gbar_destroyed;

        struct update_mode {
            dynamicbox_ret_cb cb;
            void *data;
        } update_mode;

        struct access_event {
            dynamicbox_ret_cb cb;
            void *data;
        } access_event;

        struct key_event {
            dynamicbox_ret_cb cb;
            void *data;
        } key_event;
    } cbs;
};

/* End of a file */
