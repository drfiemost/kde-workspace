KDE4_NO_ENABLE_FINAL(styles)

# needed for including kdecoration.h
include_directories (
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/kwin/libkwineffects
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/kwin/libkdecorations
    ${KDEBASE_WORKSPACE_BINARY_DIR}/kwin/libkwineffects
)

########### next target ###############

set(kcm_style_PART_SRCS ../krdb/krdb.cpp styleconfdialog.cpp kcmstyle.cpp)


kde4_add_ui_files(kcm_style_PART_SRCS stylepreview.ui finetuning.ui)

kde4_add_plugin(kcm_style ${kcm_style_PART_SRCS})

target_link_libraries(kcm_style ${KDE4_KIO_LIBS} ${X11_LIBRARIES}
    ${KDE4_PLASMA_LIBS})
if(NOT WIN32)
    target_link_libraries(kcm_style kdecorations)
endif(NOT WIN32)

install(TARGETS kcm_style  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kcmstyle.notifyrc  DESTINATION  ${DATA_INSTALL_DIR}/kcmstyle )
install( FILES style.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
