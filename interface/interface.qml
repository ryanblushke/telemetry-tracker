import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Window 2.14
import QtPositioning 5.14
import QtLocation 5.14
import QtQuick.Controls 2.13

Window {
    id: window
    width: 1024
    height: 720
    visible: true

    Plugin {
        id: mapboxglPlugin
        name: "mapboxgl"
    }

    Rectangle {
        id: rectangle
        width: 548
        Layout.alignment: Qt.AlignCenter
        color: "red"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        anchors.top: parent.top
        anchors.topMargin: 45
        anchors.left: parent.left
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.preferredWidth: 40
        Layout.preferredHeight: 40

        Map {
            id: map
            anchors.fill: parent
            plugin: mapboxglPlugin
            center: QtPositioning.coordinate(52.12474, -106.65953) // Saskatoon
            zoomLevel: 19

            MapCircle {
                center: QtPositioning.coordinate(52.12474, -106.65953)
                radius: 2.0
                color: 'red'
                border.width: 3
            }

            MapPolyline {
                line.width: 3
                line.color: 'blue'
                path: [
                    { latitude: 52.12475, longitude: -106.65943 },
                    { latitude: 52.12467, longitude: -106.65945 },
                    { latitude: 52.12476, longitude: -106.65925 },
                    { latitude: 52.12466, longitude: -106.65945 },
                    { latitude: 52.12466, longitude: -106.65925 },
                    { latitude: 52.12477, longitude: -106.65915 }
                ]
            }
        }
    }

    Button {
        id: stateButton1
        x: 900
        y: 189
        text: qsTr("Button")
    }

    Button {
        id: stateButton2
        x: 900
        y: 235
        text: qsTr("Button")
    }

    ScrollView {
        id: scrollView
        x: 824
        y: 329
        width: 200
        height: 200
    }
}

/*##^##
Designer {
    D{i:2;anchors_height:675;anchors_width:548;anchors_x:0;anchors_y:0}
}
##^##*/
