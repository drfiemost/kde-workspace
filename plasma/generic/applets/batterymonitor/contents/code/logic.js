/*
 *   Copyright 2011 Sebastian Kügler <sebas@kde.org>
 *   Copyright 2012 Viranch Mehta <viranch.mehta@gmail.com>
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

var ram = 0
var disk = 1

function updateCumulative() {
    var sum = 0;
    var count = 0;
    var charged = true;
    for (var i=0; i<batteries.count; i++) {
        var b = batteries.get(i);
        if (!b["Is Power Supply"]) {
          continue;
        }
        if (b["Plugged in"]) {
            sum += b["Percent"];
        }
        if (b["State"] != "NoCharge") {
            charged = false;
        }
        count++;
    }

    if (count > 0) {
      batteries.cumulativePercent = Math.round(sum/count);
    } else {
        // We don't have any power supply batteries
        // Use the lowest value from any battery
        if (batteries.count > 0) {
            var lowestPercent = 100;
            for (var i=0; i<batteries.count; i++) {
                var b = batteries.get(i);
                if (b["Percent"] && b["Percent"] < lowestPercent) {
                    lowestPercent = b["Percent"];
                }
            }
            batteries.cumulativePercent = lowestPercent;
        } else {
            batteries.cumulativePercent = 0;
        }
    }
    batteries.allCharged = charged;
}

function stringForState(batteryData) {
    var pluggedIn = batteryData["Plugged in"];
    var percent = batteryData["Percent"];
    var state = batteryData["State"];
    var powerSupply = batteryData["Is Power Supply"];

    var text="<b>";
    if (pluggedIn) {
        // According to UPower spec, the chargeState is only valid for primary batteries
        if (powerSupply) {
            switch(state) {
                case "NoCharge": text += i18n("%1% (charged)", percent); break;
                case "Discharging": text += i18n("%1% (discharging)", percent); break;
                default: text += i18n("%1% (charging)", percent);
            }
        } else {
            text += i18n("%1%", percent);
        }
    } else {
        text += i18nc("Battery is not plugged in", "Not present");
    }
    text += "</b>";

    return text;
}

function stringForBatteryState(batteryData) {
    switch(batteryData["State"]) {
        case "NoCharge": return i18n("Not Charging");
        case "Discharging": return i18n("Discharging");
        case "FullyCharged": return i18n("Fully Charged");
        default: return i18n("Charging");
    }
}

function iconForBattery(batteryData,pluggedIn) {
    switch(batteryData["Type"]) {
        case "Monitor":
            return "video-display";
        case "Mouse":
            return "input-mouse";
        case "Keyboard":
            return "input-keyboard";
        case "Pda":
            return "pda";
        case "Phone":
            return "phone";
        default: // Primary and UPS
            p = batteryData["Percent"];
            if (p >= 90) {
                fill = "100";
            } else if (p >= 70) {
                fill = "080";
            } else if (p >= 50) {
                fill = "060";
            } else if (p >= 30) {
                fill = "040";
            } else if (p >= 10) {
                fill = "caution";
            } else {
                fill = "low";
            }

            if (pluggedIn && batteryData["Is Power Supply"]) {
                return "battery-charging-" + fill;
            } else {
                if (p < 5) {
                    return "dialog-warning"
                }
                return "battery-" + fill;
            }
    }
}

function updateTooltip() {
    var text="";

    for (var i=0; i<batteries.count; i++) {
        var b = batteries.get(i);
        if (text != "") {
            text += "<br/>";
        }

        text += b["Pretty Name"];

        text += ": ";
        text += stringForState(pmSource.data["Battery"+i]);
    }

    if (text != "") {
        text += "<br/>";
    }

    if (pmSource.data["AC Adapter"]) {
        text += i18nc("tooltip", "AC Adapter:") + " ";
        text += pmSource.data["AC Adapter"]["Plugged in"] ? i18nc("tooltip", "<b>Plugged in</b>") : i18nc("tooltip", "<b>Not plugged in</b>");
    }
    batteries.tooltipText = "<p style='white-space: nowrap'>" + text + "</p>";
}

function updateBrightness() {
    // we don't want passive brightness change send setBrightness call
    if (!pmSource.data["PowerDevil"]) {
        return;
    }
    dialogItem.disableBrightnessUpdate = true;
    dialogItem.screenBrightness = pmSource.data["PowerDevil"]["Screen Brightness"];
    dialogItem.keyboardBrightness = pmSource.data["PowerDevil"]["Keyboard Brightness"];
    dialogItem.disableBrightnessUpdate = false;
}

function callForType(type) {
    if (type == ram) {
        return "suspendToRam";
    } else if (type == disk) {
        return "suspendToDisk";
    }

    return "suspendHybrid";
}

// TODO: give translated and formatted string with KGlobal::locale()->prettyFormatDuration(msec);
function formatDuration(msec) {
    if (msec==0)
        return "";

    var time = new Date(msec);
    var hours = time.getUTCHours();
    var minutes = time.getUTCMinutes();
    var str = "";
    if (hours > 0) str += i18np("1 hour ", "%1 hours ", hours);
    if (minutes > 0) str += i18np("1 minute", "%1 minutes", minutes);
    return str;
}