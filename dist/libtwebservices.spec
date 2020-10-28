Name:           libtwebservices
Version:        0.1
Release:        1%{?dist}
Summary:        Common libraries for the* apps to communicate with web APIs

License:        GPLv3+
URL:            https://github.com/vicr123/libtwebservices
Source0:        https://github.com/vicr123/libtwebservices/archive/v%{version}.tar.gz

%if 0%{?fedora} == 32
BuildRequires:  make qt5-devel the-libs-devel
Requires:       qt5 the-libs
%endif

%if 0%{?fedora} >= 33
BuildRequires:  make qt5-qtbase-devel the-libs-devel qt5-linguist
Requires:       qt5-qtbase the-libs
%endif

%define debug_package %{nil}
%define _unpackaged_files_terminate_build 0

%description
Common libraries for the* apps to communicate with web APIs

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%setup

%build
qmake-qt5
make

%install
rm -rf $RPM_BUILD_ROOT
#%make_install
make install INSTALL_ROOT=$RPM_BUILD_ROOT
cp -r $RPM_BUILD_ROOT/../lib64 $RPM_BUILD_ROOT/usr
rm -rf $RPM_BUILD_ROOT/../lib64
find $RPM_BUILD_ROOT -name '*.la' -exec rm -f {} ';'


%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%{_libdir}/*.so.*

%files devel
%{_includedir}/*
%{_libdir}/*.so
%{_libdir}/qt5/mkspecs/modules/qt_twebservices.pri


%changelog
* Sun May  3 2020 Victor Tran
- 
