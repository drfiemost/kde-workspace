########### add version number into compilation defines
# FIXME: CPACK_PACKAGE_VERSION_* are empty
#add_definitions ( -DAPP_VERSION=\\\"${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}\\\")
add_subdirectory( config )
add_subdirectory( demo )
add_subdirectory( data )

include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/oxygen)

########### next target ###############
set(kwin_oxygen_SRCS
     oxygenbutton.cpp
     oxygenclient.cpp
     oxygenclientgroupitemdata.cpp
     oxygendecohelper.cpp
     oxygenexceptionlist.cpp
     oxygenfactory.cpp
     oxygensizegrip.cpp
     oxygentitleanimationdata.cpp
  )

kde4_add_kcfg_files(kwin_oxygen_SRCS oxygenconfiguration.kcfgc )

kde4_add_plugin(kwin3_oxygen ${kwin_oxygen_SRCS})
target_link_libraries(
    kwin3_oxygen
    ${KDE4_KDEUI_LIBS}
    ${X11_X11_LIB}
    ${X11_Xrender_LIB}
    kdecorations
    oxygenstyle
  )

install(TARGETS kwin3_oxygen  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES oxygenclient.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kwin/ )

