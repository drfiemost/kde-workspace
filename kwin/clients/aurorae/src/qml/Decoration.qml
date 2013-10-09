/********************************************************************
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
import QtQuick 2.0
import org.kde.kwin.decoration 0.1

Item {
    property QtObject borders: Borders {
        objectName: "borders"
    }
    property QtObject maximizedBorders: Borders {
        objectName: "maximizedBorders"
    }
    property QtObject extendedBorders: Borders {
        objectName: "extendedBorders"
    }
    property QtObject padding: Borders {
        objectName: "padding"
    }
    property bool alpha: true

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onPressed: decoration.titlePressed(mouse.button, mouse.buttons)
        onDoubleClicked: decoration.titlebarDblClickOperation()
    }
}
