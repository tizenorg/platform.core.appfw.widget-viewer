CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_SUBDIRECTORY(dynamicbox_viewer)
ADD_SUBDIRECTORY(livebox-viewer)

ADD_DEPENDENCIES(livebox-viewer dynamicbox_viewer)
