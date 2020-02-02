#!/usr/bin/python3
import sys
import os
from time import sleep, time
from PyQt5 import QtCore, QtWidgets


from window import Ui_MainWindow


class Main(QtWidgets.QMainWindow, Ui_MainWindow):
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.button_launch_rover_process.clicked.connect(self.button_launch_rover_process_clicked)
        self.button_reduced_comms.clicked.connect(self.button_reduced_comms_clicked)
        self.update_timer = QtCore.QTimer()
        self.update_timer.timeout.connect(self.update_timer_tick)
        self.update_timer.start(200)

    def update_timer_tick(self):
        pass

    def button_launch_rover_process_clicked(self):
        pass

    def button_reduced_comms_clicked(self):
        pass


if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    window = Main()
    window.show()
    app.exec_()
