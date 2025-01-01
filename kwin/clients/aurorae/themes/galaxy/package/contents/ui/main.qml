/********************************************************************
Copyright (C) 2015 Giacomo Barazzetti <giacomosrv@gmail.com>
Copyright (C) 2012 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
import QtQuick 1.1
import org.kde.kwin.decoration 0.1
import org.kde.kwin.decorations.plastik 1.0

Decoration {
    function readConfig() {
        switch (decoration.readConfig("BorderSize", DecorationOptions.BorderNormal)) {
        case DecorationOptions.BorderTiny:
            borders.setBorders(3);
            extendedBorders.setAllBorders(0);
            break;
        case DecorationOptions.BorderLarge:
            borders.setBorders(8);
            extendedBorders.setAllBorders(0);
            break;
        case DecorationOptions.BorderVeryLarge:
            borders.setBorders(12);
            extendedBorders.setAllBorders(0);
            break;
        case DecorationOptions.BorderHuge:
            borders.setBorders(18);
            extendedBorders.setAllBorders(0);
            break;
        case DecorationOptions.BorderVeryHuge:
            borders.setBorders(27);
            extendedBorders.setAllBorders(0);
            break;
        case DecorationOptions.BorderOversized:
            borders.setBorders(40);
            extendedBorders.setAllBorders(0);
            break;
        case DecorationOptions.BorderNoSides:
            borders.setBorders(5);
            borders.setSideBorders(1);
            extendedBorders.setSideBorders(3);
            break;
        case DecorationOptions.BorderNone:
            borders.setBorders(1);
            extendedBorders.setBorders(3);
            break;
        case DecorationOptions.BorderNormal: // fall through to default
        default:
            borders.setBorders(5);
            extendedBorders.setAllBorders(0);
            break;
        }
        var titleAlignLeft = decoration.readConfig("titleAlignLeft", true);
        var titleAlignCenter = decoration.readConfig("titleAlignCenter", false);
        var titleAlignRight = decoration.readConfig("titleAlignRight", false);
        if (titleAlignRight) {
            root.titleAlignment = Text.AlignRight;
        } else if (titleAlignCenter) {
            root.titleAlignment = Text.AlignHCenter;
        } else {
            if (!titleAlignLeft) {
                console.log("Error reading title alignment: all alignment options are false");
            }
            root.titleAlignment = Text.AlignLeft;
        }
        root.animateButtons = decoration.readConfig("animateButtons", true);
        root.titleShadow = decoration.readConfig("titleShadow", true);
        if (decoration.animationsSupported) {
            root.animationDuration = 150;
            root.animateButtons = false;
        }
    }
    ColorHelper {
        id: colorHelper
    }
    DecorationOptions {
        id: options
        deco: decoration
    }
    property alias buttonSize: titleRow.captionHeight
    property alias titleAlignment: caption.horizontalAlignment
    property color titleBarColor: options.titleBarColor
    // set by readConfig after Component completed, ensures that buttons do not flicker
    property int animationDuration: 0
    property bool animateButtons: true
    property bool titleShadow: true
    Behavior on titleBarColor {
        ColorAnimation {
            duration: root.animationDuration
        }
    }
    id: root
    alpha: false
    Rectangle {
        id: baseRect
        radius: 6
        color: root.buttonColor
        anchors {
            fill: parent
        }
        border {
            width: decoration.maximized ? 0 : 2
            color: colorHelper.shade(root.buttonColor, ColorHelper.ShadowShade)
        }
        Rectangle {
            id: borderLeft
            anchors {
                left: parent.left
                top: top.bottom
                bottom: parent.bottom
                leftMargin: 1
                bottomMargin: 0
                topMargin: -3
            }
            visible: !decoration.maximized
            width: root.borders.left
            color: root.buttonColor
            Rectangle {
                id: topLeftDecoration
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }
                height: top.height/2
                color: root.titleBarColor
            }
        }
        Rectangle {
            id: borderRight
            anchors {
                right: parent.right
                top: top.bottom
                bottom: parent.bottom
                rightMargin: 1
                bottomMargin: 0
                topMargin: -3
            }
            visible: !decoration.maximzied
            width: root.borders.right -1
            color: root.buttonColor
            Rectangle {
                id: topRightDecoration
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }
                height: top.height/2
                color: root.titleBarColor
            }
        }
        Rectangle {
            id: borderBottom
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                leftMargin: 1
                rightMargin: 1
                bottomMargin: 0
            }
            height: root.borders.bottom
            visible: !decoration.maximized
            color: root.buttonColor
            Rectangle {
                id: bottomLeftDecoration
                anchors {
                    left: parent.left
                    bottom: parent.bottom
                }
                height: root.borders.bottom > 10 ? root.borders.bottom + top.height / 2 : top.height / 2
                width: height
                color: root.titleBarColor
            }
            Rectangle {
                id: bottomRightDecoration
                anchors {
                    right: parent.right
                    bottom: parent.bottom
                }
                height: root.borders.bottom > 10 ? root.borders.bottom + top.height / 2 : top.height / 2
                width: height
                color: root.titleBarColor
            }
            Rectangle {
                height: 1
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                color: colorHelper.shade(root.buttonColor, ColorHelper.ShadowShade)
            }
        }

        Rectangle {
            id: top
            radius: 2
            property int topMargin: 1
            property real normalHeight: titleRow.normalHeight + topMargin + 1
            property real maximizedHeight: titleRow.maximizedHeight + 1
            height: decoration.maximized ? maximizedHeight + 2 : normalHeight + 2
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                topMargin: decoration.maximized ? 0 : top.topMargin
                leftMargin: decoration.maximized ? 0 : 1
                rightMargin: decoration.maximized ? 0 : 1
            }
            gradient: Gradient {
                id: topGradient
                GradientStop {
                    position: 0.1
                    color: Qt.darker(root.titleBarColor, 1.3)
                }
                GradientStop {
                    position: 0.6
                    color: root.titleBarColor
                }
            }
            MouseArea {
                acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
                anchors.fill: parent
                onDoubleClicked: decoration.titlebarDblClickOperation()
                onPressed: {
                    if (mouse.button == Qt.LeftButton) {
                        mouse.accepted = false;
                    } else {
                        decoration.titlePressed(mouse.button, mouse.buttons);
                    }
                }
                onReleased: decoration.titleReleased(mouse.button, mouse.buttons)
            }

            Item {
                id: titleRow
                property real captionHeight: caption.implicitHeight + 4
                property int topMargin: 3
                property int bottomMargin: 2
                property real normalHeight: captionHeight + bottomMargin + topMargin
                property real maximizedHeight: captionHeight + bottomMargin
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    topMargin: decoration.maximized ? 0 : titleRow.topMargin
                    leftMargin: decoration.maximized ? 0 : 4
                    rightMargin: decoration.maximized ? 0 : 4
                    bottomMargin: titleRow.bottomMargin
                }
                ButtonGroup {
                    id: leftButtonGroup
                    spacing: 4
                    explicitSpacer: root.buttonSize
                    menuButton: menuButtonComponent
                    appMenuButton: appMenuButtonComponent
                    minimizeButton: minimizeButtonComponent
                    maximizeButton: maximizeButtonComponent
                    keepBelowButton: keepBelowButtonComponent
                    keepAboveButton: keepAboveButtonComponent
                    helpButton: helpButtonComponent
                    shadeButton: shadeButtonComponent
                    allDesktopsButton: stickyButtonComponent
                    closeButton: closeButtonComponent
                    buttons: options.titleButtonsLeft
                    anchors {
                        top: parent.top
                        left: parent.left
                    }
                }
                Text {
                    id: caption
                    textFormat: Text.PlainText
                    anchors {
                        top: parent.top
                        left: leftButtonGroup.right
                        right: rightButtonGroup.left
                        rightMargin: 5
                        leftMargin: 5
                        topMargin: 1
                    }
                    color: options.fontColor
                    Behavior on color {
                        ColorAnimation { duration: root.animationDuration }
                    }
                    text: decoration.caption
                    font: options.titleFont
                    style: root.titleShadow ? Text.Raised : Text.Normal
                    styleColor: colorHelper.shade(color, ColorHelper.ShadowShade, colorHelper.contrast + 0.2)
                    elide: Text.ElideMiddle
                }
                ButtonGroup {
                    id: rightButtonGroup
                    spacing: 4
                    explicitSpacer: root.buttonSize
                    menuButton: menuButtonComponent
                    appMenuButton: appMenuButtonComponent
                    minimizeButton: minimizeButtonComponent
                    maximizeButton: maximizeButtonComponent
                    keepBelowButton: keepBelowButtonComponent
                    keepAboveButton: keepAboveButtonComponent
                    helpButton: helpButtonComponent
                    shadeButton: shadeButtonComponent
                    allDesktopsButton: stickyButtonComponent
                    closeButton: closeButtonComponent
                    buttons: options.titleButtonsRight
                    anchors {
                        top: parent.top
                        right: parent.right
                    }
                }
            }
        }

        Item {
            id: innerBorder
            anchors.fill: parent

            Rectangle {
                anchors {
                    fill: parent
                    leftMargin: root.borders.left - 1
                    rightMargin: root.borders.right
                    topMargin: root.borders.top - 1
                    bottomMargin: root.borders.bottom
                }
                border {
                    width: 1
                    color: colorHelper.shade(root.buttonColor, ColorHelper.DarkShade)
                }
                visible: !decoration.maximized
                color: root.buttonColor
            }
        }
    }

    Component {
        id: maximizeButtonComponent
        GalaxyButton {
            buttonType: "A"
            size: root.buttonSize
        }
    }
    Component {
        id: keepBelowButtonComponent
        GalaxyButton {
            buttonType: "B"
            size: root.buttonSize
        }
    }
    Component {
        id: keepAboveButtonComponent
        GalaxyButton {
            buttonType: "F"
            size: root.buttonSize
        }
    }
    Component {
        id: helpButtonComponent
        GalaxyButton {
            buttonType: "H"
            size: root.buttonSize
        }
    }
    Component {
        id: minimizeButtonComponent
        GalaxyButton {
            buttonType: "I"
            size: root.buttonSize
        }
    }
    Component {
        id: shadeButtonComponent
        GalaxyButton {
            buttonType: "L"
            size: root.buttonSize
        }
    }
    Component {
        id: stickyButtonComponent
        GalaxyButton {
            buttonType: "S"
            size: root.buttonSize
        }
    }
    Component {
        id: closeButtonComponent
        GalaxyButton {
            buttonType: "X"
            size: root.buttonSize
        }
    }
    Component {
        id: menuButtonComponent
        MenuButton {
            width: root.buttonSize
            height: root.buttonSize
        }
    }
    Component {
        id: appMenuButtonComponent
        GalaxyButton {
            buttonType: "N"
            size: root.buttonSize
        }
    }
    Component.onCompleted: {
        borders.setBorders(5);
        borders.setTitle(top.normalHeight);
        maximizedBorders.setTitle(top.maximizedHeight);
        readConfig();
    }
    Connections {
        target: decoration
        onConfigChanged: readConfig()
    }
}
