/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#include <Ecore_X.h>

#include "elmdemo_util.h"
#include "elmdemo_test.h"

///This variable is used only for emulator.
static Eina_Bool portrait_mode = EINA_TRUE;

static void* rotate_cb_data = NULL;
static int cur_rot_degree = 0;
static Eina_Bool rotation_mode = EINA_TRUE;

static char fullpath[1024];

static int _file_owner_change(uid_t uid, gid_t gid)
{
	struct stat    statbuf;
	struct dirent  *dirp;
	DIR            *dp;
	char           *ptr;
	int            ret = 0;

	if (lstat(fullpath, &statbuf) < 0)
	{
		fprintf(stderr, "%s : stat error \n", fullpath);
		return 0;
	}
	if (S_ISDIR(statbuf.st_mode) == 0)
		return 0;

	ptr = fullpath + strlen(fullpath);
	*ptr++ = '/';
	*ptr = 0;

	if ((dp = opendir(fullpath)) == NULL)
	{
		fprintf(stderr, "can't read %s directory \n", fullpath);
		return 0;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		if (strcmp(dirp->d_name, ".") == 0  ||
				strcmp(dirp->d_name, "..") == 0)
			continue;

		strcpy(ptr, dirp->d_name);
		if ((chown(fullpath, uid, gid) == -1))
			fprintf(stderr, "%s chown error \n", fullpath);

		if ((ret = _file_owner_change(uid, gid)) != 0)
			break;
	}
	*(ptr-1) = 0;

	if (closedir(dp) < 0)
		fprintf(stderr, "can't close directory %s", fullpath);

	return ret;
}

// Save config file as app owner
// because demo program is running as root,
// so other applications cannot read new config file.
void change_config_owner()
{
	struct stat file_info;
	int ret = 0;

	if ((ret = lstat("/home/app", &file_info) == -1))
	{
		printf("error : can't get file stat \n");
	}

	strcpy(fullpath, "/home/app/.elementary");
	if ((chown(fullpath, file_info.st_uid, file_info.st_gid)) == -1)
		fprintf(stderr, "!!!! chown error \n");
	_file_owner_change(file_info.st_uid, file_info.st_gid);
}

void set_rotation_mode(Eina_Bool rot_mode)
{
	rotation_mode = rot_mode;
}

Eina_Bool get_rotation_mode()
{
	return rotation_mode;
}

int get_rotation_degree()
{
	return cur_rot_degree;
}

void set_rotation_degree(int degree)
{
	cur_rot_degree = degree;
}

void set_portrait_mode(Eina_Bool on)
{
	portrait_mode = on;
}

/*
 * is_portrait_mode()
 * This function can be called right after we enter a certain menu,
 * and check whether the device is already rotated or not.
 */
Eina_Bool is_portrait_mode()
{
	return portrait_mode;
}

