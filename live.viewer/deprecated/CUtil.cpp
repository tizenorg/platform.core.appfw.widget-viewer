#include <Elementary.h>
#include <libgen.h>

#include <ail.h>
#include <dlog.h>

#include "dlist.h"
#include "CUtil.h"

CUtil::CUtil(void)
{
}

CUtil::~CUtil(void)
{
}

char *CUtil::GetIconFile(const char *pkgname)
{
	ail_appinfo_h handle;
	ail_error_e ret;
	char *iconfile;
	char *ret_iconfile;

	ret = ail_package_get_appinfo(pkgname, &handle);
	if (ret != AIL_ERROR_OK) {
		ErrPrint("ail get pkgname = %s\n", pkgname);
		return NULL;
	}

	ret = ail_appinfo_get_str(handle, AIL_PROP_ICON_STR, &iconfile);
	if (ret != AIL_ERROR_OK) {
		ret = ail_package_destroy_appinfo(handle);
		ErrPrint("Get iconfile from pkgname = %s\n", pkgname);
		return NULL;
	}

	ret_iconfile = strdup(iconfile);
	if (!ret_iconfile)
		ErrPrint("Error: %s\n", strerror(errno));

	ret = ail_package_destroy_appinfo(handle);
	if (ret != AIL_ERROR_OK) {
		if (ret_iconfile)
			free(ret_iconfile);
		ErrPrint("Failed to destory appinfo\n");
		return NULL;
	}

	return ret_iconfile;
}

Evas_Object *CUtil::CreateCanvasImage(Evas_Object *parent, int w, int h)
{
	Evas_Object *pCanvasImage;
	pCanvasImage = evas_object_image_filled_add(evas_object_evas_get(parent));
	if (!pCanvasImage)
		return NULL;

	evas_object_image_colorspace_set(pCanvasImage, EVAS_COLORSPACE_ARGB8888);
	evas_object_image_alpha_set(pCanvasImage, EINA_TRUE);
	evas_object_move(pCanvasImage, 0, 0);

	return pCanvasImage;
}

int CUtil::UpdateCanvasImage(Evas_Object *image, void *data, int w, int h)
{
	evas_object_image_size_set(image, w, h);
	evas_object_image_fill_set(image, 0, 0, w, h);
	evas_object_image_data_copy_set(image, data);
	evas_object_image_data_update_add(image, 0, 0, w, h);

	evas_object_resize(image, w, h);
	return 0;
}

int CUtil::UpdateCanvasImage(Evas_Object *image, const char *filename, int w, int h)
{
	const char *pOldFilename;

	evas_object_image_file_get(image, &pOldFilename, NULL);
	if (pOldFilename && !strcmp(filename, pOldFilename))
		evas_object_image_reload(image);
	else
		evas_object_image_file_set(image, filename, NULL);

	evas_object_resize(image, w, h);
	return 0;
}

/*!< End of a file */
