%bcond_with wayland

Name: libdynamicbox_viewer
Summary: Library for developing the application
Version: 1.0.0
Release: 1
Group: HomeTF/DynamicBox
License: Flora
Source0: %{name}-%{version}.tar.gz
Source1001: %{name}.manifest
BuildRequires: cmake, gettext-tools, coreutils
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(com-core)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(livebox-service)
BuildRequires: pkgconfig(dynamicbox_service)
BuildRequires: pkgconfig(vconf)
BuildRequires: model-build-features

%if %{with wayland}
%else
BuildRequires: pkgconfig(x11)
BuildRequires: pkgconfig(xext)
%endif

%if "%{model_build_feature_livebox}" == "0"
ExclusiveArch:
%endif

%description
API for creating a new instance of the dynamicbox and managing its life-cycle.

%package devel
Summary: Development Library for Dynamic Box Viewer Application (dev)
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description devel
Header and package configuration files for the dynamicbox viewer development

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

%cmake . -DWAYLAND_SUPPORT=${WAYLAND_SUPPORT} -DX11_SUPPORT=${X11_SUPPORT} -DDYNAMICBOX_ENABLED=On
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post -n %{name} -p /sbin/ldconfig
%postun -n %{name} -p /sbin/ldconfig

%files -n %{name}
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/libdynamicbox_viewer.so*
%{_datarootdir}/license/libdynamicbox_viewer

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/dynamicbox_viewer/dynamicbox.h
%{_libdir}/pkgconfig/dynamicbox_viewer.pc

#################################################
# liblivebox-viewer (for old version)
%package -n liblivebox-viewer
Summary: Library for developing the dynamicbox viewer (old version)
Group: HomeTF/Dynamicbox
License: Flora
Requires: libdynamicbox_viewer

%description -n liblivebox-viewer
Provider APIs to develop the dynamicbox viewer applications. (old version)

%package -n liblivebox-viewer-devel
Summary: Header & package configuration files to support development of the dynamicbox viewer applications. (old version)
Group: Development/Libraries
Requires: liblivebox-viewer

%description -n liblivebox-viewer-devel
Dynamicbox provider application development library (dev) (old version)

%files -n liblivebox-viewer
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/liblivebox-viewer.so*
%{_datarootdir}/license/liblivebox-viewer

%files -n liblivebox-viewer-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/livebox-viewer/livebox.h
%{_libdir}/pkgconfig/livebox-viewer.pc

# End of a file
