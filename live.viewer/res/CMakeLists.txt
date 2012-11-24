ADD_CUSTOM_TARGET(live-viewer.edj ALL
		COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/res/image
		${CMAKE_CURRENT_SOURCE_DIR}/live-viewer.edc live-viewer.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/live-viewer.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} live-viewer.edj)
INSTALL(FILES live-viewer.edj DESTINATION ${PKGROOT}/res/edje)
