project( ksplashx )

add_subdirectory( scale )
add_subdirectory( utils )

include_directories(${PNG_INCLUDE_DIRS}
                    ${JPEG_INCLUDE_DIRS}
                   )

########### next target ###############

set( ksplashx_SRCS
    main.cpp
    qglobal.cpp
    qjpegio.cpp
    qpngio.cpp
    qimage.cpp
    qsize.cpp
    pixmap.cpp
    qcolor.cpp
    qcolor_x11.cpp
    qcolor_p.cpp
    x11_defs.cpp
    defs.cpp
    splash.cpp
    qrect.cpp
    kcpuinfo.cpp
    scale.cpp )

add_executable( ksplashx ${ksplashx_SRCS})
target_link_libraries( ksplashx ${X11_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES})
if( X11_Xinerama_FOUND )
    target_link_libraries( ksplashx ${X11_Xinerama_LIB} )
endif( X11_Xinerama_FOUND )
install( TARGETS ksplashx ${INSTALL_TARGETS_DEFAULT_ARGS} )
