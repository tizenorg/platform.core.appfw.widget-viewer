CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# install edj
ADD_CUSTOM_TARGET(w_add_viewer.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/images ${EDJE_CFLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/w_add_viewer.edc w_add_viewer.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/w_add_viewer.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} w_add_viewer.edj)
INSTALL(FILES w_add_viewer.edj DESTINATION ${EDJDIR})
INSTALL(FILES images/unknown.png DESTINATION ${RESDIR}/images)
