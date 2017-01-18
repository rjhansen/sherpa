QT       += core gui widgets
CONFIG   += c++14 release
TARGET    = sherpa
TEMPLATE  = app
VERSION   = 0.4.0
DEFINES  += FULL_VERSION=\\\"$${VERSION}\\\"

# Assumes you're using a 32-bit MSVC build of Qt.
#
# Rename the GnuPG-provided libgpgme.imp, libgpg-error.imp,
# libassuan.imp, to have .lib extensions for use with MSVC.
#
# A distribution of zlib binaries with minizip included can
# be downloaded from http://sixdemonbag.org/zlib-1.2.8.zip.
#
# Note: unless you're me, don't use "nmake msi".  If you want
# to do this, you'll need to install Python 3.5 and edit the
# user-serviceable parts of mkwin32.py

win32 {
    CONFIG += windows
    INCLUDEPATH += "C:\Program Files (x86)\GnuPG\include" "C:\zlib\include"
    LIBS += -L"C:\Program Files (x86)\GnuPG\lib" -L"C:\zlib\lib"
    LIBS += -llibgpgme -llibgpg-error -llibassuan -lminizip -lzdll -laes -ladvapi32

    msi.commands += cd builders && mkwin32.py && cd ..
    QMAKE_EXTRA_TARGETS += msi
}

# Using Homebrew to provide minizip, as well as my own
# homebuilt gpgme.  Use macdeployqt to automagically
# package all the necessary libs into the Sherpa bundle.
#
# Note: unless you're me, don't use "make bundle".  Or at
# the very least, use your own signing certificate.

osx {
    INCLUDEPATH += /usr/local/Cellar/minizip/1.1/include
    LIBS += -L/usr/local/Cellar/minizip/1.1/lib

    bundle.commands += rm -f \"Sherpa $${VERSION}.dmg\" &&\
make clean &&\
make &&\
macdeployqt sherpa.app -always-overwrite && \
codesign -s \"Robert Hansen\" -i com.roberthansen.sherpa -f --deep sherpa.app && \
hdiutil create -fs HFS+ -volname \"Sherpa $${VERSION}\" -srcfolder sherpa.app \"Sherpa $${VERSION}.dmg\"

    QMAKE_EXTRA_TARGETS += bundle
}

# Note: if using this from Qt Creator, make sure to start
# Qt Creator from the command line so it can inherit your
# PATH and find gpgme-config.
unix | osx {
    QMAKE_CXXFLAGS += '$$system(gpgme-config --cflags)'
    QMAKE_LFLAGS += $$system(gpgme-config --libs)
    LIBS += -lminizip

    tarball.commands += ./clean &&\
rm -rf sherpa-$${VERSION} &&\
rm -f sherpa-$${VERSION}.tar.gz &&\
mkdir sherpa-$${VERSION} &&\
cp -r src sherpa-$${VERSION} &&\
cp -r builders sherpa-$${VERSION} &&\
cp clean LICENSE README.md sherpa.pro sherpa-$${VERSION} &&\
tar czf sherpa-$${VERSION}.tar.gz sherpa-$${VERSION} &&\
rm -rf sherpa-$${VERSION}
    QMAKE_EXTRA_TARGETS += tarball
}

# To build Fedora 25 RPMs.
unix:!osx {
    f25rpm.depends += tarball
    f25rpm.commands += mkdir -p ~/rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS} &&\
mv sherpa-$${VERSION}.tar.gz builders &&\
cd builders &&\
./mkf25spec.py &&\
fedpkg --release f25 mockbuild &&\
rm -f sherpa.spec &&\
rpm --resign *.rpm &&\
cd .. &&\
mv builders/results_sherpa/$${VERSION}/1/*.rpm . &&\
rm -rf sherpa-$${VERSION}-1.src.rpm builders/sherpa.spec builders/*gz builders/*rpm builders/results_sherpa
    QMAKE_EXTRA_TARGETS += f25rpm
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
