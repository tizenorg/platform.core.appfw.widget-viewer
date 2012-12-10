PROJECT(live-viewer C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	capi-appfw-application
	capi-appfw-app-manager
	ail
	bundle
	dlog
	elementary
	ecore-x
	appcore-efl
	livebox-viewer
	livebox-service
)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -Wall -Werror -Winline -g")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

SET(PKGROOT "/opt/usr/apps/live.viewer")
ADD_DEFINITIONS("-DNDEBUG")
ADD_DEFINITIONS("-DPKGROOT=\"${PKGROOT}\"")
#ADD_DEFINITIONS("-DFLOG")
ADD_DEFINITIONS("-DLOG_TAG=\"${PROJECT_NAME}\"")
ADD_DEFINITIONS(${pkgs_CFLAGS})
ADD_DEFINITIONS(${pkgs_LDFLAGS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

ADD_EXECUTABLE(${PROJECT_NAME}
	src/main.c
	src/dlist.c
	src/live_scroller.c
	src/util.c
	src/scroller.c
	src/lb.c
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${PKGROOT}/bin)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/live.viewer.xml DESTINATION /opt/share/packages)

ADD_SUBDIRECTORY(res)
