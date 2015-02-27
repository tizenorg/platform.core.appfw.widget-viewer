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
 * @brief Close the Glance Bar if it is opened
 * @since_tizen 2.4
 * @param[in] widget widget object
 * @return int
 */
extern int widget_viewer_evas_destroy_glance_bar(Evas_Object *widget);

/**
 * @brief Set the viewe port of given widget
 * @since_tizen 2.4
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
 * @since_tizen 2.4
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
 * @since_tizen 2.4
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
 * @since_tizen 2.4
 * @param[in] widget widget object
 * @param[in] filename Filename will be used for saving content of a widget
 * @return int
 */
extern int widget_viewer_evas_dump_to_file(Evas_Object *widget, const char *filename);
#ifdef __cplusplus
}
#endif

#endif /* __WIDGET_VIEWER_EVAS_INTERNAL_H */
