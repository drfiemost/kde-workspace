SET (kephalapp_SRCS kephalapp.cpp)
QT4_WRAP_CPP (kephalapp_SRCS kephalapp.h)

ADD_EXECUTABLE (kephalapp ${kephalapp_SRCS})

INCLUDE_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR})
TARGET_LINK_LIBRARIES (kephalapp Qt4::QtCore Qt4::QtGui ${KDE4_KDECORE_LIBRARY} Qt4::QtDBus kephal)

#INSTALL (TARGETS kephalapp RUNTIME DESTINATION bin)
