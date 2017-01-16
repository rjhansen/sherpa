# Caution
This is not ready for regular users.  If you want to live
boldly and help me test it that would be great, but make
sure you have backups first!

# What's Sherpa?
[GnuPG](https://www.gnupg.org) is a great privacy tool, but
it's a lot harder than it should be to set it up on multiple
computers.  Not only are there some files you shouldn't copy
(``random_seed``), but some important files were renamed
between GnuPG versions.  The GnuPG developers have done a
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
Windows users can download an [MSI installer](https://github.com/rjhansen/sherpa/releases/download/0.3.0/sherpa-0.3.0.msi).  It's a little crude, but it works.

UNIX users need to compile from source.  You'll need a C++ compiler, Qt 5.7,
minizip, gpgme, libassuan, and libgpg-error installed, as well as the
development packages for each.

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

# Licensing
Sherpa is free software distributed under terms of the ISC License.
(c) 2017, Robert J. Hansen.
