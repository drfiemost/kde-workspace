


########### next target ###############

set(kcm_display_PART_SRCS display.cpp )


kde4_add_plugin(kcm_display ${kcm_display_PART_SRCS})


target_link_libraries(kcm_display  ${KDE4_KCMUTILS_LIBS} Qt4::QtGui ${KDE4_KDEUI_LIBS})

install(TARGETS kcm_display  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES display.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
