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
#ifndef __LIVE_DATA_PROVIDER_LIVE_GEN_TEMPLATE_MAIN_H__
#define __LIVE_DATA_PROVIDER_LIVE_GEN_TEMPLATE_MAIN_H__

/*!
 * \note
 * Sync function must has not to use any callback
 */
#define _SYNC_
/*!
 * \note
 * Async function can use the callback but it has to be invoked in specified time "timeout"
 */
#define _ASYNC_

struct info {
	char *id;
	char *content;
	int size_type;
	dynamicbox_buffer_h dbox_buffer;
	char *filename;

	Ecore_Event_Handler *damage_handler;
	Ecore_X_Damage damage;

	MMHandleType player;
	enum play_state {
		PLAYER_CREATED = 0x0,
		PLAYER_PLAY = 0x01,
		PLAYER_STOP = 0x02,
		PLAYER_PAUSE = 0x03,
		PLAYER_DESTROYED = 0x04,
	} player_state;

	struct down {
		int x;
		int y;
		int pressed;
	} down;
};


/*!
 * \brief
 * \param[in] pkgname
 * \return int
 */
_SYNC_ extern int dynamicbox_initialize(const char *pkgname);

/*!
 * \brief
 * \return int
 */
_SYNC_ extern int dynamicbox_finalize(void);

/*!
 * \brief
 * \param[in] id
 * \param[in] content
 * \param[in] cluster
 * \param[in] category
 * \return int
 */
_SYNC_ extern int dynamicbox_create(const char *id, const char *content, const char *cluster, const char *category);

/*!
 * \brief
 * \param[in] id
 * \return int
 */
_SYNC_ extern int dynamicbox_destroy(const char *id);

/*!
 * \brief
 * \param[in] id
 * \return int
 */
_SYNC_ extern int dynamicbox_need_to_update(const char *id);

/*!
 * \brief
 * \param[in] id
 * \return int
 */
_SYNC_ extern int dynamicbox_need_to_destroy(const char *id);

/*!
 * \brief
 * \param[in] cluster
 * \param[in] category
 * \return int
 */
_SYNC_ extern int dynamicbox_need_to_create(const char *cluster, const char *category);

/*!
 * \brief
 * \param[in] id
 * \return int
 */
_ASYNC_ extern int dynamicbox_update_content(const char *id);

/*!
 * \brief
 * \param[in] id
 * \param[in] event "clicked" only
 * \param[in] timestamp
 * \param[in] x
 * \param[in] y
 * \return int
 */
_SYNC_ extern int dynamicbox_clicked(const char *id, const char *event, double timestamp, double x, double y);

/*!
 * \brief if "source" == "id", emission could be "gbar,show", "gbar,hide", "dbox,show", "dbox,hide".
 * \param[in] id
 * \param[in] emission
 * \param[in] source
 * \param[in] event_info
 * \return int
 */
_SYNC_ extern int dynamicbox_content_event(const char *id, const char *emission, const char *source, dynamicbox_event_info_t event_info);

/*!
 * \brief
 * \param[in] id
 * \param[in] width
 * \param[in] height
 * \return int
 */
_SYNC_ extern int dynamicbox_resize(const char *id, int size_type);

/*!
 * \brief
 * \param[in] id
 * \param[in] cluster
 * \param[in] category
 * \return int
 */
_SYNC_ extern int dynamicbox_change_group(const char *id, const char *cluster, const char *category);

/*!
 * \brief
 * \param[in] id
 * \param[out] w Width of the last content
 * \param[out] h Height of the last content
 * \param[out] priority Priority of the last content
 * \param[out] *content Content info of the last content which should be usable for dynamicbox_create(..., content_info, ...);
 * \param[out] *title Only for the [CA] dynamicbox. Describe a title for your sub cluster
 * \return int
 */
_SYNC_ extern int dynamicbox_get_info(const char *id, int *w, int *h, double *priority, char **content, char **title);

/*!
 * \brief
 * \param[in] id
 * \param[in] pinup
 * \return char * String which is allocated on a Heap space.
 */
_SYNC_ extern char *dynamicbox_pinup(const char *id, int pinup);

/*!
 * \brief
 * \param[in] id
 * \return int 1 is pinned up, 0 is not pinned up
 */
_SYNC_ extern int dynamicbox_is_pinned_up(const char *id);
#endif

// End of a file
