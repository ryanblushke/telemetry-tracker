from PyQt5.QtCore import QObject, QUrl, pyqtSignal, pyqtSlot, QIODevice, QTimer
from PyQt5.QtSerialPort import QSerialPort
from PyQt5.QtWidgets import QApplication
from PyQt5.QtQuick import QQuickView
import sys
import serial

class Gui(QObject):
    """Talks to receiver to tx/rx info.

    Attributes:
        changeState: The integer identifier of the receiver.
    """

    changeState = pyqtSignal(str)
    stateChanged = pyqtSignal(str, arguments=['tele_state'])
    newAbsCoordinate = pyqtSignal(float, float, int, arguments=['lati', 'longi', 'alti'])
    newRelCoordinate = pyqtSignal(float, float, arguments=['lati', 'longi'])
    newAlt = pyqtSignal(int, arguments=['alti'])
    newBattStat = pyqtSignal(str, str, arguments=['pct', 'time'])

    def __init__(self, port_name):
        super().__init__()
        self.state = "IDLE"
        # open the serial port
        self.serial = serial.Serial(port_name, 115200, timeout=0)
        self.loop_timer = QTimer()
        self.loop_timer.start(50)
        self.abs_lat = 0  # type: float
        self.abs_long = 0  # type: float
        self.abs_alt = 0  # type: float
        self.rel_lat = 0  # type: float
        self.rel_long = 0  # type: float
        self.rel_alt = 0  # type: float
        self.cur_lat = 0  # type: float
        self.cur_long = 0  # type: float
        self.cur_alt = 0  # type: float
        self.batt_stat = 0  # type: int
        self.serial_in = ""

    def connect_signals(self):
        self.changeState.connect(self.set_state)
        self.loop_timer.timeout.connect(self.timer_tick)

    @pyqtSlot(str, name="set_state")
    def set_state(self, state):
        print("set state to: " + state + "\n")
        self.serial.write((state + "\r\n").encode())

    def timer_tick(self):
        # Process any complete lines that came in over serial
        while self.serial.inWaiting() > 0:
            self.serial_in += self.serial.read().decode()
            if self.serial_in[-2:] == '\r\n':
                self.recv_serial_msg(self.serial_in[:-2])
                self.serial_in = ""

    def recv_serial_msg(self, text):
        print("Received message:", text)
        if 'relLat' in text:
            self.rel_lat = float(text.lstrip('relLat'))
        elif 'relLong' in text:
            self.rel_long = float(text.lstrip('relLong'))
            self.signal_new_rel_coordinate()
        elif 'relAlt' in text:
            self.rel_alt = float(text.lstrip('relAlt'))
            if self.state != 'LANDED':
                self.signal_new_alt()
        elif 'battVolt' in text:
            self.batt_stat = int(float(text.lstrip('battVolt')))
            self.signal_new_batt_stat()
        elif 'absLat' in text:
            self.abs_lat = float(text.lstrip('absLat'))
        elif 'absLong' in text:
            self.abs_long = float(text.lstrip('absLong'))
        elif 'absAlt' in text:
            self.abs_alt = float(text.lstrip('absAlt'))
            self.signal_new_abs_coordinate()
        elif 'STATECHANGE:' in text:
            state = text.lstrip('STATECHANGE')
            state = state.lstrip(':')
            print("strip: " + state + "\n")
            self.state = state
            self.signal_state_changed(state)

    def signal_new_abs_coordinate(self):
        print("new abs undiv coordinate is: " + str(self.abs_lat), ", ", str(self.abs_long), ", ", str(self.abs_alt))
        self.abs_lat = self.abs_lat / 10000000
        self.abs_long = self.abs_long / 10000000
        int_alt = int(self.abs_alt)
        print("new abs div coordinate is: " + str(self.abs_lat), ", ", str(self.abs_long), ", ", str(int_alt))
        self.newAbsCoordinate.emit(self.abs_lat, self.abs_long, int_alt)

    def signal_new_rel_coordinate(self):
        print("new rel undiv coordinate is: " + str(self.rel_lat), ", ", str(self.rel_long))
        self.rel_lat = self.rel_lat / 10000000
        self.rel_long = self.rel_long / 10000000
        self.cur_lat = self.abs_lat + self.rel_lat
        self.cur_long = self.abs_long + self.rel_long
        self.newRelCoordinate.emit(self.cur_lat, self.cur_long)

    def signal_new_alt(self):
        print("new alt is: " + str(self.rel_alt))
        int_alt = int(self.rel_alt)
        self.cur_alt = int(self.abs_alt) + int_alt
        self.newAlt.emit(self.cur_alt)

    def signal_new_batt_stat(self):
        print("new batt_stat is: " + str(self.batt_stat))
        pct = self.batt_stat / 15.0 * 100
        pct_str = "{0:.2f}".format(pct)
        rem_time = pct / 100 * 5
        time_str = "{0:.2f}".format(rem_time)
        self.newBattStat.emit(pct_str, time_str)

    def signal_state_changed(self, state):
        print("state changed to: " + state + "\n")
        self.stateChanged.emit(state)


app = QApplication([])
view = QQuickView()
view.setWidth(1024)
view.setHeight(720)
view.setTitle('Telemetry Tracker')
view.setResizeMode(QQuickView.SizeRootObjectToView)
url = QUrl('interface.qml')
# gui = Gui(sys.argv[1])
gui = Gui("COM22")
gui.connect_signals()
view.rootContext().setContextProperty('gui', gui)
view.setSource(url)
view.show()
qml_window = view.rootObject()
app.exec_()
