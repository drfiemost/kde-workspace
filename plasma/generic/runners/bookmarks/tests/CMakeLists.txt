set( testChromeBookmarks_SRCS testchromebookmarks.cpp 
  ../browsers/chrome.cpp
  ../faviconfromblob.cpp
  ../browsers/chromefindprofile.cpp
  ../bookmarkmatch.cpp
  ../favicon.cpp
  ../fetchsqlite.cpp
)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/.. )
kde4_add_unit_test( testChromeBookmarks TESTNAME plasma-runner-bookmarks-TestChromeBookmarks ${testChromeBookmarks_SRCS} )

target_link_libraries( testChromeBookmarks ${KDE4_KDECORE_LIBS} Qt4::QtTest Qt4::QtSql ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} qjson)

file(COPY chrome-config-home DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
