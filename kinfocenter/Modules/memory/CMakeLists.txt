
########### next target ###############

set(KCM_MEMORY_PART_SRCS memory.cpp chartWidget.cpp totalMemoryChart.cpp physicalMemoryChart.cpp swapMemoryChart.cpp )

kde4_add_plugin(kcm_memory ${KCM_MEMORY_PART_SRCS})

target_link_libraries(kcm_memory ${KDE4_KDEUI_LIBS} Qt4::QtGui)

if (${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
   target_link_libraries(kcm_memory kvm)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")

install(TARGETS kcm_memory  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kcm_memory.desktop  DESTINATION  ${SERVICES_INSTALL_DIR})
