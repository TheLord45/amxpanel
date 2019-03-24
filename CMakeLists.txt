cmake_minimum_required(VERSION 2.6)
project(hvl)

FIND_PACKAGE( Boost 1.62 COMPONENTS system REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

find_package(PkgConfig)

pkg_check_modules(LIBXML++ REQUIRED libxml++-2.6)
include_directories(${LIBXML++_INCLUDE_DIRS})
link_directories(${LIBXML++_LIBRARY_DIRS})
set(LIBS ${LIBXML++_LIBRARIES})

pkg_check_modules(FREETYPE2 REQUIRED freetype2)
include_directories(${FREETYPE2_INCLUDE_DIRS})
link_directories(${FREETYPE2_LIBRARY_DIRS})
set (LIBS ${LIBS} ${FREETYPE2_LIBRARIES})

if (APPLE)
	set(OSX_INCLUDE -I/usr/local/include -I/opt/local/include)
	set(OSX_LIB -L/usr/local/lib -L/opt/local/lib -lboost_system-mt -liconv)
	set(LIBS ${LIBS} ${OSX_LIB} ${Boost_LIBRARY_DIRS})
endif(APPLE)

add_subdirectory(src)
