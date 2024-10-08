include_directories( ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/oxygen)

set(kwin_oxygen_config_PART_SRCS
    oxygenanimationconfigwidget.cpp
    oxygenconfig.cpp
    oxygenconfigwidget.cpp
    oxygendetectwidget.cpp
    oxygenexceptiondialog.cpp
    oxygenexceptionlistwidget.cpp
    oxygenexceptionmodel.cpp
    ../oxygenexceptionlist.cpp
  )

kde4_add_ui_files(kwin_oxygen_config_PART_SRCS
   ui/oxygenconfigurationui.ui
   ui/oxygendetectwidget.ui
   ui/oxygenexceptiondialog.ui
   ui/oxygenexceptionlistwidget.ui
)

kde4_add_kcfg_files(kwin_oxygen_config_PART_SRCS ../oxygenconfiguration.kcfgc )

kde4_add_plugin(kwin_oxygen_config ${kwin_oxygen_config_PART_SRCS})

target_link_libraries(
    kwin_oxygen_config
    ${KDE4_KDEUI_LIBS}
    Qt4::QtGui
    ${X11_X11_LIB}
    kdecorations
    oxygenstyle
    oxygenstyleconfig
  )

install(TARGETS kwin_oxygen_config  DESTINATION ${PLUGIN_INSTALL_DIR} )
