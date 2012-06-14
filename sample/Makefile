all:
	@gcc sample.c -o sample -Wall -p -g `pkg-config ail elementary dlog ecore-x appcore-efl glib-2.0 gio-2.0 livebox-viewer shortcut --libs --cflags`
	@gcc edje.c -o edje -DLOG_TAG="\"edje\"" -Wall -p -g `pkg-config appcore-efl elementary ecore-x dlog --libs --cflags`
