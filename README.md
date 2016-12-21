# sherpa
A GnuPG profile migrator with native look-and-feel for Win32, Linux,
and macOS.

## Windows building

The Sherpa.pro file is set up to use paths on my system.
Adjust these to use correct paths for yours.

On Windows you'll need zlib-1.2.8 binaries, available from
[sixdemonbag.org](http://sixdemonbag.org/zlib-1.2.8.zip).

You'll also need GnuPG 2.1 for Windows.  GnuPG ships with
the GnuPG development libraries named X.imp; MSVC requires
these to be named X.lib.  The easiest way to do this is to
open an Administrator command prompt and copy X.imp -> X.lib.

Once you have this built, you will need to place the 
executable in the same dir with:

* gpgme-w32spawn.exe (from GnuPG's bin dir)
* libassuan-0.dll
* libgpg-error-0.dll
* libgpgme-11.dll
* Qt5Core.dll
* Qt5Gui.dll
* Qt5Widgets.dll
* Sherpa.exe

You should now be able to run Sherpa.

## macOS

I use a combination of Homebrew (for minizip) and my own
homebuilt gpgme.  Fix these paths in the Sherpa.pro file to
reflect your own build environment.  Once built, use 
macdeployqt to turn the entire thing into a proper 
self-contained app bundle.

## Linux 

It pretty much Just Works.
