include_directories (
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_BINARY_DIR}/common
)

add_subdirectory(common)
if(KActivities_FOUND)
  add_subdirectory(activities)
endif(KActivities_FOUND)
add_subdirectory(global)
add_subdirectory(profiles)
