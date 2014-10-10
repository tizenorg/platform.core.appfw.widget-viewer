CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(provider.dbox.app-dbox C)

SET(SRCS
	src/main.c
)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

INCLUDE(FindPkgConfig)
pkg_check_modules(dbox_pkgs REQUIRED
	capi-appfw-application
	elementary
	dlog
	dynamicbox_service
	dynamicbox_provider
	dynamicbox_provider_app
	dynamicbox
	vconf
)

FOREACH(flag ${dbox_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
	SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -Wall")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXXFLAGS} -Wall")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -Wall")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -Wall")

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/include")

ADD_DEFINITIONS("-DLOG_TAG=\"DBOX_APP_ONLY\"")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${dbox_pkgs_LDFLAGS} "-lm")

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${BINDIR})

#install application HOME directory
ADD_SUBDIRECTORY(data)
