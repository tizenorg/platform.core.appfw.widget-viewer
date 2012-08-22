PROJECT(live-viewer CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "/${prefix}/bin")
SET(PROJECT_NAME "${PROJECT_NAME}")
SET(VERSION_MAJOR 0)
SET(VERSION "${VERSION_MAJOR}.0.1")

set(CMAKE_SKIP_BUILD_RPATH true)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/live-viewer/include)

INCLUDE(FindPkgConfig)
pkg_check_modules(livepkgs REQUIRED
	dlog
	ail
	elementary
	eocre-x
	appcore-efl
	glib-2.0
	gio-2.0
	livebox-viewer
)

FOREACH(flag ${livepkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -Wall -Werror -Winline -g")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${livepkgs_LDFLAGS})
ADD_EXECUTABLE(${PROJECT_NAME} SHARED
	src/CLiveBox.cpp
	src/CLiveBoxMgr.cpp
	src/CMain.cpp
	src/CResourceMgr.cpp
	src/CUtil.cpp
	src/dlist.cpp
)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION bin)

ADD_SUBDIRECTORY(res)
