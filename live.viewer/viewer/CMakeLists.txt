CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(w-add-viewer C)

SET(SRCS
	src/main.c
	src/dynamicbox_evas.c
)


INCLUDE(FindPkgConfig)
pkg_check_modules(viewer_pkgs REQUIRED
	capi-appfw-application
	appcore-efl
	ecore-x
	ecore
	evas
	edje
	vconf
	bundle
        dlog
	dynamicbox_viewer
	dynamicbox_service
	ail
	utilX
)

FOREACH(flag ${viewer_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_C_FLAGS_RELEASE "-O2")

ADD_DEFINITIONS("-DDYNAMICBOX_EVAS_RESOURCE_EDJ=\"${EDJDIR}/w_add_viewer.edj\"")

ADD_DEFINITIONS("-DDYNAMICBOX_EVAS_RESOURCE_PD=\"gbar\"")
ADD_DEFINITIONS("-DDYNAMICBOX_EVAS_RESOURCE_LB=\"dynamicbox\"")
ADD_DEFINITIONS("-DDYNAMICBOX_EVAS_RESOURCE_IMG=\"dynamicbox,image\"")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${viewer_pkgs_LDFLAGS} "-lm -fpie -fPIC")

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${BINDIR})

# data
ADD_SUBDIRECTORY(data)
ADD_SUBDIRECTORY(po)
