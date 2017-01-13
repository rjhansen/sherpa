QT       += core gui widgets
CONFIG   += c++14
TARGET    = Sherpa
TEMPLATE  = app
DEFINES  += FULL_VERSION=\\\"0.3.0\\\" SHORT_VERSION=\\\"0.3\\\"

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
    LIBS += -llibgpgme -llibgpg-error -llibassuan -lminizip -lzdll -laes -ladvapi32
    libgpgme.path = "C:\Program Files (x86)\GnuPG\lib\libgpgme-11.dll"
    libassuan.path = "C:\Program Files (x86)\GnuPG\lib\libassuan-0.dll"
    libgpgerror.path = "C:\Program Files (x86)\GnuPG\lib\libgpg-error-0.dll"
    zlib.path = "C:\zlib\zlib1.dll"
    INSTALLS += libgpgme libassuan libgpgerror zlib
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

RESOURCES  = sherpa.qrc

SOURCES   += main.cpp\
             mainwindow.cpp \
             aboutdialog.cpp

HEADERS   += mainwindow.h \
             aboutdialog.h

FORMS     += mainwindow.ui \
             aboutdialog.ui

DISTFILES += LICENSE \
             sherpa.jpg
