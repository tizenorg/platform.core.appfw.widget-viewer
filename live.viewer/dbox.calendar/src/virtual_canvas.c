#include <Elementary.h>
#include <Ecore_Evas.h>
#include <Ecore_X.h>

#include <dlog.h>
#include <dynamicbox.h>

#include "virtual_canvas.h"
#include "main.h"



#define QUALITY_N_COMPRESS "quality=100 compress=1"



Evas *create_virtual_canvas(int w, int h)
{
        Ecore_Evas *internal_ee;
        Evas *internal_e;

        // Create virtual canvas
        internal_ee = ecore_evas_buffer_new(w, h);
        if (!internal_ee) {
                LOGD("Failed to create a new canvas buffer\n");
                return NULL;
        }

	ecore_evas_alpha_set(internal_ee, EINA_TRUE);
	ecore_evas_manual_render_set(internal_ee, EINA_TRUE);

        // Get the "Evas" object from a virtual canvas
        internal_e = ecore_evas_get(internal_ee);
        if (!internal_e) {
                ecore_evas_free(internal_ee);
                LOGD("Faield to get Evas object\n");
                return NULL;
        }

        return internal_e;
}



int flush_to_file(Evas *e, const char *filename, int w, int h)
{
        void *data;
        Ecore_Evas *internal_ee;

        internal_ee = ecore_evas_ecore_evas_get(e);
        if (!internal_ee) {
		LOGD("Failed to get ecore evas\n");
                return EXIT_FAILURE;
        }

	ecore_evas_manual_render(internal_ee);

        // Get a pointer of a buffer of the virtual canvas
        data = (void*)ecore_evas_buffer_pixels_get(internal_ee);
        if (!data) {
		LOGD("Failed to get pixel data\n");
                return EXIT_FAILURE;
        }

	return flush_data_to_file(e, data, filename, w, h);
}



int flush_data_to_file(Evas *e, char *data, const char *filename, int w, int h)
{
        Evas_Object *output;

        output = evas_object_image_add(e);
        if (!output) {
		LOGD("Failed to create an image object\n");
                return EXIT_FAILURE;
        }

        evas_object_image_data_set(output, NULL);
        evas_object_image_colorspace_set(output, EVAS_COLORSPACE_ARGB8888);
        evas_object_image_alpha_set(output, EINA_TRUE);
        evas_object_image_size_set(output, w, h);
        evas_object_image_smooth_scale_set(output, EINA_TRUE);
        evas_object_image_data_set(output, data);
	evas_object_image_fill_set(output, 0, 0, w, h);
        evas_object_image_data_update_add(output, 0, 0, w, h);

        if (evas_object_image_save(output, filename, NULL, QUALITY_N_COMPRESS) == EINA_FALSE) {
                evas_object_del(output);
		LOGD("Faield to save a captured image (%s)\n", filename);
                return EXIT_FAILURE;
        }

	evas_object_del(output);

        if (access(filename, F_OK) != 0) {
		LOGD("File %s is not found\n", filename);
                return EXIT_FAILURE;
        }

	return EXIT_SUCCESS;
}



int destroy_virtual_canvas(Evas *e)
{
        Ecore_Evas *ee;

        ee = ecore_evas_ecore_evas_get(e);
        if (!ee) {
		LOGD("Failed to ecore evas object\n");
                return EXIT_FAILURE;
        }

        ecore_evas_free(ee);
        return EXIT_SUCCESS;
}



// End of a file
