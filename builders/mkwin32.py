#!/usr/bin/python3
# coding=UTF-8

import os
import os.path
import shutil
import subprocess
import re
import sys

# To use this with GitHub's Git Shell, add the following
# to your Documents\WindowsPowerShell\profile.ps1 file.
#
# pushd 'C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools' cmd /c "vsvars32.bat&set" |
# foreach {
#   if ($_ -match "=") {
#     $v = $_.split("="); set-item -force -path "ENV:\$($v[0])"  -value "$($v[1])"
#   }
# }
# popd
# write-host "`nVisual Studio 2015 Command Prompt variables set." -ForegroundColor Yellow


# User-serviceable parts here
qtdir = r"C:\Qt\Qt5.7.1\5.7\msvc2015\bin\\"
vc14dir = r"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\\"
w32sdkdir = r"C:\Program Files (x86)\Windows Kits\10\bin\x86\\"
wixdir = r"C:\Program Files (x86)\WiX Toolset v3.10\bin\\"
gnupgdir = r"C:\Program Files (x86)\GnuPG\bin\\"
msvcrtfile = r"C:\Program Files (x86)\Common Files\Merge Modules\\" +\
    "Microsoft_VC140_CRT_x86.msm"
zlibdir = r"C:\zlib\\"
shouldSign = True
# End user-serviceable parts

os.chdir("..")

cmd = {
    "qmake" : qtdir + "qmake.exe",
    "nmake" : vc14dir + "nmake.exe",
    "signtool" : w32sdkdir + "signtool.exe",
    "deployqt" : qtdir + "windeployqt.exe",
    "candle" : wixdir + "candle.exe",
    "light" : wixdir + "light.exe"
}
zlib = zlibdir + "zlib1.dll"
delfiles = ["Makefile", "Makefile.Debug", "Makefile.Release",
    "sherpa.wixobj", "sherpa.wixpdb", "sherpa.wxs"]
deldirs = ["build", "debug", "release"]

for name in cmd:
    if not os.path.isfile(cmd[name]):
        print("Couldn't find tool '" + name + "'")
        sys.exit(0)

[os.unlink("builders/" + Y) for Y in
    [X for X in os.listdir("builders") if re.search(r"\.msi$", X)]]

for fn in delfiles:
    if os.path.isfile(fn):
        os.unlink(fn)

for dirname in deldirs:
    if os.path.isdir(dirname):
        shutil.rmtree(dirname)

version_re = re.compile(r"\d\.\d\.\d")
with open("sherpa.pro") as fh:
    version = version_re.search(fh.read()).group(0)
with open("builders/sherpa_template.wxs", encoding="Windows-1252") as fh:
    template = fh.read()
with open("sherpa.wxs", "w", encoding="Windows-1252") as fh:
    print(re.sub(r"\$VERSION", version, template), file=fh)

qmakecmd = [cmd["qmake"], "Sherpa.pro"]
buildcmd = [cmd["nmake"]]
signcmd = [cmd["signtool"], "sign", "/tr", "http://timestamp.digicert.com",
    "/td", "sha256", "/fd", "sha256", "/a", r"release\Sherpa.exe"]
deploycmd = [cmd["deployqt"], r"release\sherpa.exe"]
candlecmd = [cmd["candle"], "sherpa.wxs"]
lightcmd = [cmd["light"], "-ext", "WixUIExtension", "sherpa.wixobj"]
signmsi = [cmd["signtool"], "sign", "/tr", "http://timestamp.digicert.com",
    "/td", "sha256", "/fd", "sha256", "/a", r"Sherpa.msi"]

subprocess.Popen(qmakecmd).wait()
subprocess.Popen(buildcmd).wait()

for filename in ["gpgme-w32spawn.exe", "libassuan-0.dll",
    "libgpg-error-0.dll", "libgpgme-11.dll"]:
    shutil.copy(gnupgdir + filename, "release")
shutil.copy(zlib, "release")
shutil.copy(msvcrtfile, "release")
shutil.copy("License.rtf", "release")

if shouldSign:
    subprocess.Popen(signcmd).wait()
subprocess.Popen(deploycmd).wait()
subprocess.Popen(candlecmd).wait()
subprocess.Popen(lightcmd).wait()
if shouldSign:
    subprocess.Popen(signmsi).wait()

os.rename("Sherpa.msi", "sherpa-" + version + ".msi")
shutil.move("sherpa-" + version + ".msi", "builders")

for fn in delfiles:
    if os.path.isfile(fn):
        os.unlink(fn)

for dirname in deldirs:
    if os.path.isdir(dirname):
        shutil.rmtree(dirname)

os.chdir("builders")
