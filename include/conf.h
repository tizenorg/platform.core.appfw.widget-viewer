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

/*!
 * \note
 * milli seconds
 */
#define MAX_LOG_FILE	3
#define MAX_LOG_LINE	1000
#define SLAVE_LOG_PATH "/opt/usr/share/live_magazine/log"

#if !defined(VCONFKEY_MASTER_STARTED)
#define VCONFKEY_MASTER_STARTED	"memory/data-provider-master/started"
#endif

#if !defined(VCONFKEY_MASTER_CLIENT_ADDR)
#define VCONFKEY_MASTER_CLIENT_ADDR "db/data-provider-master/serveraddr"
#endif

extern void conf_set_manual_sync(int flag);
extern int conf_manual_sync(void);
extern void conf_set_frame_drop_for_resizing(int flag);
extern int conf_frame_drop_for_resizing(void);

/* End of a file */
