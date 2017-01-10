cmake_minimum_required(VERSION 3.6)
project(Sherpa VERSION 0.4.0)

set(COMPANY "Robert Hansen")
set(COPYRIGHT "Copyright (c) 2017 Robert J. Hansen")
set(IDENTIFIER "org.sixdemonbag.Sherpa")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 14)
set(SHERPA_SRCS main.cpp mainwindow.cpp aboutdialog.cpp)
set(SHERPA_RCS sherpa.qrc)

find_package(Qt5Widgets REQUIRED)

set(GPGMELIB_HINTS "C:/Program Files (x86)/GnuPG/lib" /usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64 /Users/rjh/lib)
set(MZLIB_HINTS "C:/zlib/lib" /usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64 /usr/local/Cellar/minizip/1.1/lib)
set(GPGME_INCLUDE_HINTS "C:/Program Files (x86)/GnuPG/include" /usr/include /usr/local/include /Users/rjh/include)
set(MZ_INCLUDE_HINTS "C:/zlib/include")

if (WIN32)
  set(GPGME_LIB_NAME libgpgme)
  set(ASSUAN_LIB_NAME libassuan)
  set(GPGERR_LIB_NAME libgpg-error)
endif()
if (UNIX)
  set(GPGME_LIB_NAME gpgme)
  set(ASSUAN_LIB_NAME assuan)
  set(GPGERR_LIB_NAME gpg-error)
endif()

find_path(MINIZIP_INCLUDE_DIR zip.h HINTS ${MZ_INCLUDE_HINTS} PATH_SUFFIXES minizip)
find_path(GNUPG_INCLUDE_DIR gpgme.h HINTS ${GPGME_INCLUDE_HINTS}/include)
find_library(MINIZIP_LIBRARY minizip HINTS ${MZLIB_HINTS})

find_library(GPGME_LIBRARY ${GPGME_LIB_NAME} HINTS ${GPGMELIB_HINTS})
find_library(ASSUAN_LIBRARY ${ASSUAN_LIB_NAME} HINTS ${GPGMELIB_HINTS})
find_library(GPGERR_LIBRARY ${GPGERR_LIB_NAME} HINTS ${GPGMELIB_HINTS})
find_library(MINIZIP_LIBRARY minizip HINTS ${MZLIB_HINTS})

qt5_add_resources(SHERPA_GEN_RCS ${SHERPA_RCS})

if (APPLE)
  set(MACOSX_BUNDLE_ICON_FILE Sherpa.icns) 
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/Sherpa.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  set(SHERPA_SRCS ${SHERPA_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/Sherpa.icns)
endif(APPLE)

add_executable(Sherpa MACOSX_BUNDLE WIN32 ${SHERPA_SRCS} ${SHERPA_GEN_RCS})
target_link_libraries(Sherpa Qt5::Widgets ${GPGME_LIBRARY} ${ASSUAN_LIBRARY} ${GPGERR_LIBRARY} ${MINIZIP_LIBRARY})

set(plugin_dest_dir bin)
set(qtconf_dest_dir bin)
set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/Sherpa")
if (APPLE)
  set(plugin_dest_dir Sherpa.app/Contents/MacOS)
  set(qtconf_dest_dir Sherpa.app/Contents/Resources)
  set(APPS "\${CMAKE_INSTALL_PREFIX}/Sherpa.app")
elseif (WIN32)
  set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/Sherpa.exe")
endif()

install(TARGETS Sherpa BUNDLE DESTINATION . COMPONENT Runtime RUNTIME DESTINATION bin COMPONENT Runtime)

#--------------------------------------------------------------------------------
# Install needed Qt plugins by copying directories from the qt installation
# One can cull what gets copied by using 'REGEX "..." EXCLUDE'
install(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime)

install(CODE "
    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"\")
    " COMPONENT Runtime)

set(DIRS ${QT_LIBRARY_DIRS})

install(CODE "
    file(GLOB_RECURSE QTPLUGINS
      \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"\${QTPLUGINS}\" \"${DIRS}\")
    " COMPONENT Runtime)

# To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
# where CPackConfig.cmake is created by including CPack
# And then there's ways to customize this as well
set(CPACK_BINARY_DRAGNDROP ON)
include(CPack)

