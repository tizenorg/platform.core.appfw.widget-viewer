Name: liblivebox-viewer
Summary: Library for developing the application.
Version: 0.9.5
Release: 1
Group: framework/livebox
License: Flora License
Source0: %{name}-%{version}.tar.gz
BuildRequires: cmake, gettext-tools, coreutils
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(com-core)
BuildRequires: pkgconfig(x11)
BuildRequires: pkgconfig(xext)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(livebox-service)

%description
API for creating a new instance of the livebox and managing its life-cycle.

%package devel
Summary: Header and package configuration files for the livebox viewer development.
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description devel
Livebox viewer development library (dev)

%prep
%setup -q

%build
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}/%{_datarootdir}/license

%post

%files -n liblivebox-viewer
%manifest liblivebox-viewer.manifest
%defattr(-,root,root,-)
%{_libdir}/*.so*
%{_datarootdir}/license/*

%files devel
%defattr(-,root,root,-)
%{_includedir}/livebox-viewer/livebox.h
%{_datarootdir}/doc/livebox-viewer/livebox-viewer_PG.h
%{_libdir}/pkgconfig/*.pc

# End of a file
