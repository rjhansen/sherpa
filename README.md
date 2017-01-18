# Caution
**This is not ready for regular users.**

# What's Sherpa?
[GnuPG](https://www.gnupg.org) is a great privacy tool, but it's a lot harder than it should be to set it up on multiple computers.  Not only are there some files you shouldn't copy (``random_seed``), but some important files were renamed between GnuPG versions (``secring.gpg`` became a subdirectory, ``private-keys-v1.d``, with a host of files beneath it).  

The GnuPG developers have done a pretty good job of making the 2.0 to 2.1 migration painless, but what if you want to go the other way?  What if you're using 2.1 on your desktop and you need to copy that to 1.4 on your server?

Sherpa can help you in two ways.  First, it can back up the files you need -- and not the ones you don't! -- to a plain-vanilla ZIP archive.  Second, given a Sherpa archive it can automagically restore that data to a new GnuPG installation. This includes doing things like re-importing your certificates if you're going from 1.4/2.0 to 2.1, and vice-versa.

# How do I get it?
Windows users (both 32- and 64-bit) can download an [MSI installer](https://github.com/rjhansen/sherpa/releases/download/0.4.0/sherpa-0.4.0.msi).  It's a little crude, but it works.

Fedora 25 users on x86_64 can [download an RPM](https://github.com/rjhansen/sherpa/releases/download/0.4.0/sherpa-0.4.0-1.x86_64.rpm).  The RPM is signed with my personal GnuPG certificate, 0x1DCBDC01B44427C7, which can be found on the keyserver network.

macOS 10.12 and later users can download a [disk image](https://github.com/rjhansen/sherpa/releases/download/0.4.0/Sherpa.0.4.0.dmg).

Other users will need to compile from [source](https://github.com/rjhansen/sherpa/archive/0.4.0.tar.gz).  Please see the compilation instructions at the end of this README.

# How do I use it?

**You must have installed GnuPG and started it once.**  Sherpa depends on a GnuPG profile directory existing.  If it doesn't find one, it will abort.

Start by launching Sherpa.  You'll see a combobox offering two choices, back up and restore.  Choose which operation you want and then click the "Choose File" button.  For backing up, this will choose the file to save to; for restoring, this will choose the file to restore from.

Once you've selected your file, click "Go" and wait a few seconds.  Depending on the size of your keyring this could take a little time.

Bang!  You're done.

# Is it free and/or open-source?
Yes.  Please see the LICENSE file for details.

# How do I build it from source?

## Windows

### Prerequisites
You will need:

  1. [Microsoft Visual Studio 2015](https://go.microsoft.com/fwlink/?LinkId=691978&clcid=0x409).
  2. The [Microsoft Windows 10 SDK](https://go.microsoft.com/fwlink/?linkid=838916).
  2. [Qt 5.7 for MSVC](http://download.qt.io/official_releases/qt/5.7/5.7.1/qt-opensource-windows-x86-msvc2015-5.7.1.exe).
  3. [GnuPG](https://gnupg.org/ftp/gcrypt/binary/gnupg-w32-2.1.17_20161220.exe).
  4. [zlib and minizip](http://sixdemonbag.org/zlib-1.2.8.zip).
  5. [WiX](https://wix.codeplex.com/downloads/get/1587179).
  6. [Python 3](https://www.python.org/ftp/python/3.6.0/python-3.6.0-amd64.exe).

### Prep
Open an Administrator command prompt.  ``cd`` into your GnuPG lib directory (usually ``C:\Program Files (x86)\GnuPG\lib``).  In here you'll see three files, ``libassuan.imp``, ``libgpg-error.imp``, and ``libgpgme.imp``.

GNU likes to call things ".imp" files, but MSVC expects them to be named ".lib" files.  So for each file, copy it to the same name but with a .lib extension, like so:

```shell
> copy libassuan.imp libassuan.lib
> copy libgpg-error.imp libgpg-error.lib
> copy libgpgme.imp libgpgme.lib
```

Relax, you're not going to affect your GnuPG experience at all -- these files are only used when compiling code, not when running code.

Now that you have that taken care of, uncompress the Sherpa source code somewhere convenient.  Open the file ``builders\mkwin32.py`` in your Python editor-of-choice and look at the top of the file.  There'll be a comment reading, "User-serviceable parts here".  Edit these paths so they point to correct directories for your own system.  Also, unless you have a code signing certificate, set ``shouldSign`` to ``False``.

Next, edit the ``sherpa.pro`` file.  Adjust the file paths in the Windows target appropriately to reflect your local installation dirs.

### Build
From the top of the sherpa directory:

```shell
> \Qt5.7.1\5.7\msvc\bin\qmake
> nmake msi
```

… and you'll have an MSI installer you've made yourself.

## UNIX

### Prerequisites
You will need:

  1.  A good C++ compiler (GCC 5.0 or later, or any recent Clang++).
  2.  Qt 5.7 and development headers
  3.  GnuPG
  4.  gpgme 1.6 or later
  5.  Recent zlib and minizip, with development headers

For instance, on Fedora 25 this can be done with

```shell
$ sudo dnf install gcc-c++ qt5 qt5-devel gpgme gpgme-devel\
libassuan libassuan-devel libgpg-error libgpg-error-devel\
minizip minizip-devel gnupg2
```

Uncompress the Sherpa source code someplace convenient.  Open a terminal and ``cd`` into that directory.  At that point it's

```shell
$ qmake-qt5 sherpa.pro
$ make
```

… and you should have a bouncing baby app at the end.  Fedora users can also try ``make f25rpm``, which will make a Fedora 25 RPM.

(Note: Fedora calls Qt 5's ``qmake`` ``qmake-qt5``.  I don't know what other distros call their versions of Qt 5's ``qmake``.)

## Mac OS X

### Prerequisites
You will need:

  1. XCode and command-line development tools.
  2. [GnuPG](https://downloads.sourceforge.net/project/gpgosx/GnuPG-2.1.17-003.dmg?r=https%3A%2F%2Fsourceforge.net%2Fprojects%2Fgpgosx%2Ffiles%2F%3Fsource%3Dnavbar&ts=1484706380&use_mirror=superb-sea2).
  3. [Qt 5.7](http://download.qt.io/official_releases/qt/5.7/5.7.1/qt-opensource-mac-x64-clang-5.7.1.dmg).
  4. gpgme 1.6 or later
  5. Recent zlib and minizip

gpgme and minizip can be found in [Homebrew](http://brew.sh/).

Uncompress the Sherpa source code someplace convenient.  Open a terminal and ``cd`` into that directory.  Open the ``sherpa.pro`` file and adjust paths appropriately inside the macOS targets.

```shell
$ /path/to/qmake sherpa.pro
$ make
$ /path/to/macdeployqt sherpa.app
```

You now have a macOS application you can launch in Finder.
