configure_file(kde-plasma.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kde-plasma.desktop)
configure_file(kde-plasma-safe.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kde-plasma-safe.desktop)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/kde-plasma.desktop
	${CMAKE_CURRENT_BINARY_DIR}/kde-plasma-safe.desktop
	cde.desktop
	fluxbox.desktop
	flwm.desktop
	fvwm.desktop
	fvwm95.desktop
    gnome.desktop
	icewm.desktop
	lwm.desktop
	openbox.desktop
	twm.desktop
	ude.desktop
	xfce.desktop
	xfce4.desktop

	DESTINATION ${DATA_INSTALL_DIR}/kdm/sessions
)
