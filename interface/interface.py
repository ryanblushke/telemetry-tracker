from PyQt5.QtCore import QObject, QUrl, pyqtSignal, pyqtSlot
from PyQt5.QtGui import QGuiApplication
from PyQt5.QtQuick import QQuickView

class Gui(QObject):

    # Define a new signal called 'trigger' that has no arguments.
    trigger = pyqtSignal()

    def handle_trigger(self):
        # Show that the slot has been called.

        print("trigger signal received")



app = QGuiApplication([])
view = QQuickView()
view.setWidth(1024)
view.setHeight(720)
view.setTitle('Hello PyQt')
view.setResizeMode(QQuickView.SizeRootObjectToView)
url = QUrl("interface.qml")
gui = Gui()
view.rootContext().setContextProperty('gui', gui)
view.setSource(url)
view.show()
qml_window = view.rootObject()
qml_window.clicked.connect(gui.handle_trigger)
gui.trigger.emit()
app.exec_()
