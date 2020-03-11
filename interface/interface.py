from PyQt5.QtCore import QObject, QUrl, pyqtSignal, pyqtSlot, QIODevice
from PyQt5.QtSerialPort import QSerialPort
from PyQt5.QtChart import QChartView, QSplineSeries
from PyQt5.QtGui import QGuiApplication
from PyQt5.QtQuick import QQuickView
import sys


class Gui(QObject):
    """Talks to receiver to tx/rx info.

    Attributes:
        changeState: The integer identifier of the receiver.
    """

    changeState = pyqtSignal(str)
    newAbsCoordinate = pyqtSignal(float, float, int, arguments=['lati', 'longi', 'alti'])
    newRelCoordinate = pyqtSignal(float, float, int, arguments=['lati', 'longi', 'alti'])

    def __init__(self, port_name):
        super().__init__()
        self.state = "IDLE"
        # open the serial port
        self.serial = QSerialPort(self)
        self.serial.setPortName(port_name)
        if self.serial.open(QIODevice.ReadWrite):
            self.serial.setBaudRate(QSerialPort.Baud115200, QSerialPort.AllDirections)
        else:
            raise IOError("Cannot connect to device on port")
        self.absLat = 0  # type: float
        self.absLong = 0  # type: float
        self.absAlt = 0  # type: float
        self.relLat = 0  # type: float
        self.relLong = 0  # type: float
        self.relAlt = 0  # type: float

    def connect_signals(self):
        self.changeState.connect(self.set_state)
        self.serial.readyRead.connect(self.recv_serial_msg)

    @pyqtSlot(str, name="set_state")
    def set_state(self, state):
        print("set state to: " + state + "\n")
        self.serial.write(state.encode())

    @pyqtSlot()
    def recv_serial_msg(self):
        text = self.serial.readLine().data().decode()
        text = text.rstrip('\r\n')
        print(text)
        if 'relLat' in text:
            self.relLat = text.lstrip('relLat')
        elif 'relLong' in text:
            self.relLong = text.lstrip('relLong')
        elif 'relAlt' in text:
            self.relAlt = text.lstrip('relAlt')
            self.signal_new_rel_coordinate(self.relLat, self.relLong, self.relAlt)
        elif 'absLat' in text:
            self.absLat = text.lstrip('absLat')
        elif 'absLong' in text:
            self.absLong = text.lstrip('absLong')
        elif 'absAlt' in text:
            self.absAlt = text.lstrip('absAlt')
            self.signal_new_abs_coordinate(self.absLat, self.absLong, self.absAlt)

    def signal_new_abs_coordinate(self, lat, long, alt):
        print("new abs coordinate is: " + str(lat), ", ", str(long), ", ", str(alt))
        float_lat = float(lat)
        float_long = float(long)
        float_alt = float(alt)
        int_alt = int(float_alt)
        self.newAbsCoordinate.emit(float_lat, float_long, int_alt)

    def signal_new_rel_coordinate(self, lat, long, alt):
        print("new rel coordinate is: " + str(lat), ", ", str(long), ", ", str(alt))
        float_lat = float(lat)
        float_long = float(long)
        float_alt = float(alt)
        int_alt = int(float_alt)
        self.newRelCoordinate.emit(float_lat, float_long, int_alt)


app = QGuiApplication([])
view = QQuickView()
view.setWidth(1024)
view.setHeight(720)
view.setTitle('Telemetry Tracker')
view.setResizeMode(QQuickView.SizeRootObjectToView)
url = QUrl('interface.qml')
gui = Gui(sys.argv[1])
gui.connect_signals()
view.rootContext().setContextProperty('gui', gui)
view.setSource(url)
view.show()
qml_window = view.rootObject()
chart = QChartView(qml_window)
spline = QSplineSeries(chart)
spline.append(0, 1)
spline.append(1, 2)
spline.append(2, 3)
app.exec_()
