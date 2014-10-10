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

#include <stdlib.h>
#include "elmdemo_test.h"
#include "elmdemo_util.h"
#include "elmglviewgears.h"

static void init_gl(Evas_Object *obj);
static void resize_gl(Evas_Object *obj);
static void del_gl(Evas_Object *obj);
static void draw_gl(Evas_Object *obj);
static Eina_Bool on_animate(void *data);
static void mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void render_gears(GLData *gld);
static void gears_init(GLData *gld);
static void gears_reshape(GLData *gld, int width, int height);

// fps timer
static void print_fps(const char* filename, int per_sec);
static int view_count = 0;
static int zoom_step = 4;
static int gear_count = 1;

static int g_ut_frames = 0;
static int vertice_amount[4][50];
static struct timeval g_ut_starttime;
static struct timeval g_ut_currenttime;

static void
print_fps(const char* filename, int per_sec)
{
   char label_label[40];
   int i;
   int j;
   int total_vertice_amount = 0;

   g_ut_frames++;

   gettimeofday(&g_ut_currenttime, NULL);

   double seconds = (g_ut_currenttime.tv_sec - g_ut_starttime.tv_sec);
   seconds += (g_ut_currenttime.tv_usec - g_ut_starttime.tv_usec) / 1000000.0;

   for (i = 0; i < view_count + 1; i++)
     {
        for (j = 0; j < gear_count; j++)
          {
             total_vertice_amount += vertice_amount[i][j];
          }
     }

   if (seconds >= per_sec)
     {
        double fps = g_ut_frames / seconds;
        sprintf (label_label, "%6.3f FPS, %d Vertices", fps / (view_count + 1), total_vertice_amount);
        gettimeofday (&g_ut_starttime, NULL);
        g_ut_frames = 0;
        elm_object_text_set(fps_label, label_label);
     }


}

#define PRINT_FPS(s)    print_fps(__FILE__, s);

//--------------------------------//
// Gear Stuff....
static GLfloat *
vert(GLfloat *p, GLfloat x, GLfloat y, GLfloat z, GLfloat *n)
{
   p[0] = x;
   p[1] = y;
   p[2] = z;
   p[3] = n[0];
   p[4] = n[1];
   p[5] = n[2];

   return p + 6;
}

/*  Draw a gear wheel.  You'll probably want to call this function when
 *  building a display list since we do a lot of trig here.
 *
 *  Input:  inner_radius - radius of hole at center
 *          outer_radius - radius at center of teeth
 *          width - width of gear
 *          teeth - number of teeth
 *          tooth_depth - depth of tooth
 */
