CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_SUBDIRECTORY(widget_viewer)
ADD_SUBDIRECTORY(widget_viewer_evas)

ADD_DEPENDENCIES(widget_viewer_evas widget_viewer)
