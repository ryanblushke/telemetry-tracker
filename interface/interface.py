from PyQt5.QtCore import QObject, QUrl, pyqtSignal, pyqtSlot
from PyQt5.QtGui import QGuiApplication
from PyQt5.QtQuick import QQuickView
from enum import Enum
import serial


class State(Enum):
    SLEEP = 1
    IDLE = 2
    ARMED = 3
    ACTIVE = 4
    LANDED = 5
    DATA_TRANSFER = 6


class Receiver:
    """Talks to receiver to tx/rx info.

    Attributes:
        receiver_id: The integer identifier of the receiver.
        ser: Serial port used for communicating with receiver.
    """

    def __init__(self):
        self.receiver_id = 0
        self.ser = serial.Serial('/dev/cu.usbmodem14201')
        print(self.ser.name)
        self.ser.write(b'hello')

    def query_state(self):
        self.receiver_id = 1
        return State.SLEEP

    def set_state(self, state):
        self.receiver_id = 1
        print("Receiver State: " + str(state))

    def send_msg(self, msg):
        self.ser.write(msg.encode())

    def recv_msg(self):
        return self.ser.readline()


class Gui(QObject):
    receiver = Receiver()
    changeState = pyqtSignal(int)
    newCoordinate = pyqtSignal(float, float, arguments=['lati', 'longi'])
    state = receiver.query_state()

    def connect_signals(self):
        self.changeState.connect(self.set_state)

    @pyqtSlot(int, name='set_state')
    def set_state(self, state):
        print("set state to: " + str(state))
        self.receiver.set_state(state)
        self.receiver.send_msg("I'm talking to you.\n")

    def signal_new_coordinate(self, lat, long):
        print("new coordinate is: " + str(lat), ", ", str(long))
        self.newCoordinate.emit(lat, long)


app = QGuiApplication([])
view = QQuickView()
view.setWidth(1024)
view.setHeight(720)
view.setTitle('Hello PyQt')
view.setResizeMode(QQuickView.SizeRootObjectToView)
url = QUrl("interface.qml")
gui = Gui()
gui.connect_signals()
view.rootContext().setContextProperty('gui', gui)
view.setSource(url)
view.show()
qml_window = view.rootObject()
gui.signal_new_coordinate(52.12478, -106.65946)
app.exec_()
