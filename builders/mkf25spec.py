#!/usr/bin/python3
# coding: UTF-8

import re

ver_rx = re.compile(r"^VERSION\s*=\s*(\d.\d.\d)\s*$")

with open("../sherpa.pro") as fh:
    version = [Y for Y in [ver_rx.match(X) for X in fh.readlines()] if Y][0].group(1)

spec = re.sub("VERSION_STRING", version, """Summary:            Migrates GnuPG profiles between machines
Name:               sherpa
Version:            VERSION_STRING
Release:            1
License:            ISC
Source:             %{name}-%{version}.tar.gz
URL:                http://github.com/rjhansen/sherpa
BuildRequires:      qt5-devel, gpgme-devel >= 1.6.0, libassuan-devel
BuildRequires:      libgpg-error-devel,minizip-devel
Requires:           qt5, gpgme >= 1.6.0

%description
Sherpa is a graphical tool to help you easily migrate GnuPG profiles
between different GnuPG installations.

%prep
%autosetup

%build
qmake-qt5
make

%install
mkdir -p %{buildroot}/%{_bindir}
install -p -m 755 sherpa %{buildroot}/%{_bindir}

%files
%{_bindir}/sherpa
""")

with open("sherpa.spec", "w", encoding="UTF-8") as fh:
    print(spec, file=fh)
