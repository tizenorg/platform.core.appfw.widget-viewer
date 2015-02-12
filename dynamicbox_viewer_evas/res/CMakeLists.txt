CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# install edj
ADD_CUSTOM_TARGET(dynamicbox_viewer_evas.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/images ${EDJE_CFLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/dynamicbox_viewer_evas.edc dynamicbox_viewer_evas.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/dynamicbox_viewer_evas.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} dynamicbox_viewer_evas.edj)
INSTALL(FILES dynamicbox_viewer_evas.edj DESTINATION ${EDJDIR})
INSTALL(FILES unknown.png DESTINATION ${IMGDIR})
