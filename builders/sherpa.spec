Summary:            Migrates GnuPG profiles between machines
Name:               sherpa
Version:            0.3.0
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
