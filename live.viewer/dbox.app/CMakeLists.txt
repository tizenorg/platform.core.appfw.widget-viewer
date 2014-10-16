CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(provider.dbox.app C)

SET(SRCS
	src/main.c
	src/instance.c
#	src/entry.c
	src/util.c
	src/window.c
	src/htt.c
)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

INCLUDE(FindPkgConfig)
pkg_check_modules(app_pkgs REQUIRED
	capi-appfw-application
	vconf
	elementary
	ecore-x
	dlog
	dynamicbox_service
	dynamicbox_provider_app
	dynamicbox_provider
	dynamicbox
)

FOREACH(flag ${app_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
	SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -Wall")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXXFLAGS} -Wall")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -Wall")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -Wall")

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" STREQUAL "arm")
	ADD_DEFINITIONS("-DTARGET")
	MESSAGE("add -DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

ADD_DEFINITIONS("-DLOG_TAG=\"DBOX_APP\"")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/include")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${app_pkgs_LDFLAGS} "-lm")

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${BINDIR})

#install application HOME directory
INSTALL(DIRECTORY DESTINATION ${DATADIR})
#ADD_SUBDIRECTORY(data)
ADD_SUBDIRECTORY(res)