static Gear *
make_gear(GLData *gld, GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
     GLint teeth, GLfloat tooth_depth)
{
   GLint i;
   GLfloat r0, r1, r2;
   GLfloat da;
   GLfloat *v;
   Gear *gear;
   double s[5], c[5];
   GLfloat normal[3];
   const int tris_per_tooth = 20;
   Evas_GL_API *gl = gld->glapi;

   gear = (Gear*)malloc(sizeof(Gear));
   if (gear == NULL)
      return NULL;

   r0 = inner_radius;
   r1 = outer_radius - tooth_depth / 2.0;
   r2 = outer_radius + tooth_depth / 2.0;

   da = 2.0 * M_PI / teeth / 4.0;

   gear->vertices = calloc(teeth * tris_per_tooth * 3 * 6,
                           sizeof *gear->vertices);
   s[4] = 0;
   c[4] = 1;
   v = gear->vertices;
   for (i = 0; i < teeth; i++)
     {
        s[0] = s[4];
        c[0] = c[4];
        s[1] = sin(i * 2.0 * M_PI / teeth + da);
        c[1] = cos(i * 2.0 * M_PI / teeth + da);
        s[2] = sin(i * 2.0 * M_PI / teeth + da * 2);
        c[2] = cos(i * 2.0 * M_PI / teeth + da * 2);
        s[3] = sin(i * 2.0 * M_PI / teeth + da * 3);
        c[3] = cos(i * 2.0 * M_PI / teeth + da * 3);
        s[4] = sin(i * 2.0 * M_PI / teeth + da * 4);
        c[4] = cos(i * 2.0 * M_PI / teeth + da * 4);

        normal[0] = 0.0;
        normal[1] = 0.0;
        normal[2] = 1.0;

        v = vert(v, r2 * c[1], r2 * s[1], width * 0.5, normal);

        v = vert(v, r2 * c[1], r2 * s[1], width * 0.5, normal);
        v = vert(v, r2 * c[2], r2 * s[2], width * 0.5, normal);
        v = vert(v, r1 * c[0], r1 * s[0], width * 0.5, normal);
        v = vert(v, r1 * c[3], r1 * s[3], width * 0.5, normal);
        v = vert(v, r0 * c[0], r0 * s[0], width * 0.5, normal);
        v = vert(v, r1 * c[4], r1 * s[4], width * 0.5, normal);
        v = vert(v, r0 * c[4], r0 * s[4], width * 0.5, normal);

        v = vert(v, r0 * c[4], r0 * s[4], width * 0.5, normal);
        v = vert(v, r0 * c[0], r0 * s[0], width * 0.5, normal);
        v = vert(v, r0 * c[4], r0 * s[4], -width * 0.5, normal);
        v = vert(v, r0 * c[0], r0 * s[0], -width * 0.5, normal);

        normal[0] = 0.0;
        normal[1] = 0.0;
        normal[2] = -1.0;

        v = vert(v, r0 * c[4], r0 * s[4], -width * 0.5, normal);

        v = vert(v, r0 * c[4], r0 * s[4], -width * 0.5, normal);
        v = vert(v, r1 * c[4], r1 * s[4], -width * 0.5, normal);
        v = vert(v, r0 * c[0], r0 * s[0], -width * 0.5, normal);
        v = vert(v, r1 * c[3], r1 * s[3], -width * 0.5, normal);
        v = vert(v, r1 * c[0], r1 * s[0], -width * 0.5, normal);
        v = vert(v, r2 * c[2], r2 * s[2], -width * 0.5, normal);
        v = vert(v, r2 * c[1], r2 * s[1], -width * 0.5, normal);

        v = vert(v, r1 * c[0], r1 * s[0], width * 0.5, normal);

        v = vert(v, r1 * c[0], r1 * s[0], width * 0.5, normal);
        v = vert(v, r1 * c[0], r1 * s[0], -width * 0.5, normal);
        v = vert(v, r2 * c[1], r2 * s[1], width * 0.5, normal);
        v = vert(v, r2 * c[1], r2 * s[1], -width * 0.5, normal);
        v = vert(v, r2 * c[2], r2 * s[2], width * 0.5, normal);
        v = vert(v, r2 * c[2], r2 * s[2], -width * 0.5, normal);
        v = vert(v, r1 * c[3], r1 * s[3], width * 0.5, normal);
        v = vert(v, r1 * c[3], r1 * s[3], -width * 0.5, normal);
        v = vert(v, r1 * c[4], r1 * s[4], width * 0.5, normal);
        v = vert(v, r1 * c[4], r1 * s[4], -width * 0.5, normal);

        v = vert(v, r1 * c[4], r1 * s[4], -width * 0.5, normal);
     }

   gear->count = (v - gear->vertices) / 6;

   gl->glGenBuffers(1, &gear->vbo);
   gl->glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);
   gl->glBufferData(GL_ARRAY_BUFFER, gear->count * 6 * 4,
                    gear->vertices, GL_STATIC_DRAW);

   return gear;
}

static void
multiply(GLfloat *m, const GLfloat *n)
{
   GLfloat tmp[16];
   const GLfloat *row, *column;
   div_t d;
   int i, j;

   for (i = 0; i < 16; i++)
     {
        tmp[i] = 0;
        d = div(i, 4);
        row = n + d.quot * 4;
        column = m + d.rem;
        for (j = 0; j < 4; j++)
           tmp[i] += row[j] * column[j * 4];
     }
   memcpy(m, &tmp, sizeof tmp);
}

static void
rotate(GLfloat *m, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
   double s, c;

   s = sin(angle);
   c = cos(angle);
   GLfloat r[16] =
     {
        x * x * (1 - c) + c,     y * x * (1 - c) + z * s, x * z * (1 - c) - y * s, 0,
        x * y * (1 - c) - z * s, y * y * (1 - c) + c,     y * z * (1 - c) + x * s, 0,
        x * z * (1 - c) + y * s, y * z * (1 - c) - x * s, z * z * (1 - c) + c,     0,
        0, 0, 0, 1
     };

   multiply(m, r);
}

