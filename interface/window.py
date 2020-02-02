# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file '.\main_window.ui'
#
# Created by: PyQt5 UI code generator 5.13.0
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(800, 600)
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.button_launch_rover_process = QtWidgets.QPushButton(self.centralwidget)
        self.button_launch_rover_process.setGeometry(QtCore.QRect(30, 20, 131, 23))
        self.button_launch_rover_process.setObjectName("button_launch_rover_process")
        self.label_ssh_connnection_status = QtWidgets.QLabel(self.centralwidget)
        self.label_ssh_connnection_status.setGeometry(QtCore.QRect(170, 20, 231, 16))
        self.label_ssh_connnection_status.setObjectName("label_ssh_connnection_status")
        self.label_software_connnection_status = QtWidgets.QLabel(self.centralwidget)
        self.label_software_connnection_status.setGeometry(QtCore.QRect(170, 50, 231, 16))
        self.label_software_connnection_status.setObjectName("label_software_connnection_status")
        self.button_reduced_comms = QtWidgets.QPushButton(self.centralwidget)
        self.button_reduced_comms.setGeometry(QtCore.QRect(30, 50, 131, 23))
        self.button_reduced_comms.setObjectName("button_reduced_comms")
        self.label_drive_status = QtWidgets.QLabel(self.centralwidget)
        self.label_drive_status.setGeometry(QtCore.QRect(30, 80, 281, 16))
        self.label_drive_status.setObjectName("label_drive_status")
        MainWindow.setCentralWidget(self.centralwidget)

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "RoverControl"))
        self.button_launch_rover_process.setText(_translate("MainWindow", "Launch Rover Process"))
        self.label_ssh_connnection_status.setText(_translate("MainWindow", "SSH Connection Status:"))
        self.label_software_connnection_status.setText(_translate("MainWindow", "Software Connection Status:"))
        self.button_reduced_comms.setText(_translate("MainWindow", "Enable Reduced Comms"))
        self.label_drive_status.setText(_translate("MainWindow", "Drive Status:"))
