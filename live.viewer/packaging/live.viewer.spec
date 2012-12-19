Name: live.viewer
Summary: viewer
Version: 0.0.1
Release: 1
Group: main/app
License: Flora License
Source0: %{name}-%{version}.tar.gz
BuildRequires: cmake, gettext-tools
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(ail)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(appcore-efl)
BuildRequires: pkgconfig(livebox-viewer)
BuildRequires: pkgconfig(ecore-x)
BuildRequires: pkgconfig(livebox-service)
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-appfw-app-manager)
BuildRequires: edje-bin

%description
Livebox viewer development library

%prep
%setup -q

%build
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post

%files
%defattr(-,root,root,-)
/opt/usr/apps/live.viewer/*
/opt/share/*
