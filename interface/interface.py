from PyQt5.QtCore import QObject, QUrl, pyqtSignal, pyqtSlot, QIODevice
from PyQt5.QtSerialPort import QSerialPort
from PyQt5.QtGui import QGuiApplication
from PyQt5.QtQuick import QQuickView
import sys


class Gui(QObject):
    """Talks to receiver to tx/rx info.

    Attributes:
        changeState: The integer identifier of the receiver.
    """

    changeState = pyqtSignal(str)
    newCoordinate = pyqtSignal(float, float, arguments=['lati', 'longi'])

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
        if text == 'relLat':
            self.relLat = text.lstrip('relLat')
        elif text == 'relLong':
            self.relLong = text.lstrip('relLong')
        elif text == 'relAlt':
            self.relAlt = text.lstrip('relAlt')
            self.signal_new_coordinate(self.relLat, self.relLong)
        elif text == 'absLat':
            self.absLat = text.lstrip('absLat')
        elif text == 'absLong':
            self.absLong = text.lstrip('absLong')
        elif text == 'absAlt':
            self.absAlt = text.lstrip('absAlt')
            self.signal_new_coordinate(self.absLat, self.absLong)
        print(text)

    def signal_new_coordinate(self, lat, long):
        print("new coordinate is: " + str(lat), ", ", str(long))
        self.newCoordinate.emit(lat, long)


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
gui.signal_new_coordinate(52.12478, -106.65946)
app.exec_()
