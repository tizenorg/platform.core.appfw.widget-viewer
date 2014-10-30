#sbs-git:slp/pkgs/v/w-add-viewer w-add-viewer 0.1.2 226202351de9fefb43756c36d215ca74f52431d0
%define _project_name w-add-viewer
%define _package_name com.samsung.%{_project_name}

Name: %{_package_name}
Summary: w-add-viewer application (EFL)
Version: 0.0.1
Release: 1
Group: HomeTF/AddViewer
License: Flora
Source0: %{name}-%{version}.tar.gz
Source1001: %{name}.manifest
BuildRequires: pkgconfig(appcore-efl)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(ecore-x)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(edje)
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(ail)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(utilX)
BuildRequires: pkgconfig(pkgmgr)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(dynamicbox_viewer)
BuildRequires: pkgconfig(dynamicbox_service)
BuildRequires: pkgconfig(dynamicbox)
BuildRequires: pkgconfig(dynamicbox_provider)
BuildRequires: pkgconfig(dynamicbox_provider_app)
BuildRequires: pkgconfig(mm-player)
BuildRequires: cmake
BuildRequires: edje-bin
BuildRequires: embryo-bin
BuildRequires: gettext-devel
BuildRequires: hash-signer
Requires(post): signing-client

%description
w-add-viewer

%prep
%setup -q
cp %{SOURCE1001} .

%build
%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS="$CFLAGS -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_ENGINEER_MODE"
export FFLAGS="$FFLAGS -DTIZEN_ENGINEER_MODE"
%endif
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif
RPM_OPT=`echo $CFLAGS|sed 's/-Wp,-D_FORTIFY_SOURCE=2//'`
export CFLAGS=$RPM_OPT
export CFLAGS+=" -DARCH=%{ARCH}"
%cmake .
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%define tizen_sign 1
%define tizen_sign_base /usr/apps/%{_package_name}
%define tizen_sign_level platform
%define tizen_author_sign 1
%define tizen_dist_sign 1
%make_install

%post
/usr/bin/signing-client/hash-signer-client.sh -a -d -p platform /usr/apps/${_package_name}

%files
%manifest %{_package_name}.manifest
%defattr(-,root,root,-)
/usr/apps/%{_package_name}/*
/etc/smack/accesses2.d/%{_package_name}.rule
/usr/share/packages/%{_package_name}.xml
/usr/share/license/%{_package_name}
/opt/usr/media/Videos/*
/usr/share/icons/default/small/*.png
#/opt/usr/media/Videos/*
