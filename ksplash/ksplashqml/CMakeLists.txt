project(ksplashqml)

add_subdirectory(themes)

set(ksplashqml_SRCS
    main.cpp
    SplashApp.cpp
    SplashWindow.cpp
   )

kde4_add_executable(ksplashqml ${ksplashqml_SRCS})

target_link_libraries(ksplashqml
   ${X11_LIBRARIES}
   Qt4::QtCore
   Qt4::QtDeclarative
   Qt4::QtGui
   )

install(TARGETS ksplashqml ${INSTALL_TARGETS_DEFAULT_ARGS})
