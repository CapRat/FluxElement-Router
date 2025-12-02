import QtQuick.Controls.Material
import QtQuick
import QtQuick.VirtualKeyboard
import QtQuick.Controls

import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 800
    height: 400
    visible: true
    Material.theme: Material.Dark       // or Material.Light
    Material.accent: Material.Green     // primary accent color
    Material.primary: Material.Teal      // optional main color (buttons, headers)
    title: qsTr("MultiApp")


    StackView {
        id: pages
        anchors.fill: parent
        initialItem: "screens/Home.qml"
    }


    Button{
        x: root.x+50
        y: root.y+50
        text: "Back"
    }
    // navigation helper:
    function open(screen) {
        pages.push(screen)
    }
    function back() {
        pages.pop()
    }

    InputPanel {
        id: inputPanel
        z: 99
        y: root.height
        width: root.width

        states: State {
            name: "visible"
            when: inputPanel.active
            PropertyChanges {
                inputPanel.y: root.height - inputPanel.height
            }
        }
        transitions: Transition {
            from: ""
            to: "visible"
            reversible: true
            NumberAnimation {
                properties: "y"
                easing.type: Easing.InOutQuad
            }
        }
    }
}
