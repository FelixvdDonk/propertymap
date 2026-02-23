import QtQuick

Rectangle {
    id: root
    color: "#1a1a2e"

    Text {
        id: header
        text: "Digital Twin — " + backendName + " — 200 servos"
        font.pixelSize: root.height * 0.03
        font.bold: true
        color: "#e94560"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 10
        z: 1
    }

    // Show a grid of 200 servo "gauges" — each bound to a property
    GridView {
        id: grid
        anchors.fill: parent
        anchors.topMargin: header.height + 20
        anchors.margins: 10

        cellWidth: root.width / 20
        cellHeight: root.height / 12

        model: 200

        delegate: Item {
            width: grid.cellWidth
            height: grid.cellHeight

            Rectangle {
                anchors.fill: parent
                anchors.margins: 1
                color: "#16213e"
                radius: 3

                // This binding is identical for both backends:
                //   servo.servo0, servo.servo1, ... servo.servo199
                property real servoValue: servo["servo" + index]

                Rectangle {
                    id: bar
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 2

                    height: parent.height * Math.abs(parent.servoValue) / 360
                    color: parent.servoValue >= 0 ? "#0f3460" : "#e94560"
                    radius: 2

                    Behavior on height { NumberAnimation { duration: 30 } }
                }

                Text {
                    text: index
                    font.pixelSize: parent.height * 0.2
                    color: "#a0a0a0"
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 2
                }
            }
        }
    }
}
