Name: liblivebox-viewer
Summary: Library for the development of a livebox viewer
Version: 0.8.9
Release: 1
Group: main/app
License: Flora License
Source0: %{name}-%{version}.tar.gz
BuildRequires: cmake, gettext-tools
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
Livebox viewer development library

%package devel
Summary: Files for livebox viewer development.
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
mkdir -p %{buildroot}/usr/share/license

%post

%files -n liblivebox-viewer
%manifest liblivebox-viewer.manifest
%defattr(-,root,root,-)
/usr/lib/*.so*
/usr/share/license/*

%files devel
%defattr(-,root,root,-)
/usr/include/livebox-viewer/livebox.h
/usr/share/doc/livebox-viewer/livebox-viewer_PG.h
/usr/lib/pkgconfig/*.pc
