project(plasma-runner-kill)

set(krunner_kill_SRCS killrunner.cpp)

set(kcm_krunner_kill_SRCS
    killrunner_config.cpp
)

kde4_add_ui_files(kcm_krunner_kill_SRCS killrunner_config.ui)
kde4_add_plugin(kcm_krunner_kill ${kcm_krunner_kill_SRCS})
target_link_libraries(kcm_krunner_kill
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KCMUTILS_LIBS} 
    Qt4::QtCore
    Qt4::QtGui
    )
    
kde4_add_plugin(krunner_kill ${krunner_kill_SRCS})
target_link_libraries(krunner_kill
                      ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} processcore)
add_dependencies(krunner_kill kcm_krunner_kill)

install(TARGETS krunner_kill kcm_krunner_kill
        DESTINATION ${PLUGIN_INSTALL_DIR})
        
install(FILES plasma-runner-kill.desktop plasma-runner-kill_config.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
