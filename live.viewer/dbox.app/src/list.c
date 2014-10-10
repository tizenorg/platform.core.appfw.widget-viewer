#include "elmdemo_test.h"
#include "elmdemo_util.h"
#include "genlist.h"

extern char *genlist_demo_names[];

void list_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *list;
	int i;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	list = elm_list_add(ad->nf);
	elm_list_mode_set(list, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(list, "selected", NULL, NULL);
	for (i = 0; i < 100 ; i++) {
		Evas_Object *icon = elm_check_add(obj);
		elm_object_style_set(icon, "default/genlist");
		evas_object_repeat_events_set(icon, EINA_TRUE);
		evas_object_propagate_events_set(icon, EINA_FALSE);

		Evas_Object *end = elm_button_add(obj);
		elm_object_text_set(end, "Resend");
		evas_object_propagate_events_set(end, EINA_FALSE);

		elm_list_item_append(list, genlist_demo_names[i%NUM_OF_GENLIST_DEMO_NAMES], icon, end, NULL, ad);
	}
	elm_list_go(list);

	elm_naviframe_item_push(ad->nf, _("List"), NULL, NULL, list, NULL);
}
