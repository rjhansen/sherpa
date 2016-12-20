#-------------------------------------------------
#
# Project created by QtCreator 2016-12-03T16:56:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14
TARGET = Sherpa
TEMPLATE = app

win32 {
    CONFIG += windows
}

unix {
    LIBS += -lminizip -lgpgme -lassuan -lgpg-error
}

osx {
    INCLUDEPATH += /usr/local/Cellar/minizip/1.1/include
    INCLUDEPATH += /Users/rjh/include
    LIBS += -L/usr/local/Cellar/minizip/1.1/lib
    LIBS += -L/Users/rjh/lib
    LIBS += -lminizip -lgpgme -lassuan -lgpg-error
}

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
