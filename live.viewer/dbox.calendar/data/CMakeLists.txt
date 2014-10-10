CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# install edj
ADD_CUSTOM_TARGET(w_calendar_icon.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/images ${EDJE_CFLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/w_calendar_icon.edc w_calendar_icon.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/w_calendar_icon.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} w_calendar_icon.edj)
INSTALL(FILES w_calendar_icon.edj DESTINATION ${EDJDIR})