static void
translate(GLfloat *m, GLfloat x, GLfloat y, GLfloat z)
{
   GLfloat t[16] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  x, y, z, 1 };

   multiply(m, t);
}


static void
draw_gear(GLData *gld, Gear *gear, GLfloat *m,
          GLfloat x, GLfloat y, GLfloat z,GLfloat angle, const GLfloat *color)
{
   Evas_GL_API *gl = gld->glapi;
   GLfloat tmp[16];

   memcpy(tmp, m, sizeof tmp);
   translate(tmp, x, y, z);
   rotate(tmp, 2 * M_PI * angle / 360.0, 0, 0, 1);
   gl->glUniformMatrix4fv(gld->proj_location, 1, GL_FALSE, tmp);
   gl->glUniform3fv(gld->light_location, 1, gld->light);
   gl->glUniform4fv(gld->color_location, 1, color);

   gl->glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);

   gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                             6 * sizeof(GLfloat), NULL);
   gl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                             6 * sizeof(GLfloat), (GLfloat *) 0 + 3);
   gl->glEnableVertexAttribArray(0);
   gl->glEnableVertexAttribArray(1);
   gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, gear->count);
}

static void
gears_draw(GLData *gld)
{

   Evas_GL_API *gl = gld->glapi;
   int i;

   GLfloat m[16];

   gl->glClearColor(0.5, 0.1, 0.1, 1.0);
   gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   memcpy(m, gld->proj, sizeof m);
   rotate(m, 2 * M_PI * gld->view_rotx / 360.0, 1, 0, 0);
   rotate(m, 2 * M_PI * gld->view_roty / 360.0, 0, 1, 0);
   rotate(m, 2 * M_PI * gld->view_rotz / 360.0, 0, 0, 1);

   for (i = 0; i < gear_count; i++)
      draw_gear(gld, gld->gears[i], m, gld->gears_x_position[i], gld->gears_y_position[i], gld->gears_z_position[i], gld->angle, gld->gears_color[i]);
}

//-------------------------//

static void render_gears(GLData *gld)
{
   gears_draw(gld);

   gld->angle += 2.0;
}

/* new window size or exposure */
static void
gears_reshape(GLData *gld, int width, int height)
{
   Evas_GL_API *gl = gld->glapi;

   GLfloat ar, m[16] = {
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 0.1, 0.0,
      0.0, 0.0, 0.0, 1.0,
   };

   if (width < height)
      ar = width;
   else
      ar = height;

   m[0] = 0.1 * ar / width;
   m[5] = 0.1 * ar / height;
   memcpy(gld->proj, m, sizeof gld->proj);
   gl->glViewport(0, 0, (GLint) width, (GLint) height);
}

static const char vertex_shader[] =
   "uniform mat4 proj;\n"
   "attribute vec4 position;\n"
   "attribute vec4 normal;\n"
   "varying vec3 rotated_normal;\n"
   "varying vec3 rotated_position;\n"
   "vec4 tmp;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = proj * position;\n"
   "   rotated_position = gl_Position.xyz;\n"
   "   tmp = proj * normal;\n"
   "   rotated_normal = tmp.xyz;\n"
   "}\n";

 static const char fragment_shader[] =
   "precision mediump float;\n"
   "uniform vec4 color;\n"
   "uniform vec3 light;\n"
   "varying vec3 rotated_normal;\n"
   "varying vec3 rotated_position;\n"
   "vec3 light_direction;\n"
   "vec4 white = vec4(0.5, 0.5, 0.5, 1.0);\n"
   "void main()\n"
   "{\n"
   "   light_direction = normalize(light - rotated_position);\n"
   "   gl_FragColor = color + white * dot(light_direction, rotated_normal);\n"
   "}\n";

