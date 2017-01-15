QT       += core gui widgets
CONFIG   += c++14 release
TARGET    = sherpa
TEMPLATE  = app
DEFINES  += FULL_VERSION=\\\"0.3.0\\\"

# Assumes you're using a 32-bit MSVC build of Qt.
#
# Rename the GnuPG-provided libgpgme.imp, libgpg-error.imp,
# libassuan.imp, to have .lib extensions for use with MSVC.
#
# A distribution of zlib binaries with minizip included can
# be downloaded from http://sixdemonbag.org/zlib-1.2.8.zip.

win32 {
    CONFIG += windows
    INCLUDEPATH += "C:\Program Files (x86)\GnuPG\include" "C:\zlib\include"
    LIBS += -L"C:\Program Files (x86)\GnuPG\lib" -L"C:\zlib\lib"
    LIBS += -llibgpgme -llibgpg-error -llibassuan -lminizip -lzdll -laes -ladvapi32
}

unix {
    LIBS += -lminizip -lgpgme -lassuan -lgpg-error
}

# Using Homebrew to provide minizip, as well as my own
# homebuilt gpgme.  Use macdeployqt to automagically
# package all the necessary libs into the Sherpa bundle.
osx {
    INCLUDEPATH += /usr/local/Cellar/minizip/1.1/include /Users/rjh/include
    LIBS += -L/usr/local/Cellar/minizip/1.1/lib -L/Users/rjh/lib
    LIBS += -lminizip -lgpgme -lassuan -lgpg-error
}

RESOURCES  = src/sherpa.qrc

SOURCES   += src/main.cpp\
             src/mainwindow.cpp \
             src/aboutdialog.cpp

HEADERS   += src/mainwindow.h \
             src/aboutdialog.h

FORMS     += src/mainwindow.ui \
             src/aboutdialog.ui

DISTFILES += LICENSE \
             src/sherpa.jpg
