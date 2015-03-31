/*
 * Samsung API
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.1 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __WIDGET_VIEWER_EVAS_INTERNAL_H
#define __WIDGET_VIEWER_EVAS_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "widget_viewer_evas.h"

#define WIDGET_SMART_SIGNAL_GBAR_DESTROYED        "gbar,destroyed"        /**< GBAR is destroyed */
#define WIDGET_SMART_SIGNAL_GBAR_ABORTED          "gbar,aborted"          /**< GBAR creation is aborted */
#define WIDGET_SMART_SIGNAL_GBAR_CREATED          "gbar,created"          /**< GBAR is created */
#define WIDGET_SMART_SIGNAL_FLICKDOWN_CANCELLED   "flickdown,cancelled"   /**< Flick down is canceld */


#define WIDGET_VIEWER_EVAS_SHARED_CONTENT  0x0002	    /**< Multiple instances will share the content of one real instance */
#define WIDGET_VIEWER_EVAS_SUPPORT_GBAR    0x0004	    /**< GBAR will be used */

typedef enum widget_access_result {
    WIDGET_ACCESS_RESULT_DONE = 0x00,
    WIDGET_ACCESS_RESULT_FIRST = 0x01,
    WIDGET_ACCESS_RESULT_LAST = 0x02,
    WIDGET_ACCESS_RESULT_READ = 0x04,
    WIDGET_ACCESS_RESULT_ERROR = 0x80,
    WIDGET_ACCESS_RESULT_UNKNOWN = 0xFF
} widget_access_result_e;

/**
 * @sine_tizen 2.4
 * @brief Data structure for smart callback user parameter
 */
typedef enum widget_evas_raw_event_type {
    WIDGET_VIEWER_EVAS_RAW_DELETE = 0x00,
    WIDGET_VIEWER_EVAS_RAW_CREATE = 0x02,
    WIDGET_VIEWER_EVAS_RAW_MAX = 0xff,
} widget_evas_raw_event_type_e;

typedef struct widget_evas_raw_event_info {
    const char *pkgname;
    enum widget_evas_raw_event_type type;
    int error;
    Evas_Object *widget;
} widget_evas_raw_event_info_s;

/**
 * @brief Close the Glance Bar if it is opened
 * @since_tizen 2.3.1
 * @param[in] widget widget object
 * @return int
 */
extern int widget_viewer_evas_destroy_glance_bar(Evas_Object *widget);

/**
 * @brief Set the viewe port of given widget
 * @since_tizen 2.3.1
 * @param[in] widget
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 * @return int
 */
extern int widget_viewer_evas_set_view_port(Evas_Object *widget, int x, int y, int w, int h);

/**
 * @brief Get the current view port of given widget
 * @since_tizen 2.3.1
 * @param[in] widget
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 * @return int
 */
extern int widget_viewer_evas_get_view_port(Evas_Object *widget, int *x, int *y, int *w, int *h);


/**
 * @brief Feeds accessibility events
 * @since_tizen 2.3.1
 * @param[in] widget
 * @param[in] type
 * @param[in] info
 * @param[in] ret_cb
 * @param[in] dta
 * @return int
 */
extern int widget_viewer_evas_feed_access_event(Evas_Object *widget, int type, void *info, void (*ret_cb)(Evas_Object *obj, int ret, void *data), void *data);

/**
 * @brief Dump a contents of widget to a given filename.
 * @since_tizen 2.3.1
 * @param[in] widget widget object
 * @param[in] filename Filename will be used for saving content of a widget
 * @return int
 */
extern int widget_viewer_evas_dump_to_file(Evas_Object *widget, const char *filename);


