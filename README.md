# Caution
**This is not ready for regular users.**

# What's Sherpa?
[GnuPG](https://www.gnupg.org) is a great privacy tool, but
it's a lot harder than it should be to set it up on multiple
computers.  Not only are there some files you shouldn't copy
(``random_seed``), but some important files were renamed
between GnuPG versions (``secring.gpg`` became a subdirectory,
``private-keys-v1.d``, with a host of files beneath it).  

The GnuPG developers have done a
pretty good job of making the 2.0 to 2.1 migration painless,
but what if you want to go the other way?  What if you're using
2.1 on your desktop and you need to copy that to 1.4 on your
server?

Sherpa can help you in two ways.  First, it can back up the
files you need -- and not the ones you don't! -- to a
plain-vanilla ZIP archive.  Second, given a Sherpa archive it
can automagically restore that data to a new GnuPG installation.
This includes doing things like re-importing your certificates if
you're going from 1.4/2.0 to 2.1, and vice-versa.

# How do I get it?
Windows users (both 32- and 64-bit) can download an [MSI installer](https://github.com/rjhansen/sherpa/releases/download/0.3.0/sherpa-0.3.0.msi).  It's a little crude, but it works.

Fedora 25 users on x86_64 can
[download an RPM](http://sixdemonbag.org/sherpa-0.3.0-1.x86_64.rpm), or even
[an SRPM](http://sixdemonbag.org/sherpa-0.3.0-1.src.rpm).  The RPMs are
signed with my personal GnuPG certificate, 0x1DCBDC01B44427C7, which can be
found on the keyserver network.

Other users will need to compile from source.  For that, please see the end
of this page.

# How do I use it?

**You must have installed GnuPG and started it once.**  Sherpa depends on a
GnuPG profile directory existing.  If it doesn't find one, it will abort.

Start by launching Sherpa.  You'll see a combobox offering two choices, back up
and restore.  Choose which operation you want and then click the "Choose File"
button.  For backing up, this will choose the file to save to; for
restoring, this will choose the file to restore from.

Once you've selected your file, click "Go" and wait a few seconds.  Depending
on the size of your keyring this could take a little time.

Bang!  You're done.

# How is it licensed?
Sherpa is free software distributed under terms of the ISC License.

# How do I build it from source?

## Windows

### Prerequisites
You will need:

  1. Microsoft Visual C++ (the Community Edition works well).  It should
     also work fine with mingw's GCC, but I haven't tried it.
  2. Qt 5.7 compiled with the same compiler you'll be using.  (If you're
     using MSVC, use the MSVC Qt, etc.)
  3. GnuPG.  Any recent version should work, whether 1.4, 2.0, or 2.1.
  4. zlib and minizip, again compiled with the correct compiler.  If you're
     having trouble finding an MSVC-compiled zlib,
     [I'm hosting a copy](http://sixdemonbag.org/zlib-1.2.8.zip).
  5. WiX
  6. Python 3

### Prep
Open an Administrator command prompt.  ``cd`` into your GnuPG lib directory
(usually ``C:\Program Files (x86)\GnuPG\lib``).  In here you'll see three
files, ``libassuan.imp``, ``libgpg-error.imp``, and ``libgpgme.imp``.

GNU likes to call things ".imp" files, but MSVC expects them to be named ".lib"
files.  So for each file, copy it to the same name but with a .lib extension,
like so:

```shell
> copy libassuan.imp libassuan.lib
> copy libgpg-error.imp libgpg-error.lib
> copy libgpgme.imp libgpgme.lib
```

Relax, you're not going to affect your GnuPG experience at all -- these files
are only used when compiling code, not when running code.

Now that you have that taken care of, uncompress the Sherpa source code
somewhere convenient.  Open the file ``builders\mkwin32.py`` in your
Python editor-of-choice and look at the top of the file.  There'll be a
comment reading, "User-serviceable parts here".  Edit these paths so they
point to correct directories for your own system.  Also, unless you have a
code signing certificate, set ``shouldSign`` to ``False``.

### Build
Run ``mkwin32.py``.  You'll have a bouncing baby .MSI installer at the end of
it.

## UNIX

### Prerequisites
You will need:

  1.  A good C++ compiler (GCC 5.0 or later, or any recent Clang++).
  2.  Qt 5.7 and development headers
  3.  gpgme, libassuan, and libgpg-error development headers
  4.  zlib and minizip, with development headers

For instance, on Fedora 25 this can be done with

```shell
$ sudo dnf install gcc-c++ qt5 qt5-devel gpgme gpgme-devel\
libassuan libassuan-devel libgpg-error libgpg-error-devel\
minizip minizip-devel
```

Uncompress the Sherpa source code someplace convenient.  Open a terminal
and ``cd`` into that directory.  At that point it's

```shell
$ qmake-qt5 sherpa.pro
$ make
```

... and you should have a bouncing baby app at the end.

(Note: Fedora calls Qt 5's ``qmake`` ``qmake-qt5``.  I don't know what
other distros call their versions of Qt 5's ``qmake``.)

## Mac OS X

Good luck, man.  Still working on this one myself.  I can *almost* get an
app bundle to work, but something's still going haywire.
