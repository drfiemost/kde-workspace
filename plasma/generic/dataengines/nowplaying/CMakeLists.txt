project(plasma-dataengine-nowplaying)

set(nowplaying_engine_SRCS
    nowplayingengine.cpp
    playercontrol.cpp
    playeractionjob.cpp
    playercontainer.cpp
    playerinterface/player.cpp
    playerinterface/playerfactory.cpp
    playerinterface/dbuswatcher.cpp
    playerinterface/pollingwatcher.cpp
    playerinterface/juk.cpp
    playerinterface/mpris/mpris.cpp
    playerinterface/mpris/mprisdbustypes.cpp
    playerinterface/mpris2/mpris2.cpp
)

qt4_add_dbus_interface(nowplaying_engine_SRCS playerinterface/org.kde.juk.player.xml juk_interface)

set(mpris_player_desc playerinterface/mpris/org.freedesktop.MediaPlayer.player.xml)
set_source_files_properties(${mpris_player_desc} PROPERTIES
   NO_NAMESPACE true
   INCLUDE "playerinterface/mpris/mprisdbustypes.h"
   CLASSNAME MprisPlayer
)
qt4_add_dbus_interface(nowplaying_engine_SRCS ${mpris_player_desc} mprisplayer)

kde4_add_plugin(plasma_engine_nowplaying ${nowplaying_engine_SRCS})
target_link_libraries(plasma_engine_nowplaying ${KDE4_KIO_LIBS} ${KDE4_PLASMA_LIBS} Qt4::QtGui)

install(TARGETS plasma_engine_nowplaying DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-nowplaying.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES nowplaying.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