/**
 * @brief Subscribes an event for widgets only in a given cluster and sub-cluster.
 * @details If you wrote a view-only client,
 *   you can receive the event of specific widgets which belong to a given cluster/category.
 *   But you cannot modify their attributes (such as size, ...).
 * @since_tizen 2.3.1
 * @privlevel public
 * @privilege %http://tizen.org/privilege/widget.viewer
 * @param[in] cluster Cluster ("*" can be used for subscribe all cluster's widgets event; If you use the "*", value in the category will be ignored)
 * @param[in] category Category ("*" can be used for subscribe widgets events of all category(sub-cluster) in a given "cluster")
 * @return #WIDGET_ERROR_NONE on success,
 *          otherwise an error code (see #WIDGET_ERROR_XXX) on failure
 * @retval #WIDGET_ERROR_FAULT Unrecoverable error occurred
 * @retval #WIDGET_ERROR_NONE Successfully requested
 * @see widget_viewer_evas_unsubscribe_group()
 */
extern int widget_viewer_evas_subscribe_group(const char *cluster, const char *sub_cluster);


/**
 * @brief Unsubscribes an event for the widgets, but you will receive already added widgets events.
 * @since_tizen 2.3.1
 * @privlevel public
 * @privilege %http://tizen.org/privilege/widget.viewer
 * @param[in] cluster Cluster("*" can be used for subscribe all cluster's widgets event; If you use the "*", value in the category will be ignored)
 * @param[in] category Category ("*" can be used for subscribe all sub-cluster's widgets event in a given "cluster")
 * @return #WIDGET_ERROR_NONE on success,
 *          otherwise an error code (see #WIDGET_ERROR_XXX) on failure
 * @retval #WIDGET_ERROR_FAULT Unrecoverable error occurred
 * @retval #WIDGET_ERROR_NONE Successfully requested
 * @see widget_subscribe_group()
 */
extern int widget_viewer_evas_unsubscribe_group(const char *cluster, const char *sub_cluster);

/**
 * @brief Subscribes events of widgets which is categorized by given "category" string.
 *        "category" is written in the XML file of each widget manifest file.
 *        After subscribe the category, the master will send created event for all created widgets,
 *        Also it will notify client when a new widget is created.
 * @since_tizen 2.3.1
 * @privlevel public
 * @privilege %http://tizen.org/privilege/widget.viewer
 * @param[in] category Category name
 * @return #WIDGET_ERROR_NONE on success,
 *          otherwise an error code (see #WIDGET_ERROR_XXX) on failure
 * @retval #WIDGET_ERROR_FAULT Unrecoverable error occurred
 * @retval #WIDGET_ERROR_NONE Successfully requested
 * @see widget_viewer_evas_unsubscribe_category()
 */
extern int widget_viewer_evas_subscribe_category(const char *category);

/**
 * @brief Unsubscribes events of widgets.
 * @since_tizen 2.3.1
 * @privlevel public
 * @privilege %http://tizen.org/privilege/widget.viewer
 * @param[in] category Category name
 * @return #WIDGET_ERROR_NONE on success,
 *          otherwise an error code (see #WIDGET_ERROR_XXX) on failure
 * @retval #WIDGET_ERROR_FAULT Unrecoverable error occurred
 * @retval #WIDGET_ERROR_NONE Successfully requested
 * @see widget_viewer_evas_subscribe_category()
 */
extern int widget_viewer_evas_unsubscribe_category(const char *category);

extern int widget_viewer_evas_get_instance_id(Evas_Object *widget, char **instance_id);
/**
 * @brief Callback function for handling raw event
 * @since_tizen 2.3.1
 * @param[in] info
 * @param[in] data
 * @return void
 */

typedef void (*raw_event_cb)(struct widget_evas_raw_event_info *info, void *data);

/**
 * @brief Unregister a callback function for subscribing raw event.
 * @since_tizen 2.3.1
 * @param[in] type
 * @param[in] cb
 * @param[in] data
 * @return int
 */
extern int widget_viewer_evas_unset_raw_event_callback(enum widget_evas_raw_event_type type, raw_event_cb cb, void *data);

/**
 * @brief Register a callback function for subscribing raw event.
 * @since_tizen 2.3.1
 * @param[in] type
 * @param[in] cb
 * @param[in] data
 */
extern int widget_viewer_evas_set_raw_event_callback(enum widget_evas_raw_event_type type, raw_event_cb cb, void *data);

#ifdef __cplusplus
}
#endif

#endif /* __WIDGET_VIEWER_EVAS_INTERNAL_H */
