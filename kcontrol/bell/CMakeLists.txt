


########### next target ###############

set(kcm_bell_PART_SRCS bell.cpp )


kde4_add_plugin(kcm_bell ${kcm_bell_PART_SRCS})


target_link_libraries(kcm_bell  ${KDE4_KDEUI_LIBS} Qt4::QtGui ${X11_X11_LIB})

install(TARGETS kcm_bell  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES bell.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )
