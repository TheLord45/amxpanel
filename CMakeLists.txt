cmake_minimum_required(VERSION 2.6)
project(hvl)

FIND_PACKAGE( Boost 1.62 COMPONENTS system REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

find_package(PkgConfig)

pkg_check_modules(LIBXML REQUIRED libxml++-3.0)
include_directories(${LIBXML_INCLUDE_DIRS})
link_directories(${LIBXML_LIBRARY_DIRS})
set(LIBS ${LIBXML_LIBRARIES})

pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
include_directories(${GLIBMM_INCLUDE_DIRS})
link_directories(${GLIBMM_LIBRARY_DIRS})
set (LIBS ${LIBS} ${GLIBMM_LIBRARIES})

pkg_check_modules(FREETYPE2 REQUIRED freetype2)
include_directories(${FREETYPE2_INCLUDE_DIRS})
link_directories(${FREETYPE2_LIBRARY_DIRS})
set (LIBS ${LIBS} ${FREETYPE2_LIBRARIES})

pkg_check_modules(ZLIB REQUIRED zlib)
include_directories(${ZLIB_INCLUDE_DIRS})
link_directories(${ZLIB_LIBRARY_DIRS})
set(LIBS ${LIBS} ${ZLIB_LIBRARIES})

if (APPLE)
	set(OSX_INCLUDE -I/opt/local/include -I/usr/local/include)
	set(OSX_LIB -L/opt/local/lib -L/usr/local/lib -lboost_system-mt -liconv)
	set(LIBS ${LIBS} ${OSX_LIB} ${Boost_LIBRARY_DIRS})
endif(APPLE)

add_subdirectory(src)
