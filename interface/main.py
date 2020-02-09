#!/usr/bin/python3
import sys
import os
from time import sleep, time
from queue import Queue
from PyQt5 import QtCore, QtWidgets, QtGui, uic
from PyQt5 import QtWebEngineWidgets  # Not used, but nessisary to import so the map works


class Main(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        uic.loadUi("window.ui", self)  # Loading directly from here, as opposed to generating a .py file first (both ways work fine)
        self.button_launch_rover_process.clicked.connect(self.button_launch_rover_process_clicked)
        self.button_reduced_comms.clicked.connect(self.button_reduced_comms_clicked)
        self.update_timer = QtCore.QTimer()
        self.update_timer.timeout.connect(self.update_timer_tick)
        self.update_timer.start(200)

        html = open("map.html").read()
        leaflet_js = open("leaflet.js").read()
        html = html.replace("{{{leaflet_js}}}", leaflet_js)
        leaflet_css = open("leaflet.css").read()
        html = html.replace("{{{leaflet_css}}}", leaflet_css)
        self.map_widget.setHtml(html)
        self.map_widget.loadFinished.connect(self.on_load_finished)

        self.value = Queue()

    def on_load_finished(self):
        def callback(x):
            self.value.put(x)  # Javascript code is processed asynchronously, so we need a callback for the return value
        self.map_widget.page().runJavaScript("drawPoint(51.5, -0.09)", callback)
        # We can't wait for the return value here, or it locks everything up

    def update_timer_tick(self):
        if not self.value.empty():
            print(self.value.get())

    def button_launch_rover_process_clicked(self):
        pass

    def button_reduced_comms_clicked(self):
        pass


if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    window = Main()
    window.show()
    app.exec_()
