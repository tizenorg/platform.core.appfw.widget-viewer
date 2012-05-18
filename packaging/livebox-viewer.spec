Name: livebox-viewer
Summary: Library for the development of a livebox viewer
Version: 0.0.2
Release: 1
Group: main/app
License: Samsung Proprietary License
Source0: %{name}-%{version}.tar.gz
BuildRequires: cmake, gettext-tools
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(gio-2.0)

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
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}

%build
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post

%files
%defattr(-,root,root,-)
/usr/lib/*.so*

%files devel
%defattr(-,root,root,-)
/usr/include/livebox-viewer/livebox.h
/usr/share/doc/livebox-viewer/livebox-viewer_PG.h
/usr/lib/pkgconfig/*.pc
