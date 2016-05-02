%bcond_with wayland

Name: libwidget_viewer
Summary: Library for developing the application
Version: 1.3
Release: 1
Group: Applications/Core Applications
License: Flora-1.1
Source0: %{name}-%{version}.tar.gz
Source1001: %{name}_evas.manifest
Source1002: %{name}_dali.manifest
Source1003: org.tizen.widget_viewer_sdk.manifest
Source1004: watch-control.manifest
BuildRequires: cmake, gettext-tools, coreutils, edje-bin
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(gio-2.0)
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
BuildRequires: pkgconfig(libtzplatform-config)
BuildRequires: pkgconfig(dali-adaptor)
BuildRequires: pkgconfig(dali-toolkit)
BuildRequires: pkgconfig(pepper-dali)

%description
API for creating a new instance of the widget and managing its life-cycle.

%package devel
Summary: Development Library for widget Viewer Application (dev)
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description devel
Header and package configuration files for the widget viewer development

%prep
%setup -q
cp %{SOURCE1001} .
cp %{SOURCE1002} .
cp %{SOURCE1003} .
cp %{SOURCE1004} .

%build
%cmake . -DWIDGET_ENABLED=On -DTZ_SYS_SHARE=%{TZ_SYS_SHARE}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post -n %{name} -p /sbin/ldconfig
%postun -n %{name} -p /sbin/ldconfig

%files -n %{name}
%attr(0644,root,root)%{_libdir}/%{name}.so*
%{_datarootdir}/license/%{name}

%files devel
%{_includedir}/widget_viewer/widget_viewer.h
%{_libdir}/pkgconfig/widget_viewer.pc

################################################
# libwidget_toolkit
%package -n widget_toolkit
Summary: APIs to develop the widget viewer libraries
Version: 0.0.1
License: FLora-1.1
Group: Applications/Core Applications

%description -n widget_toolkit
A sset of APIs to implement widget libraries

%package -n widget_toolkit-devel
Summary: APIs to develop the widget viewer libraries
Group: Development/Libraries
Requires: widget_toolkit

%description -n widget_toolkit-devel
Header & package configuration files of widget_toolkit

%post -n widget_toolkit -p /sbin/ldconfig
%postun -n widget_toolkit -p /sbin/ldconfig

#################################################
# libwidget_viewer_evas
%package -n %{name}_evas
Summary: Library for developing the widget viewer evas
Group: Applications/Core Applications
License: Flora-1.1
Requires: widget_toolkit

%description -n %{name}_evas
Provider APIs to develop the widget viewer EFL application.

%package -n %{name}_evas-devel
Summary: Widget provider application development library (dev) (EFL version)
Group: Development/Libraries
Requires: %{name}_evas

%description -n %{name}_evas-devel
Header & package configuration files to support development of the widget viewer applications. (for EFL app)

%post -n %{name}_evas -p /sbin/ldconfig
%postun -n %{name}_evas -p /sbin/ldconfig

#################################################
# libwidget_viewer_dali
%package -n %{name}_dali
Summary: Library for developing the widget viewer DALi
Group: Applications/Core Applications
License: Flora-1.1

%description -n %{name}_dali
Provider APIs to develop the widget viewer DALi application.

%package -n %{name}_dali-devel
Summary: Widget provider application development library (dev) (DALi version)
Group: Development/Libraries
Requires: %{name}_dali

%description -n %{name}_dali-devel
Header & package configuration files to support development of the widget viewer applications. (for DALi app)

%post -n %{name}_dali -p /sbin/ldconfig
%postun -n %{name}_dali -p /sbin/ldconfig

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

################################################
# libwatch_control
%package -n watch-control
Summary: APIs to control watch applications
Version: 0.0.1
License: Flora-1.1
Group: Applications/Core Applications
Requires: widget_toolkit

%description -n watch-control
A set of APIs to control watch applications

%package -n watch-control-devel
Summary: APIs to control watch applications
Group: Development/Libraries
Requires: watch-control

%description -n watch-control-devel
Header & package configuration of watch-control

%post -n watch-control -p /sbin/ldconfig
%postun -n watch-control -p /sbin/ldconfig

%files -n %{name}_evas
%manifest %{name}_evas.manifest
%attr(0644,root,root) %{_libdir}/%{name}_evas.so*
%{_datarootdir}/license/%{name}_evas
%{TZ_SYS_SHARE}/widget_viewer_evas/res/edje/widget_viewer_evas.edj
%{TZ_SYS_SHARE}/widget_viewer_evas/res/image/*.png
%{TZ_SYS_SHARE}/widget_viewer_evas/res/locale/*/LC_MESSAGES/*


%files -n %{name}_evas-devel
%{_includedir}/widget_viewer_evas/widget_viewer_evas.h
%{_libdir}/pkgconfig/widget_viewer_evas.pc

%files -n %{name}_dali
%manifest %{name}_dali.manifest
%attr(0644,root,root) %{_libdir}/%{name}_dali.so*
%{_datarootdir}/license/%{name}_dali
%{TZ_SYS_SHARE}/widget_viewer_dali/images/*.png

%files -n %{name}_dali-devel
%{_includedir}/widget_viewer_dali/widget_viewer_dali.h
%{_includedir}/widget_viewer_dali/public_api/*
%{_libdir}/pkgconfig/widget_viewer_dali.pc

%files -n org.tizen.widget_viewer_sdk
%manifest org.tizen.widget_viewer_sdk.manifest
%attr(-,app,app) %dir /opt/usr/apps/org.tizen.widget_viewer_sdk/data
%{_datarootdir}/packages/org.tizen.widget_viewer_sdk.xml
%{_datarootdir}/license/org.tizen.widget_viewer_sdk
%{_prefix}/apps/org.tizen.widget_viewer_sdk/*

%files -n widget_toolkit
%{_libdir}/libwidget_toolkit.so*

%files -n widget_toolkit-devel
%{_includedir}/widget_toolkit/*.h
%{_libdir}/pkgconfig/widget_toolkit.pc

%files -n watch-control
%manifest watch-control.manifest
%{_libdir}/libwatch-control.so*

%files -n watch-control-devel
%{_includedir}/watch-control/*.h
%{_libdir}/pkgconfig/watch-control.pc


# End of a file
