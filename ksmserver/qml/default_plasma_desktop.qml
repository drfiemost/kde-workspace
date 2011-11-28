// -*- coding: iso-8859-1 -*-
/*
 *   Copyright 2011 Lamarque V. Souza <lamarque@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.qtextracomponents 0.1

PlasmaCore.FrameSvgItem {
    id: shutdownUi
    width: margins.left + 18 + 18 + 2 * buttonsLayout.width + margins.right
    height: margins.top + 4 + automaticallyDoLabel.height + 4 + 4 + buttonsLayout.height + 4 + margins.bottom
    imagePath: "dialogs/shutdowndialog"
    enabledBorders: PlasmaCore.FrameSvg.AllBorders

    signal logoutRequested()
    signal haltRequested()
    signal suspendRequested(int spdMethod)
    signal rebootRequested()
    signal rebootRequested2(int opt)
    signal cancelRequested()
    signal lockScreenRequested()

    property variant focusedButton: 0
    property variant lastButton: 0
    property int automaticallyDoSeconds: 30

    PlasmaCore.Theme {
        id: theme
    }

    PlasmaCore.SvgItem {
        id: background
        anchors.fill: parent

        svg: PlasmaCore.Svg {
            imagePath: "dialogs/shutdowndialog"
        }
        elementId: "center"
    }

    Component.onCompleted: {
        //console.log("margins: " + margins.left + ", " + margins.top + ", " + margins.right + ", " + margins.bottom);

        if (background.naturalSize.width < 1) {
            background.elementId = "background"
        }

        if (leftPicture.naturalSize.width < 1) {
            shutdownUi.width -= buttonsLayout.width
            automaticallyDoLabel.width -= buttonsLayout.width
        }

        //console.log("maysd("+maysd+") choose("+choose+") sdtype("+sdtype+")");
        if (choose || sdtype == ShutdownType.ShutdownTypeNone) {
            if (sdtype == ShutdownType.ShutdownTypeNone) {
                focusedButton = logoutButton
            }
        } else {
            logoutButton.opacity = 0
        }

        if (maysd) {
            if(choose || sdtype == ShutdownType.ShutdownTypeHalt) {
                if (sdtype == ShutdownType.ShutdownTypeHalt) {
                    focusedButton = shutdownButton
                }

                if (!spdMethods.StandbyState) {
                    standbyButton.opacity = 0
                }
                if (!spdMethods.SuspendState) {
                    suspendToRamButton.opacity = 0
                }
                if (!spdMethods.HibernateState) {
                    suspendToDiskButton.opacity = 0
                }
            } else {
                shutdownButton.opacity = 0
                standbyButton.opacity = 0
                suspendToRamButton.opacity = 0
                suspendToDiskButton.opacity = 0
            }

            if (choose || sdtype == ShutdownType.ShutdownTypeReboot) {
                if (sdtype == ShutdownType.ShutdownTypeReboot) {
                    focusedButton = rebootButton
                }

                // TODO: add reboot menu
            }
        } else {
            shutdownButton.opacity = 0
            standbyButton.opacity = 0
            suspendToRamButton.opacity = 0
            suspendToDiskButton.opacity = 0
        }

        timer.interval = 1000;
        timer.running = true;
    }

    Timer {
        id: timer
        repeat: true
        running: false

        onTriggered: {
            if (focusedButton != lastButton) {
                lastButton = focusedButton
                automaticallyDoSeconds = 30
            }
            if (focusedButton != 0) {
                if (automaticallyDoSeconds <= 0) { // timeout is at 0, do selected action
                    //console.log("focusedButton == " + focusedButton.text);
                    focusedButton.clicked()
                // following code is required to provide a clean way to translate strings
                } else if (focusedButton.text == logoutButton.text) {
                    //console.log("focusedButton == " + focusedButton.text);
                    automaticallyDoLabel.text = i18np("Logging out in 1 second.",
                                                      "Logging out in %1 seconds.", automaticallyDoSeconds)
                } else if (focusedButton.text == shutdownButton.text) {
                    //console.log("focusedButton == " + focusedButton.text);
                    automaticallyDoLabel.text = i18np("Turning off computer in 1 second.",
                                                      "Turning off computer in %1 seconds.", automaticallyDoSeconds)
                } else if (focusedButton.text == rebootButton.text) {
                    //console.log("ocusedButton == " + focusedButton.text);
                    automaticallyDoLabel.text = i18np("Restarting computer in 1 second.",
                                                      "Restarting computer in %1 seconds.", automaticallyDoSeconds)
                } else {
                    automaticallyDoLabel.text = ""
                }

                --automaticallyDoSeconds;
            }
        }
    }

    Column {
        id: mainColumn
        spacing: 4
        x: shutdownUi.margins.left
        y: shutdownUi.margins.top + 4
        width: 2 * buttonsLayout.width

        PlasmaWidgets.Label {
            id: automaticallyDoLabel
            text: ""
            font.pixelSize: 11
            alignment: Qt.AlignRight
            width: parent.width
        }

        Row {
            id: centerRow
            spacing: 10
            x: parent.x + 18

            Column {
                PlasmaCore.SvgItem {
                    id: leftPicture
                    width: buttonsLayout.width
                    height: width * naturalSize.height / naturalSize.width
                    smooth: true

                    svg: PlasmaCore.Svg {
                        imagePath: "dialogs/shutdowndialog"
                    }
                    elementId: "picture"
                }
            }

            Column {
                id: buttonsLayout
                spacing: 9

                Column {
                    spacing: 4

                    PlasmaWidgets.PushButton {
                        id: logoutButton
                        text: i18n("&Logout")
                        icon: QIcon("system-log-out")
                        width: parent.width
                        height: 32

                        onClicked: {
                            logoutRequested()
                        }
                    }

                    PlasmaWidgets.PushButton {
                        id: shutdownButton
                        text: i18n("&Turn Off Computer")
                        icon: QIcon("system-shutdown")
                        width: parent.width
                        height:32

                        onClicked: {
                            haltRequested()
                        }
                    }

                    PlasmaWidgets.PushButton {
                        id: standbyButton
                        text: i18n("&Standby")
                        icon: QIcon("system-shutdown")
                        width: parent.width
                        height:32

                        onClicked: {
                            suspendRequested(1); // Solid::PowerManagement::StandbyState
                        }
                    }

                    PlasmaWidgets.PushButton {
                        id: suspendToRamButton
                        text: i18n("Suspend to &RAM")
                        icon: QIcon("system-shutdown")
                        width: parent.width
                        height:32

                        onClicked: {
                            suspendRequested(2); // Solid::PowerManagement::SuspendState
                        }
                    }

                    PlasmaWidgets.PushButton {
                        id: suspendToDiskButton
                        text: i18n("Suspend to &Disk")
                        icon: QIcon("system-shutdown")
                        width: parent.width
                        height:32

                        onClicked: {
                            suspendRequested(3); // Solid::PowerManagement::HibernateState
                        }
                    }

                    PlasmaWidgets.PushButton {
                        id: rebootButton
                        text: i18n("&Restart Computer")
                        icon: QIcon("system-reboot")
                        width: parent.width
                        height:32

                        onClicked: {
                            rebootRequested()
                        }
                    }
                }

                PlasmaWidgets.PushButton {
                    id: cancelButton
                    text: i18n("&Cancel")
                    icon: QIcon("dialog-cancel")
                    height: 16
                    anchors.right: parent.right

                    onClicked: {
                        cancelRequested()
                    }
                }
            }
        }
    }
}
