 
/*   vim:set foldenable foldmethod=marker:
 *
 *   Copyright (C) 2011 Ivan Cukic <ivan.cukic(at)kde.org>
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


import Qt 4.7
import QtQuick 1.1

 
Item {
    id: main

    width: screenSize.width
    height: screenSize.height
    // width: 300
    // height: 300

    /* property declarations --------------------------{{{ */
    property int stage
    property int iconSize: (screenSize.width <= 1024) ? 64 : 128
    /* }}} */

    /* signal declarations ----------------------------{{{ */

    /* }}} */

    /* JavaScript functions ---------------------------{{{ */
    
    onStageChanged: {
        if (stage == 1) {
            wallpaper.opacity = 1
            gear.opacity = 1
            steptwo.opacity = 0.5
            loadingText.opacity = 0.5
            s1.opacity = 1
	    pointin.opacity = 0
            pointout.opacity = 0
            test.x= width * 0
        }
        if (stage == 2) {
	    
	    
	    loadingText.opacity = 0.4
	    s1.opacity = 0
	    pointin.opacity = 0
            pointout.opacity = 0
            s2.opacity = 1
	    test.x= width * 0.2
	    
        }
        if (stage == 3) {
	    s2.opacity = 0
	    loadingText.opacity = 0.3
	    s3.opacity = 1
	    test.x= width * 0.4
        }
        if (stage == 4) {
	    s3.opacity = 0
	    loadingText.opacity = 0.2
	    s4.opacity = 1
	    test.x= width * 0.6
	    
        }
        if (stage == 5) {
	    s4.opacity = 0
            loadingText.opacity = 0.1
	    logo.opacity = 0.8
            test.x= width * 0.8
        }
        if (stage == 6) {
	    
	    loadingText.opacity = 0
	    logo.opacity = 1
	    stepthree.opacity = 1  
	    stepfour.opacity = 1
	    
	   
        }
        
    }
    /* }}} */

    /* object properties ------------------------------{{{ */

    /* }}} */

    /* child objects ----------------------------------{{{ */

    

    
    Image {
	id: wallpaper
   
	height: parent.height
	width: parent.width
   
	x: 0
	y: 0
   
	source: "images/background.png"
	opacity: 0
    }
    
     
	  
     Text {
	  id: loadingText
            // HTML like markup can also be used
            text: '<font color="white"><b>L</b>oading <b>D</b>esktop...</font>'

            // This could also be written font { pointSize: 14 }. Both syntaxes are valid.
            font.pointSize: 10

            // StyledText format supports fewer tags, but is more efficient than RichText
            textFormat: Text.StyledText
	    x: (parent.width - width) / 2 
	    y: 250 + (parent.height - height) / 2
	    opacity: 0
	    Behavior on opacity { NumberAnimation { duration: 1000; easing { type: Easing.InOutQuad } } }

     }
     
     Text {
             id: pointin
             //anchors.centerIn: parent
             color: "white"
             text: "......"
             font.pixelSize: 12
             textFormat: Text.StyledText
             
	     x: (parent.width - width) / 2 
	     y: 275 + (parent.height - height) / 2
	     opacity: 0
             SequentialAnimation on font.letterSpacing {
                 loops: Animation.Infinite;
                 NumberAnimation { from: 0; to: 200; easing.type: Easing.InQuad; duration: 3000 }
                 ScriptAction {
                     script: {
                         container.y = (screen.height / 4) + (Math.random() * screen.height / 2)
                         container.x = (screen.width / 4) + (Math.random() * screen.width / 2)
                     }
                 }
             }

             SequentialAnimation on opacity {
                 loops: Animation.Infinite;
                 NumberAnimation { from: 1; to: 0; duration: 2600 }
                 PauseAnimation { duration: 400 }
             }
             
	     Behavior on opacity { NumberAnimation { duration: 1000; easing { type: Easing.InOutQuad } } }
	     
    }
    
    Text {
             id: pointout
             //anchors.centerIn: parent
             color: "white"
             text: "......"
             font.pixelSize: 12
             textFormat: Text.StyledText
	     x: (parent.width - width) / 2 
	     y: 275 + (parent.height - height) / 2
	     
             SequentialAnimation on font.letterSpacing {
                 loops: Animation.Infinite;
                 NumberAnimation { from: 300; to: 0; easing.type: Easing.OutQuad; duration: 3000 }
                 ScriptAction {
                     script: {
                         container.y = (screen.height / 4) + (Math.random() * screen.height / 2)
                         container.x = (screen.width / 4) + (Math.random() * screen.width / 2)
                     }
                 }
             }

             SequentialAnimation on opacity {
                 loops: Animation.Infinite;
                 NumberAnimation { from: 1; to: 0; duration: 2600 }
                 PauseAnimation { duration: 400 }
             }
             opacity: 0
	     Behavior on opacity { NumberAnimation { duration: 1000; easing { type: Easing.InOutQuad } } }
    }
     
     Text {
	id: data
        text:Qt.formatDateTime(new Date(), "dd.MM.yyyy    hh:mm")
	font.pointSize: 18
	color: "white"
	font { family: 'Electrolize'; pixelSize: 18; italic: false; bold:true; capitalization: Font.SmallCaps }

        // StyledText format supports fewer tags, but is more efficient than RichText
        textFormat: Text.StyledText
	x: (parent.width - width) / 2 
	y: 0 + (parent.height - height) / 2
	opacity: 0
    }
    
       
    
      Image {
	id: s1
	height: iconSize * 0.5
        width: iconSize * 0.5
        smooth: true

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
	source:  "images/monitor.png"
	opacity: 0
	
	Behavior on opacity { NumberAnimation { duration: 1000; easing { type: Easing.InOutQuad } } }
    }

     Image {
	id: s2
	height: iconSize * 0.5
        width: iconSize * 0.5
        smooth: true

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
	source:  "images/preferences.png"
	opacity: 0
	
	Behavior on opacity { NumberAnimation { duration: 1000; easing { type: Easing.InOutQuad } } }
    }
    Image {
	id: s3
	height: iconSize * 0.5
        width: iconSize * 0.5
        smooth: true

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
	source:  "images/internet.png"
	opacity: 0
	
	Behavior on opacity { NumberAnimation { duration: 1000; easing { type: Easing.InOutQuad } } }
    }
    Image {
	id: s4
	height: iconSize * 0.5
        width: iconSize * 0.5
        smooth: true

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
	source:  "images/user.png"
	opacity: 0
	
	Behavior on opacity { NumberAnimation { duration: 1000; easing { type: Easing.InOutQuad } } }
    }
      
      
    Image {
        id: stepone

        height: iconSize * 2
        width: iconSize * 2
        smooth: true

        x: (parent.width - width) / 2 
        y: (parent.height - height) / 2

        source: "images/gear1.png"

        opacity: 1.9
        Behavior on opacity { NumberAnimation { duration: 1000; easing { type: Easing.InOutQuad } } }
        
        NumberAnimation {
            id: animateRotationthree
            target: stepone
            properties: "rotation"
            from: 0
            to: 360
            duration: 30000

            loops: Animation.Infinite
            running: true
        }
          
    }
    
    Image {
        id: stepthree

        height: iconSize * 1.22
        width: iconSize * 1.22
        smooth: true

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        source: "images/ready-1.png"

        opacity: 0
        
        Behavior on opacity { NumberAnimation { duration: 1000; easing { type: Easing.InOutQuad } } }
    }
    
    Image {
        id: stepfour

        height: iconSize * 2.5
        width: iconSize * 2.5
        smooth: true

        x: (parent.width - width) / 2 
        y: (parent.height - height) / 2

        source: "images/gear5.png"

        opacity: 0
        Behavior on opacity { NumberAnimation { duration: 1000; easing { type: Easing.InOutQuad } } }
    }
    
    Image {
        id: gear

        height: iconSize * 2
        width: iconSize * 2
        smooth: true

        x: (parent.width - width)/2
        y: (parent.height - height)/2

        source: "images/gear2.png"

        opacity: 0
        Behavior on opacity { NumberAnimation { duration: 1000; easing { type: Easing.InOutQuad } } }

        NumberAnimation {
            id: animateRotation
            target: gear
            properties: "rotation"
            from: 360
            to: 0
            duration: 5000

            loops: Animation.Infinite
            running: true
        }

    }

    Image {
        id: steptwo

        height: iconSize * 2
        width: iconSize * 2
        smooth: true

        x: (parent.width - width) / 2 
        y: (parent.height - height) / 2

        source: "images/gear3-2.png"

        opacity: 0
        Behavior on opacity { NumberAnimation { duration: 1000; easing { type: Easing.InOutQuad } } }
        
        NumberAnimation {
            id: animateRotationtwo
            target: steptwo
            properties: "rotation"
            from: 0
            to: 360
            duration: 1000

            loops: Animation.Infinite
            running: true
        }
    }
    
    
    
    Image {
        id: logo

        height: iconSize * 1
        width: iconSize * 1
        smooth: true

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2 - 1

        source: "images/ready.png"

        opacity: 0
        Behavior on opacity { NumberAnimation { properties:"opacity"; duration: 2000; easing { type: Easing.InOutQuad } } }
                
    }
    
   
    

    /* }}} */

    /* stages -----------------------------------------{{{ */

    /* }}} */

    /* transitions ------------------------------------{{{ */

    /* }}} */
}

