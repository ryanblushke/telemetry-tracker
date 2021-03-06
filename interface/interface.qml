/*
    telemetry-tracker is a telemetry tracker system for model rockets/airplanes.

    Copyright (C) 2020 Alex McNabb, Ryan Blushke, Torban Peterson, Scott Seidle

    This file is part of telemetry-tracker.

    telemetry-tracker is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    telemetry-tracker is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with telemetry-tracker.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtPositioning 5.14
import QtLocation 5.14
import QtQuick.Controls 2.13
import QtCharts 2.3

Rectangle {
    id: window
    width: 1200
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
                onNewCenter: map.center = QtPositioning.coordinate(lati, longi)
            }
        }
    }

    Button {
        id: stateButton1
        x: 906
        y: 17
        text: qsTr("ARM")

        onClicked: gui.changeState("ARM")
    }

    Text {
        id: sysstate
        x: 607
        y: 17
        width: 100
        height: 32
        text: qsTr("Current State: IDLE")
        verticalAlignment: Text.AlignTop
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 12

        Connections {
            target: gui
            onStateChanged: sysstate.text = qsTr("Current State: ") + tele_state
        }
    }

    ChartView {
        id: spline
        x: 554
        y: 96
        width: 638
        height: 616
        Connections {
            target: gui
            onNewAbsCoordinate: {
                spline.removeAllSeries()
                var test = spline.createSeries(SplineSeries, "Altitude vs Time", 0, alti)
                var axis_x = spline.axisX(test)
                var axis_y = spline.axisY(test)
                test.append(0, alti)
                axis_x.min = 0
                axis_y.min = alti
                axis_x.max = 1
                axis_y.max = alti
            }
            onNewAlt: {
                var test = spline.series("Altitude vs Time")
                if (test !== null) {
                    var axis_x = spline.axisX(test)
                    var axis_y = spline.axisY(test)
                    axis_x.max = test.count
                    test.append(test.count, alti)
                    if (alti > axis_y.max) axis_y.max = alti
                    if (alti < axis_y.min) axis_y.min = alti
                } else {
                    sysstate.text = "QTERROR"
                }
            }
        }
    }

    Text {
        id: battPct
        x: 780
        y: 17
        width: 100
        height: 32
        text: qsTr("Battery (%):")
        verticalAlignment: Text.AlignTop
        horizontalAlignment: Text.AlignLeft
        font.pixelSize: 12

        Connections {
            target: gui
            onNewBattStat: battPct.text = qsTr("Battery (%): ") + pct
        }
    }

    Text {
        id: battTime
        x: 780
        y: 42
        width: 100
        height: 32
        text: qsTr("Battery (h):")
        verticalAlignment: Text.AlignTop
        horizontalAlignment: Text.AlignLeft
        font.pixelSize: 12

        Connections {
            target: gui
            onNewBattStat: battTime.text = qsTr("Battery (h): ") + time
        }
    }

    Image {
        id: qr
        x: 1012
        y: 0
        width: 180
        height: 146
        fillMode: Image.PreserveAspectFit
        source: ""

        Connections {
            target: gui
            onNewQRCode: qr.source = "qr.png"
        }
    }
}

/*##^##
Designer {
    D{i:2;anchors_height:675;anchors_width:548;anchors_x:0;anchors_y:0}
}
##^##*/
