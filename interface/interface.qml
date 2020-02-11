import QtQuick 2.14
import QtQuick.Window 2.14
import QtPositioning 5.14
import QtLocation 5.14

Window {
    width: 1920
    height: 1080
    visible: true

    Plugin {
        id: mapboxglPlugin
        name: "mapboxgl"
    }

    Map {
        anchors.fill: parent
        plugin: mapboxglPlugin
        center: QtPositioning.coordinate(52.12474, -106.65953) // Saskatoon
        zoomLevel: 14

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
