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

/**
 * @ingroup CAPI_DYNAMICBOX_FRAMEWORK
 * @defgroup DYNAMICBOX_VIEWER_MODULE Dynamic Box Viewer
 * @brief API for dynamic box viewer (widget screen, home screen, ...)
 * @section DYNAMICBOX_VIEWER_MODULE_HEADER Required Header
 * \#include <dynamicbox.h>
 * @section DYNAMICBOX_VIEWER_MODULE_OVERVIEW Overview
 * Tizen homescreen S/W framework is supporing the Dynamic Box. (aka widget which is similiar with the android widget)
 *
 * @image html front.jpg
 *
 * @subsection WhatIsTheDynamicBox 1. What is the Dynamic Box
 * The Dynamic Box is the widget of the TIZEN.
 *
 * It works as a small application displayed on other applications' (such as homescreen, lockscreen, etc ...) view.
 * Each Dynamic Box can have (not a mandatory option) a Glance Bar (Glance Bar) in which more detailed information can be found.
 * The content of Glance Bar can be exposed when a certain gesture (e.g., flick-down) has been applied to the Dynamic Box.
 * If you are interested in developing a dynamic box, there are things you should know prior to making any source code for the box.
 * To make your Dynamic Box added to any Dynamic Box viewer application (e.g., live panel in our case), then you need to create and prepare    
 * controller(SO file), layout script (EDJE for a Glance Bar if necessary), configuration files.
 *
 * A dynamic box is managed by data provider, since each SO file of a dynamic box is loaded on and controlled by data provider using predefined ABI.
 * A viewer will receive any dynamic box's content in forms of "image file", "buffer" or "text" and display the content in various formats on its window.
 * A dynamic box developer needs to make sure that your Dynamic Box generates desirable content in-time on a explicit update-request or periodic update.
 *
 * After a data provider loads a dynamic box's SO file, it then assigns a specific "file name" for the dynamic box via an argument of a dynamic box function.
 * Since then the dynamic box just generates content using then given file name.
 * Passing an image file (whose name is the previously given name) is the basic method for providing contents to the viewer.
 * But if you want play animation or handles user event in real-time, you can use the buffer type.
 *
 * And you should prepare the content of the Glance Bar.
 * The Glance Bar is only updated by the "buffer" type. so you should prepare the layout script for it.
 * If you didn't install any script file for Glance Bar, the viewer will ignore the "flick down" event from your dynamic box.
 *
 * @subsubsection DynamicBox 1.1 Dynamic Box
 * Live box is a default content of your widget. It always displays on the screen and updated periodically.
 * It looks like below captured images.
 * @image html weather.png Weather Dynamic Box
 * @image html stock.png Stock Dynamic Box
 * @image html twitter.png Twitter Dynamic Box
 *
 * @subsubsection GlanceBar 1.2 Glance Bar
 * @image html PD.png Glance Bar
 * Glance Bar will be displayed when a user flicks down a dynamic box. (basically it depends on the implementation of the viewer applications)
 * To supports this, a developer should prepare the layout script (EDJE only for the moment) of the dynamic box's Glance Bar. (or you can use the buffer directly)
 * Data provider supports EDJE script but the developer can use various scripts if (which is BIG IF) their interpreters can be implemented based on evas & ecore.
 *
 * When a layout script has been installed, data provider can load and rendering the given layout on the buffer.
 * The content on the buffer can be shared between applications that need to display the content on their window.
 * Description data file is necessary to place proper content components in rendered layout.
 * Check this page Description Data. 
 *
 * @subsubsection ClusterCategory 1.3 What is the "cluster" and "category"
 * The cluster and the sub-cluster is just like the grouping concept.
 * It is used for creating/destorying your dynamic box instance when the data provider receives any context event from the context engine.
 * You will only get "user,created" cluster and "default" category(sub cluster) info.
 *
 * @subsection DynamicBoxContent 2. How the dynamic box can draw contents for viewer?
 * There are several ways to update the content of a dynamic box.
 *
 * @li Image file based content updating
 * @li Description file based content updating (with the layout script file)
 * @li Buffer based content updating
 *
 * Each method has specific benefit for implementing the dynamic box.
 *
 * @subsubsection ImageFormat 2.1 Via image file
 * This is the basic method for providing content of a dynamic box to the viewer application.
 * But this can be used only for the dynamic box. (Unavailable for the Glance Bar).
 * When your dynamic box is created, the provider will assign an unique ID for your dynamic box(it would be a filename).
 * You should keep that ID until your dynamic box is running. The ID will be passed to you via dynamicbox_create function.
 * \image html image_format.png
 *
 * When you need to update the output of your dynamic box, you should generate the image file using given ID(filename).
 * Then the data provider will recognize the event of updated output of a dynamic box and it will send that event to the viewer to reload it on the screen.
 *
 * @subsubsection ScriptFormat 2.2 Via layout script
 * @image html script_format.png
 * This method is supported for static layout & various contents (text & image)
 * When you develop your dynamic box, first design the layout of box content using script (edje is default)
 * Then the provider will load it to the content buffer and start rendering.
 * After the sciprt is loaded, you can fill it using description data format.
 * libdynamicbox defines description data handling functions.
 *
 * @subsubsection TextFormat 2.3 Via text data
 * @image html text_format.png
 * This is the simplified method to update the content of dynamic box.
 * So your box only need to update the text data using description data format.
 * Then the viewer will parse it to fill its screen.
 * So there is no buffer area, just viewer decide how handles it.
 *
 * @subsubsection BufferFormat 2.4 Via buffer
 * This method is very complex to implement.
 * The provider will give a content buffer to you, then your box should render its contents on this buffer.
 * This type is only supported for 3rd party dynamic box such as OSP and WEB.
 * Inhouse(EFL) dynamic box is not able to use this buffer type for the box content.
 *
 * @subsection DynamicBoxDirectory 3. Dynamic Box directory hierachy
 * @image html preload_folder.png
 * @image html download_folder.png
 *
 * @subsection WritingViewerApp 4. Writing a new application for displaying Dynamic Boxes
 * If you want install dynamic boxes on your application screen, you should initialize the viewer system first.
 *
 * @code
 * extern int dynamicbox_init(void *disp, int prevent_overwrite, double event_filter, int use_thread);
 * @endcode
 *
 * @a disp should be current display object. if we are on X11 based windowing system, it will give you a Display Object, when you connect to X Server.
 * Viewer application also needs it to preparing rendering buffer to display contents of dynamic boxes.
 *
 * @a prevent_overwirte flag is used for image or script type dynamic boxes.
 * If this option is turn on, the viewer library will copy the image file of dyanmic box content to "reader" folder.
 * To prevent from overwriting content image file.
 *
 * @a event_filter is used for feeding events.
 * Basically, the dynamic box can be feed touch event by viewer application or master widget controller. (aka, data-provider-master).
 * If a viewer feeds event to the dynamic box, it could more slow than data-provider-master's direct feeding.
 * But sometimes, the viewer requires to feeds event by itself.
 * In that case, we should choose the feeding option. feeding every events can be slow down.
 * To save it, this event_filter will be used. if the event is generated in this time-gap, it will be ignored.
 *
 * @a use_thread if this flag is turned on, the viewer library will create a new thread for handling the IPC packets only.
 * It will helps to increase the throughput of main thread. because it will not be interrupted to handles IPC packets.
 *
 * After the viewer is initiated, you can create a new box and locate it in your screen.
 *
 * Opposite function is "dynamicbox_fini"
 *
 * @code
 * extern int dynamicbox_fini(void);
 * @endcode
 *
 * Here is a sample code
 *
 * @code
 * #include <stdio.h>
 * #include <errno.h>
 * #include <dynamicbox.h>
 * #include <app.h>
 *
 * #include <dlog.h>
 *
 * int errno;
 *
 * static bool _create_cb(void *data)
 * {
 *      int ret;
 *	ret = dynamicbox_init(NULL, 1, 0.0f, 1);
 *      if (ret != DBOX_STATUS_ERROR_NONE) {
 *          LOGE("Failed to initialize the dynamic box viewer");
 *      }
 *      return true;
 * }
 *
 * static void _terminate_cb(void *data)
 * {
 *     dynamicbox_fini();
 * }
 *
 * int main(int argc, char *argv[])
 * {
 *     app_event_callback_s event_callback;
 *     event_callback.create = _create_cb;
 *     event_callback.terminate = _terminate_cb;
 *     event_callback.pause = _pause_cb;
 *     event_callback.resume = _resume_cb;
 *     event_callback.app_control = _app_control;
 *     event_callback.low_memory = NULL;
 *     event_callback.low_battery = NULL;
 *     event_callback.device_orientation = NULL;
 *     event_callback.language_changed = _language_changed;
 *     event_callback.region_format_changed = NULL;
 *     
 *     return app_efl_main(&argc, &argv, &event_callback, &main_info);
 * }
 * @endocde
 *
 * If you want add a new dynamic box, you can call "dynamicbox_add()" function.
 *
 */
