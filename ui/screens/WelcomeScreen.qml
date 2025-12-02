import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
Page {
    id: welcomeScreen
    width: 800
    height: 400
    Rectangle {

        color: "#1e1e1e"     // dark background
        opacity: 0

        // Fade-in animation
        Behavior on opacity {
            NumberAnimation {
                duration: 500; easing.type: Easing.InOutQuad
            }
        }

        Component.onCompleted: opacity = 1

        Column {
            id: content
            anchors.centerIn: parent
            spacing: 20
            width: parent.width * 0.8

            Image {
                source: "/images/forest.jpg"
                fillMode: Image.PreserveAspectFit
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                text: "Welcome!"
                font.pixelSize: 36
                font.bold: true
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                text: "This is your new application."
                font.pixelSize: 18
                color: "#cccccc"
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Button {
                text: "Continue"
                width: 200
                height: 40
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    console.log("Welcome screen: Continue pressed")
                    // signal or navigation logic here
                }
            }

            Button {
                text: "Learn More"
                flat: true
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: Qt.openUrlExternally("https://example.com")
            }
        }
    }
}
