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

enum livebox_status {
	LB_STATUS_SUCCESS = 0x00000000,
	LB_STATUS_ERROR = 0x80000000,
	LB_STATUS_ERROR_INVALID = LB_STATUS_ERROR | 0x0001,
	LB_STATUS_ERROR_FAULT = LB_STATUS_ERROR | 0x0002,
	LB_STATUS_ERROR_MEMORY = LB_STATUS_ERROR | 0x0004,
	LB_STATUS_ERROR_EXIST = LB_STATUS_ERROR | 0x0008,
	LB_STATUS_ERROR_BUSY = LB_STATUS_ERROR | 0x0010,
	LB_STATUS_ERROR_PERMISSION = LB_STATUS_ERROR | 0x0020,
	LB_STATUS_ERROR_ALREADY = LB_STATUS_ERROR | 0x0040,
	LB_STATUS_ERROR_CANCEL = LB_STATUS_ERROR | 0x0080,
	LB_STATUS_ERROR_IO = LB_STATUS_ERROR | 0x00100,
};

#define LB_STATUS_IS_ERROR(s)	(!!((s) & LB_STATUS_ERROR))

/* End of a file */
