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

#ifndef __DEF_test_ElmGLViewGears_H_
#define __DEF_test_ElmGLViewGears_H_


#include <Elementary.h>
#include <Evas_GL.h>

#ifndef M_PI
#define M_PI 3.14159265
#endif

typedef struct _Gear {
   GLfloat *vertices;
   GLuint vbo;
   int count;
   int vertices_amount;
} Gear;

// GL related data here..
typedef struct _GLData
{
   Evas_GL_API *glapi;
   GLuint       program;
   GLuint       vtx_shader;
   GLuint       fgmt_shader;
   int          initialized : 1;
   int          mouse_down : 1;

   // Gear Stuff
   GLfloat      view_rotx;
   GLfloat      view_roty;
   GLfloat      view_rotz;

   Gear        *gears[50];

   GLfloat      angle;

   GLuint       proj_location;
   GLuint       light_location;
   GLuint       color_location;

   GLfloat      proj[16];
   GLfloat      light[3];

   GLfloat gears_color[50][4];
   GLfloat gears_x_position[50];
   GLfloat gears_y_position[50];
   GLfloat gears_z_position[50];


} GLData;

Evas_Object* fps_label;

void elmgears_cb(void *data, Evas_Object *obj, void *event_info);

Evas_Object *glview[4], *box;

#endif
