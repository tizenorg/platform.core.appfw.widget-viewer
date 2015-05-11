%bcond_with wayland

Name: libwidget_viewer
Summary: Library for developing the application
Version: 1.1.4
Release: 1
Group: Applications/Core Applications
License: Flora License, Version 1.1
Source0: %{name}-%{version}.tar.gz
Source1001: %{name}.manifest
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

%if %{with wayland}
%else
BuildRequires: pkgconfig(x11)
BuildRequires: pkgconfig(xext)
%endif

%if "%{model_build_feature_widget}" == "0"
ExclusiveArch:
%endif

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

%build
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif

%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS="${CFLAGS} -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="${CXXFLAGS} -DTIZEN_ENGINEER_MODE"
export FFLAGS="${FFLAGS} -DTIZEN_ENGINEER_MODE"
%endif

%if %{with wayland}
export WAYLAND_SUPPORT=On
export X11_SUPPORT=Off
%else
export WAYLAND_SUPPORT=Off
export X11_SUPPORT=On
%endif

%cmake . -DWAYLAND_SUPPORT=${WAYLAND_SUPPORT} -DX11_SUPPORT=${X11_SUPPORT} -DWIDGET_ENABLED=On
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post -n %{name} -p /sbin/ldconfig
%postun -n %{name} -p /sbin/ldconfig

%files -n %{name}
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/%{name}.so*
%{_datarootdir}/license/%{name}

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/widget_viewer/widget_viewer.h
%{_libdir}/pkgconfig/widget_viewer.pc

#################################################
# libwidget_viewer_evas
%package -n %{name}_evas
Summary: Library for developing the widget viewer evas
Group: Applications/Core Applications
License: Flora

%description -n %{name}_evas
Provider APIs to develop the widget viewer EFL application.

%package -n %{name}_evas-devel
Summary: widget provider application development library (dev) (EFL version)
Group: Development/Libraries
Requires: %{name}_evas

%description -n %{name}_evas-devel
Header & package configuration files to support development of the widget viewer applications. (for EFL app)

%post -n %{name}_evas -p /sbin/ldconfig
%postun -n %{name}_evas -p /sbin/ldconfig

%files -n %{name}_evas
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/%{name}_evas.so*
%{_datadir}/widget_viewer_evas/*
%{_datarootdir}/license/%{name}_evas

%files -n %{name}_evas-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/widget_viewer_evas/widget_viewer_evas.h
%{_includedir}/widget_viewer_evas/widget_viewer_evas_internal.h
%{_libdir}/pkgconfig/widget_viewer_evas.pc

# End of a file