static void
gears_init(GLData *gld)
{
   Evas_GL_API *gl = gld->glapi;
   int i;
   GLfloat inner_radius;
   GLfloat outer_radius;
   GLfloat width;
   GLint teeth;
   GLfloat tooth_depth;

   const char *p;
   char msg[512];

   gl->glEnable(GL_CULL_FACE);
   gl->glEnable(GL_DEPTH_TEST);

   p = vertex_shader;
   gld->vtx_shader = gl->glCreateShader(GL_VERTEX_SHADER);
   gl->glShaderSource(gld->vtx_shader, 1, &p, NULL);
   gl->glCompileShader(gld->vtx_shader);
   gl->glGetShaderInfoLog(gld->vtx_shader, sizeof msg, NULL, msg);

   p = fragment_shader;
   gld->fgmt_shader = gl->glCreateShader(GL_FRAGMENT_SHADER);
   gl->glShaderSource(gld->fgmt_shader, 1, &p, NULL);
   gl->glCompileShader(gld->fgmt_shader);
   gl->glGetShaderInfoLog(gld->fgmt_shader, sizeof msg, NULL, msg);

   gld->program = gl->glCreateProgram();
   gl->glAttachShader(gld->program, gld->vtx_shader);
   gl->glAttachShader(gld->program, gld->fgmt_shader);
   gl->glBindAttribLocation(gld->program, 0, "position");
   gl->glBindAttribLocation(gld->program, 1, "normal");

   gl->glLinkProgram(gld->program);
   gl->glGetProgramInfoLog(gld->program, sizeof msg, NULL, msg);

   gl->glUseProgram(gld->program);
   gld->proj_location  = gl->glGetUniformLocation(gld->program, "proj");
   gld->light_location = gl->glGetUniformLocation(gld->program, "light");
   gld->color_location = gl->glGetUniformLocation(gld->program, "color");

   /* make the gears */
   for (i = 0; i < 50; i++)
     {
        inner_radius = 1.0;
        outer_radius = 1.0;
        width = 0.5;
        teeth = 20;
        tooth_depth = 0.3;
        gld->gears_color[i][0] = 1.0;
        gld->gears_color[i][1] = 1.0;
        gld->gears_color[i][2] = 0;
        gld->gears_color[i][3] = 1.0;

        gld->gears_x_position[i] = (i * 3) / 10 - 6.0;
        gld->gears_y_position[i] = (i * 3) % 10 - 5.0;
        gld->gears_z_position[i] = 0;
        gld->gears[i] = make_gear(gld, inner_radius, inner_radius+outer_radius, width, teeth, tooth_depth);
        vertice_amount[view_count][i] = gld->gears[i]->count * 3;
     }

}

#if 0
static void
on_done(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_del((Evas_Object*) data);
}
#endif

static void del_first_glview(Evas_Object *obj)
{
   GLData *gld = evas_object_data_get(obj, "..gld");
   int i;

   view_count = 0;
   zoom_step = 4;
   gear_count = 1;

   if (!gld)
     {
        printf("Unable to get GLData. \n");
        return;
     }
   Evas_GL_API *gl = gld->glapi;

   gl->glDeleteShader(gld->vtx_shader);
   gl->glDeleteShader(gld->fgmt_shader);
   gl->glDeleteProgram(gld->program);

   for (i = 0; i < 50; i++)
     {
        gl->glDeleteBuffers(1, &(gld->gears[i]->vbo));
        free(gld->gears[i]->vertices);
        free(gld->gears[i]);
     }

   evas_object_data_del((Evas_Object*)obj, "..gld");
   free(gld);

   Ecore_Animator *ani = evas_object_data_get(obj, "..ani");
   ecore_animator_del(ani);
}

static void
gldata_init(GLData *gld)
{
   gld->initialized = 0;
   gld->mouse_down = 0;

   gld->view_rotx = -20.0;
   gld->view_roty = -30.0;
   gld->view_rotz = 0.0;
   gld->angle = 0.0;

   gld->light[0] = 1.0;
   gld->light[1] = 1.0;
   gld->light[2] = -5.0;
}

