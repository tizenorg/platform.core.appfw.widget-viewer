CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(com.samsung.w-add-viewer.dbox.text C)

INCLUDE(FindPkgConfig)
pkg_check_modules(com.samsung.w-add-viewer.text REQUIRED
	elementary
	dlog
	dynamicbox
	dynamicbox_service
	aul
)

FOREACH (flag ${com.samsung.w-add-viewer.text_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Winline -Werror -fno-builtin-malloc -fno-omit-frame-pointer -g")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

ADD_DEFINITIONS(${EXTRA_CFLAGS})
ADD_DEFINITIONS("-DLOG_TAG=\"DBOX_TEXT\"")

ADD_LIBRARY(${PROJECT_NAME} SHARED
	src/main.c
)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${com.samsung.w-add-viewer.text_LDFLAGS})

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${LIBEXECDIR})

#ADD_SUBDIRECTORY(data)
#ADD_SUBDIRECTORY(res)
