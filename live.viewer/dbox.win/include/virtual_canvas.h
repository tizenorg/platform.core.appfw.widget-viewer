/*
 *  [live-data-provider]
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Sangil Lee <sangil77.lee@samsung.com>, Seho Chang <seho.chang@samsung.com>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __LIVE_DATA_PROVIDER_LIVE_GEN_TEMPLATE_VIRTUAL_CANVAS_H__
#define __LIVE_DATA_PROVIDER_LIVE_GEN_TEMPLATE_VIRTUAL_CANVAS_H__

extern int flush_to_file(Evas *e, const char *filename, int w, int h);
extern int flush_data_to_file(Evas *e, char *data, const char *filename, int w, int h);

extern Evas *create_virtual_canvas(int w, int h);
extern int destroy_virtual_canvas(Evas *e);
extern void get_screen_geometry(int *w, int *h);

#endif

// End of a file
