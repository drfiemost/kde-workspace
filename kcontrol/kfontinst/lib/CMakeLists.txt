set(kfontinst_LIB_SRCS Misc.cpp Fc.cpp Family.cpp Style.cpp File.cpp WritingSystems.cpp)
set(kfontinstui_LIB_SRCS FcEngine.cpp )

kde4_add_library(kfontinst SHARED ${kfontinst_LIB_SRCS})
target_link_libraries(kfontinst ${KDE4_KDECORE_LIBRARY} Qt4::QtXml Qt4::QtDBus Qt4::QtCore ${FREETYPE_LIBRARIES} ${FONTCONFIG_LIBRARIES})
set_target_properties(kfontinst PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

kde4_add_library(kfontinstui SHARED ${kfontinstui_LIB_SRCS})
target_link_libraries(kfontinstui ${KDE4_KIO_LIBS} ${FREETYPE_LIBRARIES} ${FONTCONFIG_LIBRARIES} ${X11_X11_LIB} ${X11_Xft_LIB} kfontinst )
set_target_properties(kfontinstui PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kfontinst kfontinstui ${INSTALL_TARGETS_DEFAULT_ARGS} )
