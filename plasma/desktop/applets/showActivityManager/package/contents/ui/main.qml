/*
 *   Copyright 2012 Gregor Taetzner <gregor@freenet.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

MouseArea {
    id: iconContainer
    property string activeSource: "Status"
    height: 16
    width: 16
    property int minimumWidth: 16
    property int minimumHeight: 16
    implicitWidth: theme.iconSizes["panel"]
    implicitHeight: implicitWidth
    onClicked:
    {
	var service = dataSource.serviceForSource(activeSource)
        var operation = service.operationDescription("toggleActivityManager")
        service.startOperationCall(operation)
    }

    PlasmaCore.DataSource {
        id: dataSource
        engine: "org.kde.activities"
        connectedSources: [activeSource]
    }

    PlasmaCore.ToolTip {
        id: tooltip
        mainText: i18n("Show Activity Manager")
        subText: i18n("Click to show the activity manager")
        target: iconContainer
        image: "preferences-activities"
    }

    PlasmaCore.IconItem
    {
        id: icon
        source: "preferences-activities"
        width: parent.width
        height: parent.height
    }
}

