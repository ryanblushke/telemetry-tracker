from PyQt5.QtCore import *
from PyQt5.QtGui import *

from PyQt5.QtWebEngineWidgets import *
from PyQt5.QtWidgets import QWidget, QVBoxLayout, QApplication


class Browser(QApplication):
    def __init__(self):
        QApplication.__init__(self, [])
        self.window = QWidget()

        self.web = QWebEngineView(self.window)

        self.web.setHtml(open("map.html").read())
        self.layout = QVBoxLayout(self.window)
        self.layout.addWidget(self.web)
        self.window.show()

import sys

if __name__ == "__main__":
    app = Browser()
    sys.exit(app.exec_())