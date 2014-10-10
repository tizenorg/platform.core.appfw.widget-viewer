CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(com.samsung.w-add-viewer.dbox.calendar C)

INCLUDE(FindPkgConfig)
pkg_check_modules(com.samsung.calendar.icon REQUIRED
	elementary
	dlog
	dynamicbox
	dynamicbox_service
	aul
)

SET(EXTRA_CFLAGS "")
FOREACH (flag ${com.samsung.calendar.icon_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "-DTIZEN_DEBUG_ENABLE -Wall -Winline -Werror -fno-builtin-malloc -fno-omit-frame-pointer -g")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

ADD_DEFINITIONS(${EXTRA_CFLAGS})
ADD_DEFINITIONS("-DLOG_TAG=\"DBOX_CALENDAR\"")

ADD_LIBRARY(${PROJECT_NAME} SHARED
	src/main.c
	src/virtual_canvas.c
)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${com.samsung.calendar.icon_LDFLAGS})

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${LIBEXECDIR})

ADD_SUBDIRECTORY(data)
#ADD_SUBDIRECTORY(res)
