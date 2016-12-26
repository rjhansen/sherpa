#!/usr/bin/python3
# coding=UTF-8

import os
import os.path
import shutil
import subprocess
import re
import sys

# User-serviceable parts here
qtdir = r"C:\Qt\Qt5.7.1\5.7\msvc2015\bin\\"
vc14dir = r"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\\"
w32sdkdir = r"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Bin\\"
wixdir = r"C:\Program Files (x86)\WiX Toolset v3.10\bin\\"
gnupgdir = r"C:\Program Files (x86)\GnuPG\bin\\"
msvcrtfile = r"C:\Program Files (x86)\Common Files\Merge Modules\\" +\
    "Microsoft_VC140_CRT_x86.msm"
zlib = r"C:\zlib\zlib1.dll"
# End user-serviceable parts

delfiles = ["Makefile", "Makefile.Debug", "Makefile.Release",
    "sherpa.wixobj", "sherpa.wixpdb", "sherpa.wxs"]
deldirs = ["build", "debug", "release"]

[os.unlink(Y) for Y in [X for X in os.listdir() if re.search(r"\.msi$", X)]]

for fn in delfiles:
    if os.path.isfile(fn):
        os.unlink(fn)

for dirname in deldirs:
    if os.path.isdir(dirname):
        shutil.rmtree(dirname)

version_re = re.compile(r"^\d+(\.\d+){1,2}$")
print("Sherpa version tag: ", end="")
version = input()
while not version_re.match(version):
    print("Bad tag.  New tag: ", end="")
    version = input()

with open("sherpa_template.wxs", encoding="Windows-1252") as fh:
    template = fh.read()
with open("sherpa.wxs", "w", encoding="Windows-1252") as fh:
    print(re.sub(r"\$VERSION", version, template), file=fh)

qmakecmd = [qtdir + "qmake.exe", "Sherpa.pro"]
buildcmd = [vc14dir + "nmake.exe"]
signcmd = [w32sdkdir + "signtool.exe", "sign",
    "/tr", "http://timestamp.digicert.com", "/td", "sha256", "/fd",
    "sha256", "/a", r"build\Sherpa.exe"]
deploycmd = [qtdir + "windeployqt.exe", r"build\sherpa.exe"]
candlecmd = [wixdir + "candle.exe", "sherpa.wxs"]
lightcmd = [wixdir + "light.exe", "-ext", "WixUIExtension", "sherpa.wixobj"]
signmsi = [w32sdkdir + "signtool.exe", "sign",
    "/tr", "http://timestamp.digicert.com", "/td", "sha256", "/fd",
    "sha256", "/a", r"Sherpa.msi"]

subprocess.Popen(qmakecmd).wait()
subprocess.Popen(buildcmd).wait()

for filename in ["gpgme-w32spawn.exe", "libassuan-0.dll",
    "libgpg-error-0.dll", "libgpgme-11.dll"]:
    shutil.copy(gnupgdir + filename, "release")
shutil.copy(zlib, "release")
shutil.copy(msvcrtfile, "release")
shutil.copy("License.rtf", "release")
os.rename("release", "build")

subprocess.Popen(signcmd).wait()
subprocess.Popen(deploycmd).wait()
subprocess.Popen(candlecmd).wait()
subprocess.Popen(lightcmd).wait()
subprocess.Popen(signmsi).wait()

os.rename("Sherpa.msi", "sherpa-" + version + ".msi")

for fn in delfiles:
    if os.path.isfile(fn):
        os.unlink(fn)

for dirname in deldirs:
    if os.path.isdir(dirname):
        shutil.rmtree(dirname)
