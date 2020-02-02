#!/usr/bin/python3
import sys
import os
from time import sleep
from PyQt5 import QtCore, QtWidgets, QtGui
import subprocess
import paramiko
from functools import wraps
import traceback
from math import sin, cos, pi

# root_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../"))
# if root_path not in sys.path: sys.path.append(root_path)


from map_manager import MapManager

map_name = "airbnb"


def error_trap(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        try:
            func(*args, **kwargs)
        except Exception:
            traceback.print_exc()
    return wrapper


class Main(QtWidgets.QMainWindow):
    def __init__(self, request_function):
        super().__init__()
        self.request = request_function  # This is the request function inherited from device
        self.map_manager = MapManager(map_name)
        self.plotted_coords = {} # name: (lat, lng)

        self.main_grid = QtWidgets.QGridLayout()

        main_grid = self.main_grid

        main_widget = QtWidgets.QWidget()
        main_widget.setLayout(main_grid)
        self.setCentralWidget(main_widget)

        self.timer = QtCore.QTimer()
        self.timer.setInterval(100)
        self.timer.start()
        self.timer.timeout.connect(self.update)

        self.zoom_slider = QtWidgets.QSlider(QtCore.Qt.Horizontal)
        self.main_grid.addWidget(self.zoom_slider, 0, 0)
        self.zoom_slider.setMinimum(0)
        self.zoom_slider.setMaximum(100)
        # self.zoom_slider.SliderValueChange.connect(self.update)
        self.max_zoom = 5
        self.min_zoom = 1

        button_reload_coords = QtWidgets.QPushButton("Reload Saved Coords")
        self.main_grid.addWidget(button_reload_coords, 1, 0)

        button_reload_coords.clicked.connect(self.update_config)

        self.image = QtGui.QPixmap(self.map_manager.map_filename)

        self.update_config()


        self.setGeometry(300, 300, 300, 220)
        self.setWindowTitle('GPS UI')
        self.show()

    @error_trap
    def paintEvent(self, event):
        """Redraw display"""
        success, data = self.request("position")
        if not success:
            print("Failed to contact rover")
            return
        print(data)
        rover_lat, rover_lng, rover_bearing, _ = data
        # rover_lat, rover_lng, rover_bearing, _ = 52.132604, -106.627935, 0, None  # Manhole cover behind hardy lab
        rover_x, rover_y = self.map_manager.get_pixel_from_coords(rover_lat, rover_lng)
        print("Rover", rover_lat, rover_lng)
        zoom = self.zoom_slider.value() / self.zoom_slider.maximum() * (self.max_zoom - self.min_zoom) + self.min_zoom
        painter = QtGui.QPainter(self)
        offset_x = (rover_x * zoom - self.width() / 2) / zoom
        offset_y = (rover_y * zoom - self.height() / 2) / zoom
        painter.drawPixmap(0, 0, self.width() * zoom, self.height() * zoom, self.image, offset_x, offset_y, self.width(), self.height())
        self.draw_point(painter, self.width() / 2, self.height() / 2, rover_bearing)

        top_left_x = self.width() / 2 - rover_x * zoom
        top_left_y = self.height() / 2 - rover_y / 2 * zoom
        for name, (lat, lng) in self.plotted_coords.items():
            print(lat, lng)
            point_x, point_y = self.map_manager.get_pixel_from_coords(lat, lng)
            point_x = top_left_x + point_x * zoom
            point_y = top_left_y + point_y * zoom
            self.draw_point(painter, point_x, point_y)
            painter.drawText(point_x + 2, point_y - 5, name)

    def draw_point(self, painter, x, y, direction=None):
        print(x, y)
        r = 6
        pen = QtGui.QPen(QtCore.Qt.red)
        pen.setWidth(3)
        painter.setPen(pen)
        painter.setBrush(QtGui.QBrush(QtCore.Qt.SolidPattern))
        painter.drawEllipse(x - r / 2, y - r / 2, r, r)
        painter.setBrush(QtGui.QBrush(QtCore.Qt.NoBrush))
        # if direction is not None:
        #     r_line = 10
        #     line_x = r_line * cos(direction * pi / 180)
        #     line_y = r_line * sin(direction * pi / 180)
        #     pen = QtGui.QPen(QtCore.Qt.orange)
        #     pen.setWidth(3)
        #     painter.setPen(pen)
        #     painter.drawLine(x, y, x + line_x, y + line_y)
        #     painter.setPen(QtGui.QPen(QtCore.Qt.orange))


    def update_config(self):
        """Reload saved coord points from file"""
        self.plotted_coords = {}
        for line in open(os.path.join(root_path, "processes/navigation/plotted_coords.txt")):
            line = line.strip()
            if not line or line[0] == "#":
                continue
            parts = line.split(",")
            if len(parts) != 3:
                continue
            name = parts[0]
            lat, lng = [float(x) for x in parts[1:]]
            self.plotted_coords[name] = (lat, lng)


    def take_pic(self):
        """takes a screenshot of the map"""


if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    window = Main()
    window.show()
    app.exec_()
