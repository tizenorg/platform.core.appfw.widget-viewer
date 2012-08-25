/*
 * com.samsung.live-magazine
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Sung-jae Park <nicesj.park@samsung.com>, Youngjoo Park <yjoo93.park@samsung.com>
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

extern int util_check_extension(const char *filename, const char *check_ptr);
extern double util_timestamp(void);
extern const char *util_basename(const char *name);
extern int util_validate_livebox_package(const char *pkgname);
extern const char *util_uri_to_path(const char *uri);

#define SCHEMA_FILE	"file://"
#define SCHEMA_PIXMAP	"pixmap://"
#define SCHEMA_SHM	"shm://"

#define container_of(ptr, type, member) \
        ({ const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

/* End of a file */
