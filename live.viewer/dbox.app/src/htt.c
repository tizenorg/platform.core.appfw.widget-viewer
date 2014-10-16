#include <Elementary.h>
#include <stdlib.h>

const char *enemies[] = { "sistar", "girlsday", "fx", "2ne1", "kara", "missa", "apink", "crayonpop", "4minute", "tara" };
static Eina_List *enemy_list = NULL;
static Evas_Object *bricks[9];
static Evas_Object *score;
static int score_val = 0;

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
   Evas *e = data;

   Evas_Object *enemy = create_enemy(e);

   enemy_list = eina_list_append(enemy_list, enemy);

   return ECORE_CALLBACK_RENEW;
}

void remove_brick()
{
   int i;

   for (i = 0; i < 9; i++)
   {
      if (!evas_object_visible_get(bricks[i])) continue;
      evas_object_hide(bricks[i]);
      return;
   }

   elm_exit();
}

Eina_Bool move_enemy_timer_cb(void *data)
{
   Eina_List *l, *l_next;
   Evas_Object *enemy;
   Evas_Coord x, y;

   EINA_LIST_FOREACH_SAFE(enemy_list, l, l_next, enemy)
   {
      evas_object_geometry_get(enemy, &x, &y, NULL, NULL);

      if (y >= 400)
      {
         enemy_list = eina_list_remove(enemy_list, enemy);
         evas_object_del(enemy);
         remove_brick();
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

void attack(const char *input)
{
   Eina_List *l, *l_next;
   Evas_Object *enemy;

   EINA_LIST_FOREACH_SAFE(enemy_list, l, l_next, enemy)
   {
      if (!strcmp(evas_object_text_text_get(enemy), input))
      {
         enemy_list = eina_list_remove(enemy_list, enemy);
         evas_object_del(enemy);
         score_val += 100;
      }
   }

   char buf[100];
   snprintf(buf, sizeof(buf), "Score: %d", score_val);
   evas_object_text_text_set(score, buf);
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
      attack(buf);
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

   bricks[0] = create_brick(e, 225, 355);
   bricks[1] = create_brick(e, 250, 355);
   bricks[2] = create_brick(e, 275, 355);
   bricks[3] = create_brick(e, 225, 370);
   bricks[4] = create_brick(e, 250, 370);
   bricks[5] = create_brick(e, 275, 370);
   bricks[6] = create_brick(e, 225, 385);
   bricks[7] = create_brick(e, 250, 385);
   bricks[8] = create_brick(e, 275, 385);

   Evas_Object *entry = evas_object_image_filled_add(e);
   evas_object_image_file_set(entry, RESDIR"/entry.png", NULL);
   evas_object_move(entry, 217, 326);
   evas_object_resize(entry, 90, 30);
   evas_object_show(entry);

   score = evas_object_text_add(e);
   evas_object_text_font_set(score, "DejaVu", 12);
   evas_object_text_text_set(score, "Score: 0");
   evas_object_color_set(score, 0, 0, 0, 255);
   evas_object_resize(score, 100, 30);
   evas_object_show(score);

   Evas_Object *input = evas_object_text_add(e);
   evas_object_text_font_set(input, "DejaVu", 12);
   evas_object_color_set(input, 0, 0, 0, 255);
   evas_object_move(input, 220, 330);
   evas_object_resize(input, 100, 30);
   evas_object_show(input);

   ecore_timer_add(2, create_enemy_timer_cb, e);
   ecore_timer_add(0.8, move_enemy_timer_cb, e);

   evas_object_event_callback_add(win, EVAS_CALLBACK_KEY_DOWN, key_down_cb, input);
   return 0;
}
