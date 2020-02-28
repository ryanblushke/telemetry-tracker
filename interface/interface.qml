import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtPositioning 5.14
import QtLocation 5.14
import QtQuick.Controls 2.13

Rectangle {
    id: window
    width: 1024
    height: 720
    visible: true

    signal clicked()

    Plugin {
        id: mapplugin
        name: "osm"

        PluginParameter {
            name: 'osm.mapping.providersrepository.disabled'
            value: true
        }

        PluginParameter {
            id: offlineDirectory
            name: "osm.mapping.offline.directory"
            value: "/Users/ryanblushke/telemetry-tracker-capstone/interface/offline_tiles/"
        }

        PluginParameter {
            id: highDPI
            name: "osm.mapping.highdpi_tiles"
            value: true
        }
    }

    Rectangle {
        id: rectangle
        width: 548
        Layout.alignment: Qt.AlignCenter
        color: "red"
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.top: parent.top
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.preferredWidth: 40
        Layout.preferredHeight: 40

        Map {
            id: map
            anchors.fill: parent
            plugin: mapplugin
            center: QtPositioning.coordinate(52.12474, -106.65953) // Saskatoon
            zoomLevel: 19
            activeMapType: map.supportedMapTypes[1]

            MapCircle {
                id: currentLocation
                center: QtPositioning.coordinate(52.12474, -106.65953)
                radius: 2.0
                color: 'red'
                border.width: 3
            }

            MapPolyline {
                id: flightPath
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

                Connections {
                    target: gui
                    onNewCoordinate: flightPath.insertCoordinate(flightPath.pathLength(), QtPositioning.coordinate(lati, longi))
                }
            }
        }
    }

    Button {
        id: stateButton1
        x: 900
        y: 189
        text: qsTr("State1")

        onClicked: gui.changeState(1)
    }

    Button {
        id: stateButton2
        x: 900
        y: 235
        text: qsTr("State2")

        onClicked: gui.changeState(2)
    }
}

/*##^##
Designer {
    D{i:2;anchors_height:675;anchors_width:548;anchors_x:0;anchors_y:0}
}
##^##*/
