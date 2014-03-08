/*   vim:set foldmethod=marker:
 *
 *   Copyright (C) 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.2
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.activities 0.1 as Activities


Flickable {
    id: root

    /*contentWidth: content.width*/
    contentHeight: content.height

    property var model: activitiesModel
    property string filterString: ""

    Activities.ActivityModel {
        id: activitiesModel

        shownStates: "Running,Stopping"
    }

    Activities.ActivityModel {
        id: stoppedActivitiesModel

        shownStates: "Stopped,Starting"
    }

    Column {
        id: content

        width: parent.width

        // Running activities

        Repeater {
            model: activitiesModel

            ActivityItem {

                width:  parent.width

                visible      : (root.filterString == "") ||
                               (title.toLowerCase().indexOf(root.filterString) != -1)

                title        : model.name
                icon         : model.iconSource
                background   : model.background
                current      : model.current
                innerPadding : 2 * units.smallSpacing

                onClicked          : {
                    activitiesModel.setCurrentActivity(model.id, function () {})
                }

                onStopClicked      : {
                    activitiesModel.stopActivity(model.id, function () {});
                }

                onConfigureClicked : {
                    activityConfigurationDialog.activityName = title
                    activityConfigurationDialog.activityIconSource = icon
                    activityConfigurationDialog.visible = true
                }

                ActivityCreationDialog {
                    id: activityConfigurationDialog
                    z: 10

                    acceptButtonText: i18n("Apply")

                    anchors.centerIn: parent
                    width: parent.width
                    visible: false

                    onAccepted: {
                        activitiesModel.setActivityName(model.id,
                            activityConfigurationDialog.activityName,
                            function () {});
                        activitiesModel.setActivityIcon(model.id,
                            activityConfigurationDialog.activityIconSource,
                            function () {});
                    }
                }
            }
        }

        // Stopped activities

        Item {
            // spacer
            width  : parent.width
            height : units.largeSpacing
        }

        PlasmaExtras.Heading {
            text: "Stopped activities:"
            level: 3
            visible: stoppedActivitiesList.count > 0
        }

        Repeater {
            id: stoppedActivitiesList

            model: stoppedActivitiesModel

            delegate: StoppedActivityItem {
                id: stoppedActivityItem

                width:  parent.width

                visible      : (root.filterString == "") ||
                               (title.toLowerCase().indexOf(root.filterString) != -1)

                title        : model.name
                icon         : model.iconSource
                innerPadding : 2 * units.smallSpacing

                onClicked          : {
                    stoppedActivitiesModel.setCurrentActivity(model.id, function () {})
                }

                onDeleteClicked    : {
                    activityDeletionDialog.visible = true
                }

                onConfigureClicked : {
                    activityConfigurationDialog.activityName = title
                    activityConfigurationDialog.activityIconSource = icon
                    activityConfigurationDialog.visible = true
                }

                ActivityDeletionDialog {
                    id: activityDeletionDialog
                    z: 10000

                    anchors.centerIn: parent
                    width: parent.width
                    visible: false

                    onAccepted: {
                        activitiesModel.removeActivity(model.id, function () {})
                    }
                }

                ActivityCreationDialog {
                    id: activityConfigurationDialog
                    z: 10000

                    acceptButtonText: i18n("Apply")

                    anchors.centerIn: parent
                    width: parent.width
                    visible: false

                    onAccepted: {
                        activitiesModel.setActivityName(model.id,
                            activityConfigurationDialog.activityName,
                            function () {});
                        activitiesModel.setActivityIcon(model.id,
                            activityConfigurationDialog.activityIconSource,
                            function () {});
                    }
                }
            }
        }

        add: Transition {
            NumberAnimation {
                properties: "x"
                from: -100
                duration: units.shortDuration
            }
        }

        move: Transition {
            NumberAnimation {
                id: animation
                properties: "y"
                duration: units.longDuration
            }
        }
    }
}

