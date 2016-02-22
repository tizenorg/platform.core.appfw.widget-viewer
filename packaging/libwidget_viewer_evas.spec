%bcond_with wayland

Name: libwidget_viewer_evas
Summary: Library for developing the widget viewer evas
Version: 2.0.0
Release: 1
Group: Applications/Core Applications
License: Flora-1.1
Source0: %{name}-%{version}.tar.gz
Source1001: %{name}_evas.manifest
Source1002: org.tizen.widget_viewer_sdk.manifest
BuildRequires: cmake, gettext-tools, coreutils, edje-bin
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(com-core)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(widget_service)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(appcore-efl)
BuildRequires: pkgconfig(capi-system-info)
BuildRequires: pkgconfig(efl-extension)
BuildRequires: pkgconfig(wayland-client)
BuildRequires: pkgconfig(libtbm)
BuildRequires: pkgconfig(libpepper-efl)

%description
Provider APIs to develop the widget viewer EFL application.

%package devel
Summary: Widget provider application development library (dev) (EFL version)
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description devel
Header & package configuration files to support development of the widget viewer applications. (for EFL app)

%prep
%setup -q
cp %{SOURCE1001} .
cp %{SOURCE1002} .

%build
%cmake . -DWIDGET_ENABLED=On
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post -n %{name} -p /sbin/ldconfig
%postun -n %{name} -p /sbin/ldconfig

%files -n %{name}
%defattr(-,root,root,-)
%{_libdir}/%{name}.so*
%{_datarootdir}/license/%{name}

%files devel
%defattr(-,root,root,-)
%{_includedir}/widget_viewer/widget_viewer.h
%{_libdir}/pkgconfig/widget_viewer.pc

#################################################
# org.tizen.widget_viewer_sdk
%package -n org.tizen.widget_viewer_sdk
Summary: The widget viewer for development using SDK(IDE)
Version: 0.0.1
Group: Development/Tools
License: Flora-1.1
Requires: %{name}_evas

%description -n org.tizen.widget_viewer_sdk
While developing the widget applications, this viewer will load it and execute it to help you to see it on the screen.

%post -n org.tizen.widget_viewer_sdk -p /sbin/ldconfig
%postun -n org.tizen.widget_viewer_sdk -p /sbin/ldconfig

%files -n org.tizen.widget_viewer_sdk
%manifest org.tizen.widget_viewer_sdk.manifest
%defattr(-,root,root,-)
%attr(-,app,app) %dir /opt/usr/apps/org.tizen.widget_viewer_sdk/data
%{_datarootdir}/packages/org.tizen.widget_viewer_sdk.xml
%{_datarootdir}/license/org.tizen.widget_viewer_sdk
%{_prefix}/apps/org.tizen.widget_viewer_sdk/*

# End of a file
