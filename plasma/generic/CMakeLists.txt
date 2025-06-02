if(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")
   add_subdirectory(containmentactions)
endif(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")

add_subdirectory(applets)
add_subdirectory(dataengines)
add_subdirectory(runners)
add_subdirectory(scriptengines)
add_subdirectory(shells)
add_subdirectory(wallpapers)

