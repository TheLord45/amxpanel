add_executable(amxpanel main.cpp
            daemonize.cpp
            touchpanel.cpp
            panel.cpp
            page.cpp
            pushbutton.cpp
            palette.cpp
            icon.cpp
            fontlist.cpp
            amxnet.cpp
            config.cpp
            server.cpp
            reply.cpp
            requesthandler.cpp
            requestparser.cpp
            connection.cpp
            connectionmanager.cpp
            requestparser.cpp
            mapping.cpp
            mimetypes.cpp
            nameformat.cpp
            datetime.cpp
            sunset.cpp
            strings.cpp
            regexp.cpp
            syslog.cpp)

add_definitions(-D_REENTRANT)
add_definitions(-D_GNU_SOURCE)
# add_definitions(-DASIO_STANDALONE)

if (DEBUG)
   add_definitions(-g)
endif(DEBUG)

if (APPLE)
	add_definitions(-std=c++14 ${OSX_INCLUDE})
endif(APPLE)

add_definitions(-pedantic -fexceptions -Wextra)

target_link_libraries(amxpanel m pthread gd png z jpeg freetype ${LIBS})

install(TARGETS amxpanel RUNTIME DESTINATION usr)

