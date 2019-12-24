add_executable(amxpanel main.cpp
            daemonize.cpp
            touchpanel.cpp
            panel.cpp
            page.cpp
            pushbutton.cpp
            palette.cpp
            icon.cpp
            fontlist.cpp
            map.cpp
            amxnet.cpp
            websocket.cpp
            expand.cpp
            directory.cpp
            config.cpp
            nameformat.cpp
            datetime.cpp
            sunset.cpp
            str.cpp
            syslog.cpp
            trace.cpp)

add_definitions(-D_REENTRANT)
add_definitions(-D_GNU_SOURCE)

if (DEBUG)
   add_definitions(-g)
endif(DEBUG)

if (APPLE)
	add_definitions(-std=c++17 ${OSX_INCLUDE})
else(APPLE)
    add_definitions(-DASIO_STANDALONE)
	add_definitions(-std=c++17)
endif(APPLE)

add_definitions(-pedantic -fexceptions -Wextra)

if(CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
	target_link_libraries(amxpanel stdc++fs)
endif()

target_link_libraries(amxpanel m pthread ssl crypto gd png z jpeg freetype cidr ${LIBS} ${Boost_LIBRARIES})

install(TARGETS amxpanel RUNTIME DESTINATION sbin)

