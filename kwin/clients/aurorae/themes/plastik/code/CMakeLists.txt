set(plastik_plugin_SRCS
    plastikbutton.cpp
    plastikplugin.cpp
    )

set(CMAKE_AUTOMOC ON)

add_library(plastikplugin SHARED ${plastik_plugin_SRCS})
target_link_libraries(plastikplugin Qt4::QtCore Qt4::QtDeclarative ${KDE4_KDEUI_LIBRARY} kdecorations)
install(TARGETS plastikplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/kwin/decorations/plastik)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/kwin/decorations/plastik)
