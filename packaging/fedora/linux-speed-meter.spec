Name:           linux-speed-meter
Version:        1.0.0
Release:        1%{?dist}
Summary:        Real-time network speed monitor for Linux

License:        MIT
URL:            https://github.com/linux-speed-meter/linux-speed-meter
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc-c++, cmake, libgtk-3-dev, libappindicator3-dev
Requires:       gtk3 >= 3.24
Requires:       libappindicator-gtk3 >= 12.10

%description
Linux Speed Meter is a cross-desktop application designed to monitor
and display internet upload and download speeds in real-time. It runs
as a system tray application, making it compatible with popular desktop
environments such as GNOME, KDE, and XFCE.

%prep
%setup -q

%build
mkdir -p build
cd build
cmake ..
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_datadir}/applications
mkdir -p %{buildroot}%{_datadir}/icons/hicolor/256x256/apps

install -m 755 build/LinuxSpeedMeter %{buildroot}%{_bindir}/linux-speed-meter

cat > %{buildroot}%{_datadir}/applications/linux-speed-meter.desktop << EOF
[Desktop Entry]
Type=Application
Name=Linux Speed Meter
Comment=Real-time network speed monitor
Exec=linux-speed-meter
Icon=linux-speed-meter
Categories=Utility;Network;Monitor;
StartupNotify=false
Terminal=false
EOF

# Create a simple icon (placeholder)
convert -size 256x256 xc:"#4A90E2" -fill white -pointsize 72 -gravity center -annotate +0+0 "LSM" %{buildroot}%{_datadir}/icons/hicolor/256x256/apps/linux-speed-meter.png 2>/dev/null || {
    touch %{buildroot}%{_datadir}/icons/hicolor/256x256/apps/linux-speed-meter.png
}

%files
%{_bindir}/linux-speed-meter
%{_datadir}/applications/linux-speed-meter.desktop
%{_datadir}/icons/hicolor/256x256/apps/linux-speed-meter.png

%changelog
* Sun Sep 22 2025 Linux Speed Meter Team <info@linuxspeedmeter.com> - 1.0.0-1
- Initial package