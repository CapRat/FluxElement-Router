import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: homeScreen
    title: "Forest Home"
    opacity: 0

    // Fade-in animation
    Behavior on opacity {
        NumberAnimation {
            duration: 500; easing.type: Easing.InOutQuad
        }
    }

    Component.onCompleted: opacity = 1
    // Full-screen forest background


    // Optional semi-transparent overlay for readability
    Rectangle {
        anchors.fill: parent
        color: "black"
        opacity: 0.25
    }

    // Main content
    Column {
        anchors.centerIn: parent
        spacing: 24
        width: parent.width * 0.8

        // Title
        Text {
            text: "Welcome to ForestApp"
            font.pixelSize: 36
            font.bold: true
            color: Material.primary
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // Subtitle
        Text {
            text: "Explore the greenery!"
            font.pixelSize: 20
            color: Material.accent
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // Buttons
        Button {
            text: "Start Routing"
            width: 200
            height: 40
            onClicked: root.open("screens/PipewireRouting.qml")
        }

        Button {
            text: "Settings"
            width: 200
            height: 40
            flat: true
            onClicked: root.open("screens/SettingsScreen.qml")
        }
    }
}
