
install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/${KWIN_NAME}/decorations/kwin4_decoration_qml_galaxy)

install(FILES package/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}/${KWIN_NAME}
        RENAME kwin4_decoration_qml_galaxy.desktop)
