/*
 *   Copyright 2011 Viranch Mehta <viranch.mehta@gmail.com>
 *   Copyright 2012 Jacopo De Simoi <wilderkde@gmail.com>
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
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: devicenotifier
    property string devicesType: "removable"
    property string expandedDevice
    property string popupIcon: "device-notifier"

    Plasmoid.switchWidth: units.gridUnit * 10
    Plasmoid.switchHeight: units.gridUnit * 15
    Plasmoid.onExpandedChanged: {
        popupEventSlot(plasmoid.expanded);
    }

    function popupEventSlot(popped) {
        if (!popped) {
            // reset the property that lets us remember if an item was clicked
            // (versus only hovered) for autohide purposes
            notifierDialog.itemClicked = true;
            expandedDevice = "";
            notifierDialog.currentExpanded = -1;
            notifierDialog.currentIndex = -1;
        }
    }

    PlasmaCore.DataSource {
        id: hpSource
        engine: "hotplug"
        connectedSources: sources
        interval: 0
    }

    Plasmoid.compactRepresentation: PlasmaCore.IconItem {
        source: devicenotifier.popupIcon
        width: 36;
        height: 36;
        MouseArea {
            anchors.fill: parent
            onClicked: plasmoid.expanded = !plasmoid.expanded
        }
    }

    PlasmaCore.DataSource {
        id: sdSource
        engine: "soliddevice"
        connectedSources: hpSource.sources
        interval: 0
        property string last
        onSourceAdded: {
            last = source;
            processLastDevice(true)
        }

        onSourceRemoved: {
            if (expandedDevice == source) {
                notifierDialog.currentExpanded = -1;
                expandedDevice = "";
            }
        }

        onDataChanged: {
            processLastDevice(true)
        }

        onNewData: {
            last = sourceName;
            processLastDevice(false);
        }

        function processLastDevice(expand) {
            if (last != "") {
                if (devicesType == "all" ||
                    (devicesType == "removable" && data[last] && data[last]["Removable"] == true) ||
                    (devicesType == "nonRemovable" && data[last] && data[last]["Removable"] == false)) {
                    updateTooltip();
                    if (expand) {
                        expandDevice(last)
                    }
                    last = "";
                }
            }
        }
    }

    PlasmaCore.DataSource {
        id: statusSource
        engine: "devicenotifications"
        property string last
        onSourceAdded: {
            last = source;
            connectSource(source);
        }
        onDataChanged: {
            if (last != "") {
                statusBar.setData(data[last]["error"], data[last]["errorDetails"], data[last]["udi"]);
                plasmoid.status = PlasmaCore.Types.NeedsAttentionStatus;
                plasmoid.expanded = true;
            }
        }
    }

    Component.onCompleted: {
        //plasmoid.addEventListener ('ConfigChanged', configChanged);
        if (sdSource.connectedSources.count == 0) {
            plasmoid.status = PlasmaCore.Types.PassiveStatus;
        }
        updateTooltip()
    }

    function configChanged()
    {
        var all = plasmoid.configuration.allDevices;
        var removable = plasmoid.configuration.removableDevices;
        if (all == true) {
            devicesType = "all";
            filterModel.filterRegExp = "";
        } else if (removable == true) {
            devicesType = "removable";
            filterModel.filterRegExp = "true";
        } else {
            devicesType = "nonRemovable";
            filterModel.filterRegExp = "false";
        }
        notifierDialog.currentIndex = -1;
        notifierDialog.currentExpanded = -1;
    }

    function expandDevice(udi)
    {
        if (hpSource.data[udi]["actions"].length > 1) {
            expandedDevice = udi
        }

        // reset the property that lets us remember if an item was clicked
        // (versus only hovered) for autohide purposes
        notifierDialog.itemClicked = false;

        devicenotifier.popupIcon = "preferences-desktop-notification";
        plasmoid.expanded = true;
        popupIconTimer.restart()
    }

    function updateTooltip()
    {
        var tooltip = new Object
        if (notifierDialog.count == 0) {
            tooltip["image"] = "device-notifier"
            tooltip["mainText"] = i18n("No devices available")
        } else if (sdSource.last != "") {
            tooltip["image"] = sdSource.data[sdSource.last]["Icon"]
            tooltip["mainText"] = i18n("Most recent device")
            tooltip["subText"] = sdSource.data[sdSource.last]["Description"]
        }
        //plasmoid.popupIconToolTip = tooltip // FIXME
    }

    function isMounted (udi) {
        var types = sdSource.data[udi]["Device Types"];
        if (types.indexOf("Storage Access")>=0) {
            if (sdSource.data[udi]["Accessible"]) {
                return true;
            }
            else {
                return false;
            }
        }
        else if (types.indexOf("Storage Volume")>=0 && types.indexOf("OpticalDisc")>=0) {
            return true;
        }
        else {
            return false;
        }
    }

    Timer {
        id: popupIconTimer
        interval: 2500
        onTriggered: devicenotifier.popupIcon  = "device-notifier";
    }

    Timer {
        id: passiveTimer
        interval: 2500
        onTriggered: plasmoid.status = PlasmaCore.Types.PassiveStatus
    }


    Plasmoid.fullRepresentation: MouseArea {
        Layout.minimumWidth: units.gridUnit * 8
        Layout.minimumHeight: units.gridUnit * 10

        hoverEnabled: true
        anchors.fill: parent

        onEntered: notifierDialog.itemHovered()
        onExited: notifierDialog.itemUnhovered()


        PlasmaCore.Svg {
            id: lineSvg
            imagePath: "widgets/line"
        }

        PlasmaComponents.Label {
            id: header
            text: filterModel.count>0 ? i18n("Available Devices") : i18n("No Devices Available")
            anchors { top: parent.top; topMargin: 3; left: parent.left; right: parent.right }
        }

        PlasmaExtras.ScrollArea {
            anchors {
                top : header.bottom
                topMargin: 10
                bottom: statusBarSeparator.top
                left: parent.left
                right: parent.right
            }

            ListView {
                id: notifierDialog

                model: PlasmaCore.SortFilterModel {
                    id: filterModel
                    sourceModel: PlasmaCore.DataModel {
                        dataSource: sdSource
                    }
                    filterRole: "Removable"
                    filterRegExp: {
                        // FIXME: This crashes
                        //var all = plasmoid.configuration.allDevices;
                        //var removable = plasmoid.configuration.removableDevices;
                        print("FIXME: Disabled reading from config due to crash");
                        var all = false;
                        var removable = true;

                        if (all == true) {
                            devicesType = "all";
                            print("ST2P all");
                            return "";
                        } else if (removable == true) {
                            print("ST2P rem true");
                            devicesType = "removable";
                            return "true";
                        } else {
                            print("ST2P nonRemovable");
                            devicesType = "nonRemovable";
                            return "false";
                        }
                    }
                    sortRole: "Timestamp"
                    sortOrder: Qt.DescendingOrder
                }

                property int currentExpanded: -1
                property bool itemClicked: true
                delegate: deviceItem
                highlight: PlasmaComponents.Highlight{}

                //this is needed to make SectionScroller actually work
                //acceptable since one doesn't have a billion of devices
                cacheBuffer: 1000

                onCountChanged: {
                    if (count == 0) {
                        updateTooltip();
                        passiveTimer.restart()
                    } else {
                        passiveTimer.stop()
                        plasmoid.status = PlasmaCore.Types.ActiveStatus
                    }
                }

                function itemHovered()
                {
                    // prevent autohide from catching us!
                    plasmoid.expanded = true;
                }

                function itemUnhovered()
                {
                    if (!itemClicked) {
                        plasmoid.expanded = true;
                    }
                }

                function itemFocused()
                {
                    if (!itemClicked) {
                        // prevent autohide from catching us!
                        itemClicked = true;
                        plasmoid.expanded = true;
                    }
                }

                section {
                    property: "Type Description"
                    delegate: Item {
                        height: childrenRect.height
                        width: notifierDialog.width
                        PlasmaCore.SvgItem {
                            visible: parent.y > 0
                            svg: lineSvg
                            elementId: "horizontal-line"
                            anchors {
                                left: parent.left
                                right: parent.right
                            }
                            height: lineSvg.elementSize("horizontal-line").height
                        }
                        PlasmaComponents.Label {
                            x: 8
                            y: 8
                            opacity: 0.6
                            text: section
                            color: theme.textColor
                        }
                    }
                }

                Component.onCompleted: currentIndex=-1
            }

        }

        Component {
            id: deviceItem

            DeviceItem {
                id: wrapper
                width: notifierDialog.width
                udi: DataEngineSource
                icon: sdSource.data[udi]["Icon"]
                deviceName: sdSource.data[udi]["Description"]
                emblemIcon: Emblems[0]
                state: model["State"]

                percentUsage: {
                    var freeSpace = new Number(sdSource.data[udi]["Free Space"]);
                    var size = new Number(model["Size"]);
                    var used = size-freeSpace;
                    return used*100/size;
                }
                leftActionIcon: {
                    if (mounted) {
                        return "media-eject";
                    } else {
                        return "emblem-mounted";
                    }
                }
                mounted: model["Accessible"]

                onLeftActionTriggered: {
                    var operationName = mounted ? "unmount" : "mount";
                    var service = sdSource.serviceForSource(udi);
                    var operation = service.operationDescription(operationName);
                    service.startOperationCall(operation);
                }
                property bool isLast: (expandedDevice == udi)
                property int operationResult: (model["Operation result"])

                onIsLastChanged: {
                    if (isLast) {
                        notifierDialog.currentExpanded = index
                        makeCurrent();
                    }
                }
                onOperationResultChanged: {
                    if (operationResult == 1) {
                        plasmoid.setPopupIconByName("dialog-ok")
                        popupIconTimer.restart()
                    } else if (operationResult == 2) {
                        plasmoid.setPopupIconByName("dialog-error")
                        popupIconTimer.restart()
                    }
                }
                Behavior on height { NumberAnimation { duration: units.shortDuration * 3 } }
            }
        }

        PlasmaCore.SvgItem {
            id: statusBarSeparator
            svg: lineSvg
            elementId: "horizontal-line"
            height: lineSvg.elementSize("horizontal-line").height
            anchors {
                bottom: statusBar.top
                bottomMargin: statusBar.visible ? 3:0
                left: parent.left
                right: parent.right
            }
            visible: statusBar.height>0
        }

        StatusBar {
            id: statusBar
            anchors {
                left: parent.left
                leftMargin: 5
                right: parent.right
                rightMargin: 5
                bottom: parent.bottom
                bottomMargin: 5
            }
        }
    } // MouseArea
}
