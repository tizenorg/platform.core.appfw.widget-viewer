#include <Elementary.h>
#include <stdlib.h>

const char *enemies[] = { "sistar", "girlsday", "fx", "2ne1", "kara", "missa", "apink", "crayonpop", "4minute", "tara" };

struct instance {
    Eina_List *enemy_list;
    Evas_Object *bricks[9];
    Evas_Object *score;
    int score_val;
};

Evas_Object *create_enemy(Evas *e)
{
   int idx = rand() % 10;

   Evas_Object *enemy = evas_object_text_add(e);
   evas_object_text_font_set(enemy, "DejaVu", 11);
   evas_object_text_text_set(enemy, enemies[idx]);
   evas_object_color_set(enemy, 0, 0, 0, 255);
   evas_object_resize(enemy, 50, 30);
   evas_object_move(enemy, idx * 50, 0);
   evas_object_show(enemy);

   return enemy;
}

Eina_Bool create_enemy_timer_cb(void *data)
{
   Evas *e;
   struct instance *inst;

   e = evas_object_evas_get(data);
   inst = evas_object_data_get(data, "instance");

   Evas_Object *enemy = create_enemy(e);

   inst->enemy_list = eina_list_append(inst->enemy_list, enemy);

   return ECORE_CALLBACK_RENEW;
}

void remove_brick(struct instance *inst)
{
   int i;

   for (i = 0; i < 9; i++)
   {
      if (!evas_object_visible_get(inst->bricks[i])) continue;
      evas_object_hide(inst->bricks[i]);
      return;
   }

   elm_exit();
}

Eina_Bool move_enemy_timer_cb(void *data)
{
   Eina_List *l, *l_next;
   Evas_Object *enemy;
   Evas_Coord x, y;
   struct instance *inst;

   inst = evas_object_data_get(data, "instance");

   EINA_LIST_FOREACH_SAFE(inst->enemy_list, l, l_next, enemy)
   {
      evas_object_geometry_get(enemy, &x, &y, NULL, NULL);

      if (y >= 400)
      {
         inst->enemy_list = eina_list_remove(inst->enemy_list, enemy);
         evas_object_del(enemy);
         remove_brick(inst);
      }
      else
      {
         evas_object_move(enemy, x, y + 20);
      }
   }

   return ECORE_CALLBACK_RENEW;
}

Evas_Object *create_brick(Evas *e, Evas_Coord x, Evas_Coord y)
{
   Evas_Object *img = evas_object_image_filled_add(e);
   evas_object_image_file_set(img, RESDIR"/brick.png", NULL);
   evas_object_move(img, x, y);
   evas_object_resize(img, 25, 15);
   evas_object_show(img);

   return img;
}

void attack(Evas_Object *win, const char *input)
{
   Eina_List *l, *l_next;
   Evas_Object *enemy;
   struct instance *inst;

   inst = evas_object_data_get(win, "instance");

   EINA_LIST_FOREACH_SAFE(inst->enemy_list, l, l_next, enemy)
   {
      if (!strcmp(evas_object_text_text_get(enemy), input))
      {
         inst->enemy_list = eina_list_remove(inst->enemy_list, enemy);
         evas_object_del(enemy);
         inst->score_val += 100;
      }
   }

/*
   char buf[100];
   snprintf(buf, sizeof(buf), "Score: %d", inst->score_val);
   evas_object_text_text_set(inst->score, buf);
*/
}

void set_score(Evas_Object *win, const char *source, const char *emission)
{
    char *buf;
    struct instance *inst;

    inst = evas_object_data_get(win, "instance");

    buf = malloc(strlen(source) + strlen(emission) + 100);
    snprintf(buf, strlen(source) + strlen(emission) + 100, "%s/%s", source, emission);
    evas_object_text_text_set(inst->score, buf);
}

void key_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   const int MAX_CUR = 10;

   static char buf[50];
   static int cur = 0;

   Evas_Event_Key_Down *ev = event_info;
   Evas_Object *input = data;
   char tmp[50];

   if (cur == 0) snprintf(buf, sizeof(buf), " ");
//   printf("%s %s %s %s (%d)\n", ev->keyname, ev->key, ev->string, ev->compose, ev->keycode);
   if (!strcmp(ev->keyname, "space") || !strcmp(ev->keyname, "Return"))
   {
      attack(obj, buf);
      evas_object_text_text_set(input, "");
      cur = 0;
      return;
   }

   if (!strcmp(ev->keyname, "BackSpace"))
   {
      snprintf(tmp, strlen(buf), "%s", buf);
      evas_object_text_text_set(input, tmp);
      strcpy(buf, tmp);
      cur--;
      return;
   }

   if (cur >= MAX_CUR) return;

   snprintf(tmp, sizeof(tmp), "%s%s", buf, ev->keyname);
   evas_object_text_text_set(input, tmp);
   cur++;
   strcpy(buf, tmp);
}

int htt_main(Evas_Object *win)
{
   struct instance *inst;

   inst = calloc(1, sizeof(*inst));
   if (!inst) {
      return -ENOMEM;
   }

   srand((unsigned int) time(NULL));

   Evas *e = evas_object_evas_get(win);

   Evas_Object *bg = evas_object_rectangle_add(e);
   evas_object_color_set(bg, 168, 168, 168, 255);
   evas_object_resize(bg, 500, 400);
   evas_object_show(bg);

   Evas_Object *ocean = evas_object_image_add(e);
   evas_object_image_file_set(ocean, RESDIR"/ocean.png", NULL);
   evas_object_image_fill_set(ocean, 0, 0, 20, 20);
   evas_object_move(ocean, 0, 380);
   evas_object_resize(ocean, 500, 20);
   evas_object_show(ocean);

   inst->bricks[0] = create_brick(e, 225, 355);
   inst->bricks[1] = create_brick(e, 250, 355);
   inst->bricks[2] = create_brick(e, 275, 355);
   inst->bricks[3] = create_brick(e, 225, 370);
   inst->bricks[4] = create_brick(e, 250, 370);
   inst->bricks[5] = create_brick(e, 275, 370);
   inst->bricks[6] = create_brick(e, 225, 385);
   inst->bricks[7] = create_brick(e, 250, 385);
   inst->bricks[8] = create_brick(e, 275, 385);

   Evas_Object *entry = evas_object_image_filled_add(e);
   evas_object_image_file_set(entry, RESDIR"/entry.png", NULL);
   evas_object_move(entry, 217, 326);
   evas_object_resize(entry, 90, 30);
   evas_object_show(entry);

   inst->score = evas_object_text_add(e);
   evas_object_text_font_set(inst->score, "DejaVu", 12);
   evas_object_text_text_set(inst->score, "Score: 0");
   evas_object_color_set(inst->score, 0, 0, 0, 255);
   evas_object_resize(inst->score, 100, 30);
   evas_object_show(inst->score);

   Evas_Object *input = evas_object_text_add(e);
   evas_object_text_font_set(input, "DejaVu", 12);
   evas_object_color_set(input, 0, 0, 0, 255);
   evas_object_move(input, 220, 330);
   evas_object_resize(input, 100, 30);
   evas_object_show(input);

   ecore_timer_add(2, create_enemy_timer_cb, win);
   ecore_timer_add(0.8, move_enemy_timer_cb, win);

   evas_object_data_set(win, "instance", inst);

   evas_object_event_callback_add(win, EVAS_CALLBACK_KEY_DOWN, key_down_cb, input);
   return 0;
}
