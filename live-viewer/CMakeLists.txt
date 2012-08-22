PROJECT(live-viewer CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	dlog
	ail
	elementary
	ecore-x
	appcore-efl
	livebox-viewer
)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -Wall -Werror -Winline -g")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

ADD_DEFINITIONS("-DNDEBUG")
#ADD_DEFINITIONS("-DFLOG")
ADD_DEFINITIONS(${pkgs_CFLAGS})
ADD_DEFINITIONS(${pkgs_LDFLAGS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

ADD_EXECUTABLE(${PROJECT_NAME}
	src/CLiveBox.cpp
	src/CLiveBoxMgr.cpp
	src/CMain.cpp
	src/CResourceMgr.cpp
	src/CUtil.cpp
	src/dlist.cpp
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin)

ADD_SUBDIRECTORY(res)
