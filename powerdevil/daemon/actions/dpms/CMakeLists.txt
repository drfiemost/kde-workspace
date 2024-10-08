include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${X11_dpms_INCLUDE_PATH})

set(powerdevildpmsaction_SRCS powerdevildpmsaction.cpp)

kde4_add_plugin(powerdevildpmsaction ${powerdevildpmsaction_SRCS})
target_link_libraries(powerdevildpmsaction
        ${X11_LIBRARIES}
        ${KDE4_KDECORE_LIBS}
        Qt4::QtGui
        powerdevilcore)
install(TARGETS powerdevildpmsaction DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES powerdevildpmsaction.desktop DESTINATION ${SERVICES_INSTALL_DIR})

# Action config
set(dpmsconfig_SRCS powerdevildpmsactionconfig.cpp)
kde4_add_plugin(powerdevildpmsaction_config
                ${dpmsconfig_SRCS})
target_link_libraries(powerdevildpmsaction_config
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KDEUI_LIBS}
                      powerdevilui)
install(TARGETS powerdevildpmsaction_config DESTINATION ${PLUGIN_INSTALL_DIR})