######################################################################
# Automatically generated by qmake (3.1) Sun Apr 21 21:25:44 2024
######################################################################

TEMPLATE = app
TARGET = Client
INCLUDEPATH += .
QT += core gui network widgets

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += ui_QtDesigner.h \
           Chess/board.h \
           Chess/game.h \
           Chess/piece.h \
           Qt/GameWidget.h \
           Qt/MainWindow.h

FORMS += QtDesigner.ui

SOURCES += main.cpp \
           Chess/board.cpp \
           Chess/game.cpp \
           Chess/piece.cpp \
           Qt/GameWidget.cpp \
           Qt/MainWindow.cpp
