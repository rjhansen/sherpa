QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14
TARGET = Sherpa
TEMPLATE = app

# Assumes you're using a 32-bit MSVC build of Qt.
#
# Rename the GnuPG-provided libgpgme.imp, libgpg-error.imp,
# libassuan.imp, to have .lib extensions for use with MSVC.
#
# A distribution of zlib binaries with minizip included can
# be downloaded from http://sixdemonbag.org/zlib-1.2.8.zip.

win32 {
    CONFIG += windows
    INCLUDEPATH += "C:\Program Files (x86)\GnuPG\include"
    INCLUDEPATH += "C:\zlib\include"
    LIBS += -L"C:\Program Files (x86)\GnuPG\lib"
    LIBS += -L"C:\zlib\lib"
    LIBS += -llibgpgme -llibgpg-error -llibassuan -lminizip-static -lzlib-static -ladvapi32
}

unix {
    LIBS += -lminizip -lgpgme -lassuan -lgpg-error
}

# Using Homebrew to provide minizip, as well as my own
# homebuilt gpgme.  Use macdeployqt to automagically
# package all the necessary libs into the Sherpa bundle.
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

DISTFILES += \
    LICENSE