static void
set_glview(Evas_Object *glview)
{
   GLData *gld = NULL;
   Ecore_Animator *ani;

   if (!(gld = calloc(1, sizeof(GLData))))
      return;

   gldata_init(gld);

   evas_object_size_hint_align_set(glview, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(glview, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_glview_mode_set(glview, ELM_GLVIEW_ALPHA|ELM_GLVIEW_DEPTH);
   elm_glview_resize_policy_set(glview, ELM_GLVIEW_RESIZE_POLICY_RECREATE);
   elm_glview_render_policy_set(glview, ELM_GLVIEW_RENDER_POLICY_ON_DEMAND);
   elm_glview_init_func_set(glview, (Elm_GLView_Func_Cb)init_gl);

   if (view_count)
        elm_glview_del_func_set(glview, (Elm_GLView_Func_Cb)del_gl);
   else
        elm_glview_del_func_set(glview, (Elm_GLView_Func_Cb)del_first_glview);

   elm_glview_resize_func_set(glview, (Elm_GLView_Func_Cb)resize_gl);
   elm_glview_render_func_set(glview, (Elm_GLView_Func_Cb)draw_gl);

   elm_object_focus_set(glview, EINA_TRUE);
   evas_object_event_callback_add(glview, EVAS_CALLBACK_MOUSE_DOWN, mouse_down, glview);
   evas_object_event_callback_add(glview, EVAS_CALLBACK_MOUSE_UP, mouse_up, glview);
   evas_object_event_callback_add(glview, EVAS_CALLBACK_MOUSE_MOVE, mouse_move, glview);

   ani = ecore_animator_add(on_animate, glview);
   evas_object_data_set(glview, "..ani", ani);
   gld->glapi = elm_glview_gl_api_get(glview);
   evas_object_data_set(glview, "..gld", gld);

   evas_object_show(glview);

}

static void
add_btn_click(void *data, Evas_Object *obj, void *event_info)
{
   if (zoom_step == 4)
     {
        if (view_count != 3)
          {
             view_count++;
             glview[view_count] = elm_glview_add((Evas_Object *)data);
             set_glview(glview[view_count]);
             elm_box_pack_end(box, glview[view_count]);
             evas_object_show(glview[view_count]);
          }
     }
}

static void
delete_btn_click(void *data, Evas_Object *obj, void *event_info)
{
   if (zoom_step == 4)
     {
        if (view_count != 0)
          {
             elm_box_unpack(box, glview[view_count]);
             evas_object_del(glview[view_count]);
             view_count--;

          }
     }
}

static void
gear_add_btn_click(void *data, Evas_Object *obj, void *event_info)
{
    if (gear_count != 50)
      {
         gear_count++;
      }
}

static void
gear_delete_btn_click(void *data, Evas_Object *obj, void *event_info)
{
    if (gear_count != 1)
      {
         gear_count--;
      }
}


static void
expand_btn_click(void *data, Evas_Object *obj, void *event_info)
{
   int default_width = 100;
   int default_height = 130;

   if (!view_count )
     {
        if ( zoom_step != 4)
          {
             zoom_step++;
             if(zoom_step != 4)
               {
                  evas_object_size_hint_align_set(glview[0], 0,0);
                  evas_object_size_hint_min_set(glview[0], default_width*zoom_step, default_height*zoom_step);
               }
             else
               {
                  evas_object_size_hint_min_set(glview[0], 0, 0);
                  evas_object_size_hint_align_set(glview[0], EVAS_HINT_FILL, EVAS_HINT_FILL);
                  evas_object_size_hint_weight_set(glview[0], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
               }
          }
     }
}

static void
contract_btn_click(void *data, Evas_Object *obj, void *event_info)
{
   int default_width = 100;
   int default_height = 130;

   if (!view_count )
     {
        if ( zoom_step != 1)
          {
             zoom_step--;
             evas_object_size_hint_align_set(glview[0], 0,0);
             evas_object_size_hint_min_set(glview[0], default_width * zoom_step, default_height * zoom_step);

          }
     }
}



void
elmgears_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win;
   Evas_Object *amount_btn_box;
   Evas_Object *gear_btn_box;
   Evas_Object *gear_add_btn, *gear_delete_btn;
   Evas_Object *add_btn, *delete_btn;
   Evas_Object *expand_btn, *contract_btn;
   Evas_Object *scale_btn_box;

   struct appdata *ad = (struct appdata *)data;
   if (ad == NULL) return;

   win = ad->nf;

   //modified
   box = elm_box_add(win);
   evas_object_move(box, 0, 10);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_min_set(box, 480, 600);
   evas_object_show(box);

   /* add an amount_btn_box*/
   amount_btn_box = elm_box_add(win);
   elm_box_horizontal_set(amount_btn_box, EINA_TRUE);
   evas_object_size_hint_weight_set(amount_btn_box, 0.0, 0.0);
   elm_box_pack_end(box, amount_btn_box);
   evas_object_show(amount_btn_box);

   /* add an ok button */
   add_btn = elm_button_add(win);
   elm_object_text_set(add_btn, "GLView Add");
   evas_object_size_hint_weight_set(add_btn, 0.0, 0.0);
   evas_object_size_hint_min_set(add_btn, 350, 80);
   elm_box_pack_end(amount_btn_box, add_btn);
   evas_object_show(add_btn);
   evas_object_smart_callback_add(add_btn, "clicked", add_btn_click, ad->win_main);

   /* add a delete button */
   delete_btn = elm_button_add(win);
   elm_object_text_set(delete_btn, "GLView Delete");
   evas_object_size_hint_weight_set(delete_btn, 0.0, 0.0);
   evas_object_size_hint_min_set(delete_btn, 350, 80);
   elm_box_pack_end(amount_btn_box, delete_btn);
   evas_object_show(delete_btn);
   evas_object_smart_callback_add(delete_btn, "clicked", delete_btn_click, ad->win_main);

   /* add a scale_btn_box*/
   scale_btn_box = elm_box_add(win);
   elm_box_horizontal_set(scale_btn_box, EINA_TRUE);
   evas_object_size_hint_weight_set(scale_btn_box, 0.0, 0.0);
   elm_box_pack_end(box, scale_btn_box);
   evas_object_show(scale_btn_box);

   /* add a contract button */
   contract_btn = elm_button_add(win);
   elm_object_text_set(contract_btn, "GLView Contract");
   evas_object_size_hint_weight_set(contract_btn, 0.0, 0.0);
   evas_object_size_hint_min_set(contract_btn, 350, 80);
   elm_box_pack_end(scale_btn_box, contract_btn);
   evas_object_show(contract_btn);
   evas_object_smart_callback_add(contract_btn, "clicked", contract_btn_click, NULL);

   /* add an expand button */
   expand_btn = elm_button_add(win);
   elm_object_text_set(expand_btn, "GLView Expand");
   evas_object_size_hint_weight_set(expand_btn, 0.0, 0.0);
   evas_object_size_hint_min_set(expand_btn, 350, 80);
   elm_box_pack_end(scale_btn_box, expand_btn);
   evas_object_show(expand_btn);
   evas_object_smart_callback_add(expand_btn, "clicked", expand_btn_click, NULL);

   /* add an amount_btn_box*/
   gear_btn_box = elm_box_add(win);
   elm_box_horizontal_set(gear_btn_box, EINA_TRUE);
   evas_object_size_hint_weight_set(gear_btn_box, 0.0, 0.0);
   elm_box_pack_end(box, gear_btn_box);
   evas_object_show(gear_btn_box);

   /* add an ok button */
   gear_add_btn = elm_button_add(win);
   elm_object_text_set(gear_add_btn, "Gear Add");
   evas_object_size_hint_weight_set(gear_add_btn, 0.0, 0.0);
   evas_object_size_hint_min_set(gear_add_btn, 350, 80);
   elm_box_pack_end(gear_btn_box, gear_add_btn);
   evas_object_show(gear_add_btn);
   evas_object_smart_callback_add(gear_add_btn, "clicked", gear_add_btn_click, NULL);

   /* add a delete button */
   gear_delete_btn = elm_button_add(win);
   elm_object_text_set(gear_delete_btn, "Gear Delete");
   evas_object_size_hint_weight_set(gear_delete_btn, 0.0, 0.0);
   evas_object_size_hint_min_set(gear_delete_btn, 350, 80);
   elm_box_pack_end(gear_btn_box, gear_delete_btn);
   evas_object_show(gear_delete_btn);
   evas_object_smart_callback_add(gear_delete_btn, "clicked", gear_delete_btn_click, NULL);

   /* add an fps label*/
   fps_label = elm_label_add(win);
   elm_object_text_set(fps_label, "fps");
   evas_object_size_hint_weight_set(fps_label, 0.0, 0.0);
   evas_object_size_hint_min_set(fps_label, 480, 80);
   elm_box_pack_end(box, fps_label);
   evas_object_show(fps_label);

   glview[0] = elm_glview_add(ad->win_main);
   set_glview(glview[0]);
   elm_box_pack_end(box, glview[0]);

   elm_naviframe_item_push(ad->nf, _("Gears"), NULL, NULL, box, NULL);

   evas_object_show(win);
}

static Eina_Bool on_animate(void *data)
{
   elm_glview_changed_set((Evas_Object*)data);

   PRINT_FPS(1);

   return EINA_TRUE;
}

static void init_gl(Evas_Object *obj)
{
   GLData *gld = evas_object_data_get(obj, "..gld");
   if (!gld) return;

   //printf("Init Function Called...\n");

   gears_init(gld);

   //resize_gl(obj);
}

static void del_gl(Evas_Object *obj)
{
   GLData *gld = evas_object_data_get(obj, "..gld");
   int i;

   if (!gld)
     {
        printf("Unable to get GLData. \n");
        return;
     }
   Evas_GL_API *gl = gld->glapi;

   //printf("Delete Function Called...\n");

   gl->glDeleteShader(gld->vtx_shader);
   gl->glDeleteShader(gld->fgmt_shader);
   gl->glDeleteProgram(gld->program);

   for (i = 0; i < 50; i++)
     {
        gl->glDeleteBuffers(1, &(gld->gears[i]->vbo));
        free(gld->gears[i]->vertices);
        free(gld->gears[i]);
     }

   evas_object_data_del((Evas_Object*)obj, "..gld");
   free(gld);

   Ecore_Animator *ani = evas_object_data_get(obj, "..ani");
   ecore_animator_del(ani);

}

static void resize_gl(Evas_Object *obj)
{
   GLData *gld = evas_object_data_get(obj, "..gld");
   int w, h;

   if (!gld) return;

   // get the image size in case it changed with evas_object_image_size_set()
   elm_glview_size_get(obj, &w, &h);

   // GL Viewport stuff. you can avoid doing this if viewport is all the
   // same as last frame if you want
   gears_reshape(gld, w, h);
}


static void draw_gl(Evas_Object *obj)
{
   GLData *gld = evas_object_data_get(obj, "..gld");
   if (!gld) return;
   Evas_GL_API *gl = gld->glapi;

   render_gears(gld);
   gl->glFinish();
}

#if 0
void key_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Key_Down *ev;
   ev = (Evas_Event_Key_Down *)event_info;
   GLData *gld = evas_object_data_get(obj, "..gld");

   if (strcmp(ev->keyname, "Left") == 0)
     {
        gld->view_roty += 5.0;
        return;
     }

   if (strcmp(ev->keyname, "Right") == 0)
     {
        gld->view_roty -= 5.0;
        return;
     }

   if (strcmp(ev->keyname, "Up") == 0)
     {
        gld->view_rotx += 5.0;
        return;
     }

   if (strcmp(ev->keyname, "Down") == 0)
     {
        gld->view_rotx -= 5.0;
        return;
     }
   if ((strcmp(ev->keyname, "Escape") == 0) ||
       (strcmp(ev->keyname, "Return") == 0))
     {
        on_done(data, obj, event_info);
        return;
     }
}
#endif

void mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   GLData *gld = evas_object_data_get(obj, "..gld");
   gld->mouse_down = 1;
}

void mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev;
   ev = (Evas_Event_Mouse_Move *)event_info;
   GLData *gld = evas_object_data_get(obj, "..gld");
   float dx = 0, dy = 0;

   if (gld->mouse_down)
     {
        dx = ev->cur.canvas.x - ev->prev.canvas.x;
        dy = ev->cur.canvas.y - ev->prev.canvas.y;

        gld->view_roty += -1.0 * dx;
        gld->view_rotx += -1.0 * dy;
     }

}

void mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   GLData *gld = evas_object_data_get(obj, "..gld");
   gld->mouse_down = 0;
}

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
