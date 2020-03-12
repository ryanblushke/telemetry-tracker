import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtPositioning 5.14
import QtLocation 5.14
import QtQuick.Controls 2.13
import QtCharts 2.3

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
            center: QtPositioning.coordinate(52.1247, -106.6595) // Saskatoon
            zoomLevel: 19
            activeMapType: map.supportedMapTypes[1]

            MapCircle {
                id: startingLocation
                center: QtPositioning.coordinate(52.1247, -106.6595)
                radius: 2.0
                color: 'green'
                border.width: 3

                Connections {
                    target: gui
                    onNewAbsCoordinate: startingLocation.center = QtPositioning.coordinate(lati, longi)
                }
            }

            MapCircle {
                id: currentLocation
                center: QtPositioning.coordinate(52.1247, -106.6595)
                radius: 2.0
                color: 'red'
                border.width: 3

                Connections {
                    target: gui
                    onNewAbsCoordinate: currentLocation.center = QtPositioning.coordinate(lati, longi)
                    onNewRelCoordinate: currentLocation.center = QtPositioning.coordinate(lati, longi)
                }
            }

            MapPolyline {
                id: flightPath
                line.width: 3
                line.color: 'blue'

                Connections {
                    target: gui
                    onNewAbsCoordinate: flightPath.insertCoordinate(flightPath.pathLength(), QtPositioning.coordinate(lati, longi))
                    onNewRelCoordinate: flightPath.insertCoordinate(flightPath.pathLength(), QtPositioning.coordinate(lati, longi))
                }
            }

            Connections {
                target: gui
                onNewAbsCoordinate: map.center = QtPositioning.coordinate(lati, longi)
            }
        }
    }

    Button {
        id: stateButton1
        x: 900
        y: 189
        text: qsTr("ARM")

        onClicked: gui.changeState("ARM")
    }

    Text {
        id: element
        x: 601
        y: 8
        width: 100
        height: 32
        text: qsTr("Current State: IDLE")
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 12

        Connections {
            target: gui
            onStateChanged: element.text = qsTr("Current State: ") + tele_state
        }
    }

    ChartView {
        id: spline
        x: 554
        y: 210
        width: 300
        height: 300
        Connections {
            target: gui
            onNewAbsCoordinate: {
                spline.removeAllSeries()
                var test = spline.createSeries(SplineSeries, "Altitude vs Time", 0, alti)
                var axis_x = spline.axisX(test)
                var axis_y = spline.axisY(test)
                test.append(0, alti)
                axis_x.min = 0
                axis_y.min = 0
                axis_x.max = 1
                axis_y.max = 9848
            }
            onNewRelCoordinate: {
                var test = spline.series("Altitude vs Time")
                if (test !== null) {
                    var axis_x = spline.axisX(test)
                    var axis_y = spline.axisY(test)
                    axis_x.max = test.count + 1
                    test.append(test.count, alti)
                } else {
                    element.text = "QTERROR"
                }
            }
        }
    }
}

/*##^##
Designer {
    D{i:2;anchors_height:675;anchors_width:548;anchors_x:0;anchors_y:0}
}
##^##*/
