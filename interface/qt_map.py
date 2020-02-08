from PySide2.QtWidgets import QApplication
from PySide2.QtQml import QQmlApplicationEngine
from PySide2.QtCore import QUrl

app = QApplication([])
engine = QQmlApplicationEngine()
url = QUrl("view.qml")

engine.load(url)
app.exec_()
