# Copyright (c) ZeroC, Inc.

# git_tag, when defined, is typically a branch, for example 3.7
%if 0%{?git_tag:1}
   %define archive_tag %{git_tag}
%else
  %define archive_tag main
%endif

# Java is required to build IceGridGUI. The RPM packages doesn't include the Ice for Java JAR files.
%define javaversion 17-openjdk

%if "%{dist}" == ".amzn2023"
%define javaversion 17-amazon-corretto
%endif

%if "%{dist}" == ".el10"
%define javaversion 21-openjdk
%endif

%define shadow shadow-utils
%define javapackagestools javapackages-tools
%define phpname php
%define phpcommon php-common
%define phpdir %{_datadir}/php
# Macros are lazily evaluated we can can modify phpname later
%define phplibdir %{_libdir}/%{phpname}/modules

# Use Python 3.12
%global python3_pkgversion 3.12

%if "%{dist}" == ".amzn2023"
# We use php8.4 on Amazon Linux 2023
%define phpname php8.4
%define phpcommon %{phpname}
%endif

%if "%{_prefix}" == "/usr"
   %define runpath embedded_runpath=no
%else
   %define runpath embedded_runpath_prefix=%{_prefix}
%endif

Name: %{?nameprefix}ice
Version: 3.8.0~alpha0
Release: 1%{?dist}
Summary: Comprehensive RPC framework with support for C++, Java, JavaScript, Python and more.
%if "%{?ice_license}"
License: %{ice_license}
%else
License: GPLv2 with exceptions
%endif
Vendor: ZeroC, Inc.
URL: https://zeroc.com/
Source0:  https://github.com/zeroc-ice/ice/archive/%{archive_tag}.tar.gz#/%{name}-%{version}.tar.gz

BuildRequires: pkgconfig(expat)
BuildRequires: pkgconfig(libedit)
BuildRequires: pkgconfig(openssl)
BuildRequires: pkgconfig(bzip2)
BuildRequires: pkgconfig(lmdb)
BuildRequires: pkgconfig(libsystemd)

# Amazon Linux 2023 does not provide pkgconfig(mcpp)
%if "%{dist}" == ".amzn2023"
BuildRequires: libmcpp-devel
%else
BuildRequires: pkgconfig(mcpp)
%endif

BuildRequires: java-%{javaversion}, java-%{javaversion}-jmods
BuildRequires: %{phpname}-devel
BuildRequires: python3.12-devel, python3-rpm-macros

%description
This is a meta package for Ice. It does not install any files directly.

#
# Enable debug package except if it's already enabled
#
%if %{!?_enable_debug_packages:1}%{?_enable_debug_packages:0}
%debug_package
%endif

#
# ice-slice package
#
%package -n %{?nameprefix}ice-slice
Summary: Slice files for Ice.
BuildArch: noarch
%description -n %{?nameprefix}ice-slice
This package contains Slice files used by the Ice framework.

Ice is a comprehensive RPC framework that helps you network your software
with minimal effort. Ice takes care of all interactions with low-level
network programming interfaces and allows you to focus your efforts on
your application logic.

#
# icegridgui package
#
%package -n %{?nameprefix}icegridgui
Summary: IceGrid GUI admin client.
BuildArch: noarch
Requires: java
%description -n %{?nameprefix}icegridgui
The IceGrid service helps you locate, deploy and manage Ice servers.

IceGridGUI gives you complete control over your deployed applications.
Activities such as starting a server or modifying a configuration setting
are just a mouse click away.

Ice is a comprehensive RPC framework that helps you network your software
with minimal effort. Ice takes care of all interactions with low-level
network programming interfaces and allows you to focus your efforts on
your application logic.

# Transitional dummy package for clean upgrade from Ice 3.7
%package -n %{?nameprefix}ice-all-runtime
Summary: Transitional package for Ice run-time components.
Obsoletes: %{?nameprefix}ice-all-runtime < %{version}-%{release}
Provides: %{?nameprefix}ice-all-runtime = %{version}-%{release}

%description -n %{?nameprefix}ice-all-runtime
This transitional package exists to support upgrades from Ice 3.7.
It does not install any content and can be safely removed.

# Transitional dummy package for clean upgrade from Ice 3.7
%package -n %{?nameprefix}ice-all-devel
Summary: Transitional package for Ice development components.
Obsoletes: %{?nameprefix}ice-all-devel < %{version}-%{release}
Provides: %{?nameprefix}ice-all-devel = %{version}-%{release}

%description -n %{?nameprefix}ice-all-devel
This transitional package exists to support upgrades from Ice 3.7.
It does not install any content and can be safely removed.

# Transitional dummy package for clean upgrade from Ice 3.7
Summary: Transitional package to install all Slice compilers
Name: %{?nameprefix}ice-compilers
Requires: libice-c++-devel, python3-%{?nameprefix}ice, %{phpname}-%{?nameprefix}ice
Obsoletes: %{?nameprefix}ice-compilers < %{version}-%{release}
Provides: %{?nameprefix}ice-compilers = %{version}-%{release}

#
# libiceMm-c++ package
#
%package -n lib%{?nameprefix}ice3.8-c++
Summary: Ice for C++ run-time libraries.
%description -n lib%{?nameprefix}ice3.8-c++
This package contains the C++ run-time libraries for the Ice framework.

Ice is a comprehensive RPC framework that helps you network your software
with minimal effort. Ice takes care of all interactions with low-level
network programming interfaces and allows you to focus your efforts on
your application logic.

#
# icebox package
#
%package -n %{?nameprefix}icebox
Summary: IceBox server, a framework for Ice application services.
Requires: lib%{?nameprefix}ice3.8-c++ = %{version}-%{release}
Requires: %{?nameprefix}ice-utils = %{version}-%{release}
%description -n %{?nameprefix}icebox
This package contains the IceBox server, an easy-to-use framework for
developing and deploying Ice application services.

Ice is a comprehensive RPC framework that helps you network your software
with minimal effort. Ice takes care of all interactions with low-level
network programming interfaces and allows you to focus your efforts on
your application logic.

#
# libicestorm3.8 package
#
%package -n lib%{?nameprefix}icestorm3.8
Summary: IceStorm publish-subscribe event distribution service.
Requires: lib%{?nameprefix}ice3.8-c++ = %{version}-%{release}
%description -n lib%{?nameprefix}icestorm3.8
This package contains the IceStorm publish-subscribe event distribution
service.

IceStorm helps you create push applications. Your Ice client (the publisher)
sends a request to a topic managed by IceStorm, and IceStorm delivers this
request to all the subscribers (Ice objects) that you registered with this
topic.

Ice is a comprehensive RPC framework that helps you network your software
with minimal effort. Ice takes care of all interactions with low-level
network programming interfaces and allows you to focus your efforts on
your application logic.

#
# libice-c++-devel package
#
%package -n lib%{?nameprefix}ice-c++-devel
Summary: Libraries and headers for developing Ice applications in C++.
Requires: lib%{?nameprefix}ice3.8-c++ = %{version}-%{release}
Requires: glibc-devel
Requires: openssl-devel
%description -n lib%{?nameprefix}ice-c++-devel
This package contains the libraries and headers needed for developing
Ice applications in C++.

Ice is a comprehensive RPC framework that helps you network your software
with minimal effort. Ice takes care of all interactions with low-level
network programming interfaces and allows you to focus your efforts on
your application logic.

#
# ice2slice package
#
%package -n %{?nameprefix}ice2slice
Summary: The Ice-to-Slice compiler (ice2slice)
Requires: %{?nameprefix}ice-slice = %{version}-%{release}
%description -n %{?nameprefix}ice2slice
 This package contains the Ice-to-Slice compiler (ice2slice). The Ice-to-Slice
 compiler converts Slice definitions in .ice files into Slice definitions in .slice
 files. .ice is the Slice syntax and format understood by the Slice compilers provided
 by Ice; .slice is the Slice syntax and format understood by slicec, the Slice compiler
 provided by IceRPC.

Ice is a comprehensive RPC framework that helps you network your software
with minimal effort. Ice takes care of all interactions with low-level
network programming interfaces and allows you to focus your efforts on
your application logic.

#
# ice-utils package
#
%package -n %{?nameprefix}ice-utils
Summary: Ice utilities and admin tools.
Requires: lib%{?nameprefix}ice3.8-c++ = %{version}-%{release}
%description -n %{?nameprefix}ice-utils
This package contains Ice utilities and admin tools.

Ice is a comprehensive RPC framework that helps you network your software
with minimal effort. Ice takes care of all interactions with low-level
network programming interfaces and allows you to focus your efforts on
your application logic.

#
# icegrid package
#
%package -n %{?nameprefix}icegrid
Summary: Locate, deploy, and manage Ice servers.
Requires: lib%{?nameprefix}ice3.8-c++ = %{version}-%{release}
Requires: %{?nameprefix}ice-utils = %{version}-%{release}
# Requirements for the users
Requires(pre): %{shadow}
%{?systemd_requires: %systemd_requires}
%description -n %{?nameprefix}icegrid
This package contains the IceGrid service. IceGrid helps you locate,
deploy and manage Ice servers.

Ice is a comprehensive RPC framework that helps you network your software
with minimal effort. Ice takes care of all interactions with low-level
network programming interfaces and allows you to focus your efforts on
your application logic.

#
# dsnode package
#
%package -n %{?nameprefix}dsnode
Summary: DataStorm node server.
Requires: lib%{?nameprefix}ice3.8-c++ = %{version}-%{release}
%description -n %{?nameprefix}dsnode
This package contains the DataStorm node server. The DataStorm node server allows
other DataStorm nodes to exchange topic discovery information without relying on
UDP multicast. It also helps create deployments that span multiple IP networks.

Ice is a comprehensive RPC framework that helps you network your software
with minimal effort. Ice takes care of all interactions with low-level
network programming interfaces and allows you to focus your efforts on
your application logic.

#
# glacier2 package
#
%package -n %{?nameprefix}glacier2
Summary: Glacier2 router.
Requires: lib%{?nameprefix}ice3.8-c++ = %{version}-%{release}
Requires(pre): %{shadow}
%{?systemd_requires: %systemd_requires}
%description -n %{?nameprefix}glacier2
This package contains the Glacier2 router. A Glacier2 router allows you to
securely route Ice communications across networks, such as the public Internet
and a private network behind a firewall. With Glacier2, you only need to open
one port in your firewall to make multiple back-end Ice servers reachable by
remote Ice clients on the Internet.

Ice is a comprehensive RPC framework that helps you network your software
with minimal effort. Ice takes care of all interactions with low-level
network programming interfaces and allows you to focus your efforts on
your application logic.

#
# icebridge package
#
%package -n %{?nameprefix}icebridge
Summary: Ice bridge.
Requires: lib%{?nameprefix}ice3.8-c++ = %{version}-%{release}
%description -n %{?nameprefix}icebridge
This package contains the Ice bridge. The Ice bridge allows you to bridge
connections securely between one or multiple clients and a server. It
relays requests from clients to a target server and makes every effort
to be as transparent as possible.

Ice is a comprehensive RPC framework that helps you network your software
with minimal effort. Ice takes care of all interactions with low-level
network programming interfaces and allows you to focus your efforts on
your application logic.

#
# php-ice package
#
%package -n %{phpname}-%{?nameprefix}ice
Summary: PHP extension for Ice.
Requires: lib%{?nameprefix}ice3.8-c++ = %{version}-%{release}
Requires: %{phpcommon}

%description -n %{phpname}-%{?nameprefix}ice
This package contains a PHP extension for communicating with Ice.

Ice is a comprehensive RPC framework that helps you network your software
with minimal effort. Ice takes care of all interactions with low-level
network programming interfaces and allows you to focus your efforts on
your application logic.

#
# python3-ice package
#
%package -n python3-%{?nameprefix}ice
Summary: Python extension for Ice.
Requires: lib%{?nameprefix}ice3.8-c++ = %{version}-%{release}
Requires: python3
%description -n python3-%{?nameprefix}ice
This package contains a Python extension for communicating with Ice.

Ice is a comprehensive RPC framework that helps you network your software
with minimal effort. Ice takes care of all interactions with low-level
network programming interfaces and allows you to focus your efforts on
your application logic.

%prep
%setup -q -n %{name}-%{archive_tag}

%build
#
# Recommended flags for optimized hardened build
#
export CXXFLAGS="%{optflags}"
export LDFLAGS="%{?__global_ldflags}"

make CONFIGS="shared" \
  OPTIMIZE=yes \
  V=1 \
  %{runpath} \
  %{?_smp_mflags} \
  PYTHON=python%{python3_pkgversion} \
  LANGUAGES="cpp java php python" srcs

%install

make %{?_smp_mflags} \
  CONFIGS="shared" \
  OPTIMIZE=yes V=1 \
  %{runpath} \
  DESTDIR=%{buildroot} \
  PYTHON=python%{python3_pkgversion} install_pythondir=%{python3_sitearch} \
  prefix=%{_prefix} \
  install_bindir=%{_bindir} \
  install_libdir=%{_libdir} \
  install_slicedir=%{_datadir}/ice/slice \
  install_includedir=%{_includedir} \
  install_mandir=%{_mandir} \
  install_configdir=%{_datadir}/ice \
  install_phplibdir=%{phplibdir} \
  install_phpdir=%{phpdir} \
  LANGUAGES="cpp php python" install

make -C java DESTDIR=%{buildroot} install_javadir=%{_javadir} prefix=%{_prefix} install-icegridgui

# Remove unused installed files
rm -f %{buildroot}%{_datadir}/ice/LICENSE
rm -f %{buildroot}%{_datadir}/ice/ICE_LICENSE

#
# php ice.ini
#
mkdir -p %{buildroot}%{_sysconfdir}/php.d
cp -p packaging/rpm/ice.ini %{buildroot}%{_sysconfdir}/php.d

#
# systemd files (for servers)
#
mkdir -p %{buildroot}%{_sysconfdir}
for i in icegridregistry icegridnode glacier2router
do
    cp packaging/rpm/$i.conf %{buildroot}%{_sysconfdir}
    install -m 644 -p -D packaging/rpm/$i.service %{buildroot}%{_unitdir}/$i.service
done

#
# IceGridGUI
#
mkdir -p %{buildroot}%{_bindir}
cp -p packaging/rpm/icegridgui %{buildroot}%{_bindir}/icegridgui

#
# noarch file packages
#

%files -n %{?nameprefix}ice-slice
%license LICENSE
%license ICE_LICENSE
%doc packaging/rpm/README
%dir %{_datadir}/ice
%{_datadir}/ice/slice

%files -n %{?nameprefix}icegridgui
%license LICENSE
%license ICE_LICENSE
%license packaging/rpm/JGOODIES_LICENSE
%doc packaging/rpm/README
%attr(755,root,root) %{_bindir}/icegridgui
%{_javadir}/icegridgui.jar

#
# Generate "ice-all-runtime" meta package as arch-specific
#
%files -n %{?nameprefix}ice-all-runtime
%license LICENSE
%license ICE_LICENSE
%doc packaging/rpm/README

#
# Generate "ice-all-devel" meta package as arch-specific
#
%files -n %{?nameprefix}ice-all-devel
%license LICENSE
%license ICE_LICENSE
%doc packaging/rpm/README

#
# libice-Mm-c++ package
#
%files -n lib%{?nameprefix}ice3.8-c++
%license LICENSE
%license ICE_LICENSE
%license packaging/rpm/LMDB_LICENSE
%license packaging/rpm/MCPP_LICENSE
%doc packaging/rpm/README
%{_libdir}/libDataStorm.so.*
%{_libdir}/libGlacier2.so.*
%{_libdir}/libIce.so.*
%{_libdir}/libIceBox.so.*
%{_libdir}/libIceDiscovery.so.*
%{_libdir}/libIceGrid.so.*
%{_libdir}/libIceLocatorDiscovery.so.*
%{_libdir}/libIceStorm.so.*
%{_libdir}/cmake/*/*.cmake
%post -n lib%{?nameprefix}ice3.8-c++ -p /sbin/ldconfig
%postun -n lib%{?nameprefix}ice3.8-c++
/sbin/ldconfig
exit 0

#
# icebox package
#
%files -n %{?nameprefix}icebox
%license LICENSE
%license ICE_LICENSE
%doc packaging/rpm/README
%{_bindir}/icebox
%{_mandir}/man1/icebox.1*
%post -n %{?nameprefix}icebox -p /sbin/ldconfig
%postun -n %{?nameprefix}icebox
/sbin/ldconfig
exit 0

#
# libice-c++devel package
#
%files -n lib%{?nameprefix}ice-c++-devel
%license LICENSE
%license ICE_LICENSE
%license packaging/rpm/MCPP_LICENSE
%doc packaging/rpm/README
%{_libdir}/libDataStorm.so
%{_libdir}/libGlacier2.so
%{_libdir}/libIce.so
%{_libdir}/libIceBox.so
%{_libdir}/libIceDiscovery.so
%{_libdir}/libIceGrid.so
%{_libdir}/libIceLocatorDiscovery.so
%{_libdir}/libIceStorm.so
%{_includedir}/DataStorm
%{_includedir}/Glacier2
%{_includedir}/Ice
%{_includedir}/IceDiscovery
%{_includedir}/IceLocatorDiscovery
%{_includedir}/IceBox
%{_includedir}/IceGrid
%{_includedir}/IceStorm
%{_bindir}/slice2cpp
%{_mandir}/man1/slice2cpp.1*

#
# libicestorm-Mm package
#
%files -n lib%{?nameprefix}icestorm3.8
%license LICENSE
%license ICE_LICENSE
%doc packaging/rpm/README
%{_libdir}/libIceStormService.so.*
%post -n lib%{?nameprefix}icestorm3.8 -p /sbin/ldconfig
%postun -n lib%{?nameprefix}icestorm3.8
/sbin/ldconfig
exit 0

#
# ice2slice package
#
%files -n %{?nameprefix}ice2slice
%license LICENSE
%license ICE_LICENSE
%license packaging/rpm/MCPP_LICENSE
%doc packaging/rpm/README
%{_bindir}/ice2slice
%{_mandir}/man1/ice2slice.1*

#
# ice-utils package
#
%files -n %{?nameprefix}ice-utils
%license LICENSE
%license ICE_LICENSE
%doc packaging/rpm/README
%{_bindir}/iceboxadmin
%{_mandir}/man1/iceboxadmin.1*
%{_bindir}/icestormadmin
%{_mandir}/man1/icestormadmin.1*
%{_bindir}/icestormdb
%{_mandir}/man1/icestormdb.1*
%{_bindir}/icegridadmin
%{_mandir}/man1/icegridadmin.1*
%{_bindir}/icegriddb
%{_mandir}/man1/icegriddb.1*
%post -n %{?nameprefix}ice-utils -p /sbin/ldconfig
%postun -n %{?nameprefix}ice-utils
/sbin/ldconfig
exit 0

#
# icegrid package
#
%files -n %{?nameprefix}icegrid
%license LICENSE
%license ICE_LICENSE
%doc packaging/rpm/README
%{_bindir}/icegridnode
%{_mandir}/man1/icegridnode.1*
%{_bindir}/icegridregistry
%{_mandir}/man1/icegridregistry.1*
%dir %{_datadir}/ice
%{_datadir}/ice/templates.xml
%attr(644,root,root) %{_unitdir}/icegridregistry.service
%attr(644,root,root) %{_unitdir}/icegridnode.service
%config(noreplace) %{_sysconfdir}/icegridregistry.conf
%config(noreplace) %{_sysconfdir}/icegridnode.conf

%pre -n %{?nameprefix}icegrid
%if "%{_prefix}" == "/usr"
getent group ice > /dev/null || groupadd -r ice
getent passwd ice > /dev/null || \
  useradd -r -g ice -d %{_localstatedir}/lib/ice -s /sbin/nologin -c "Ice Service account" ice
test -d %{_localstatedir}/lib/ice/icegrid/registry || \
  mkdir -p %{_localstatedir}/lib/ice/icegrid/registry; chown -R ice.ice %{_localstatedir}/lib/ice
test -d %{_localstatedir}/lib/ice/icegrid/node1 || \
  mkdir -p %{_localstatedir}/lib/ice/icegrid/node1; chown -R ice.ice %{_localstatedir}/lib/ice
exit 0
%endif

%post -n %{?nameprefix}icegrid
/sbin/ldconfig
%if "%{_prefix}" == "/usr"
  %systemd_post icegridregistry.service
  %systemd_post icegridnode.service
%endif

%preun -n %{?nameprefix}icegrid
%if "%{_prefix}" == "/usr"
  %systemd_preun icegridnode.service
  %systemd_preun icegridregistry.service
%endif

%postun -n %{?nameprefix}icegrid
%if "%{_prefix}" == "/usr"
  %systemd_postun_with_restart icegridnode.service
  %systemd_postun_with_restart icegridregistry.service
%endif
/sbin/ldconfig
exit 0

#
# dsnode package
#
%files -n %{?nameprefix}dsnode
%license LICENSE
%license ICE_LICENSE
%doc packaging/rpm/README
%{_bindir}/dsnode
%{_mandir}/man1/dsnode.1*
%post -n %{?nameprefix}dsnode -p /sbin/ldconfig
%postun -n %{?nameprefix}dsnode
/sbin/ldconfig
exit 0

#
# glacier2 package
#
%files -n %{?nameprefix}glacier2
%license LICENSE
%license ICE_LICENSE
%doc packaging/rpm/README
%{_bindir}/glacier2router
%{_mandir}/man1/glacier2router.1*
%attr(644,root,root) %{_unitdir}/glacier2router.service
%config(noreplace) %{_sysconfdir}/glacier2router.conf

%pre -n %{?nameprefix}glacier2
%if "%{_prefix}" == "/usr"
  getent group ice > /dev/null || groupadd -r ice
  getent passwd ice > /dev/null || \
         useradd -r -g ice -d %{_localstatedir}/lib/ice \
         -s /sbin/nologin -c "Ice Service account" ice
  exit 0
%endif

%post -n %{?nameprefix}glacier2
/sbin/ldconfig
%if "%{_prefix}" == "/usr"
  %systemd_post glacier2router.service
%endif

%preun -n %{?nameprefix}glacier2
%if "%{_prefix}" == "/usr"
  %systemd_preun glacier2router.service
%endif

%postun -n %{?nameprefix}glacier2
%if "%{_prefix}" == "/usr"
  %systemd_postun_with_restart glacier2router.service
%endif
/sbin/ldconfig
exit 0

#
# icebridge package
#
%files -n %{?nameprefix}icebridge
%license LICENSE
%license ICE_LICENSE
%doc packaging/rpm/README
%{_bindir}/icebridge
%{_mandir}/man1/icebridge.1*
%post -n %{?nameprefix}icebridge -p /sbin/ldconfig
%postun -n %{?nameprefix}icebridge
/sbin/ldconfig
exit 0

#
# php-ice package
#
%files -n %{phpname}-%{?nameprefix}ice
%license LICENSE
%license ICE_LICENSE
%license packaging/rpm/MCPP_LICENSE
%doc packaging/rpm/README
%{phpdir}
%{phplibdir}/ice.so
%config(noreplace) %{_sysconfdir}/php.d/ice.ini
%{_bindir}/slice2php
%{_mandir}/man1/slice2php.1*

#
# python3-ice package
#
%files -n python3-%{?nameprefix}ice
%license LICENSE
%license ICE_LICENSE
%license packaging/rpm/MCPP_LICENSE
%doc packaging/rpm/README
%{python3_sitearch}/*
%{_bindir}/slice2py
%{_mandir}/man1/slice2py.1*

%changelog
* Wed Feb 5 2025 José Gutiérrez de la Concha <jose@zeroc.com> 3.8a0
- The 3.8.0-alpha0 release

* Thu Nov 21 2024 José Gutiérrez de la Concha <jose@zeroc.com> 3.7.10-3
- Fix RHEL 9 x86 builds https://github.com/zeroc-ice/ice/issues/3168

* Fri Mar 8 2024 José Gutiérrez de la Concha <jose@zeroc.com> 3.7.10-2
- Fix IceGridGUI JDK 8 builds https://github.com/zeroc-ice/ice/pull/1913

* Tue Nov 7 2023 José Gutiérrez de la Concha <jose@zeroc.com> 3.7.10
- 3.7.10 release

* Tue Mar 14 2023 José Gutiérrez de la Concha <jose@zeroc.com> 3.7.9
- 3.7.9 release

* Mon Jun 27 2022 José Gutiérrez de la Concha <jose@zeroc.com> 3.7.8
- 3.7.8 release

* Tue Nov 30 2021 José Gutiérrez de la Concha <jose@zeroc.com> 3.7.7
- 3.7.7 release

* Thu May 20 2021 José Gutiérrez de la Concha <jose@zeroc.com> 3.7.6
- 3.7.6 release

* Fri Jan 1 2021 José Gutiérrez de la Concha <jose@zeroc.com> 3.7.5
- 3.7.5 release

* Tue Apr 21 2020 José Gutiérrez de la Concha <jose@zeroc.com> 3.7.4
- 3.7.4 release

* Wed Apr 8 2020 Benoit Foucher <benoit@zeroc.com> 3.7.3-2
- Support for x86 multilib build on EL8 platforms

* Wed Jul 17 2019 Bernard Normier <bernard@zeroc.com> 3.7.3
- Updates for the 3.7.3 release.

* Thu Nov 29 2018 Bernard Normier <bernard@zeroc.com> 3.7.2
- Updates for the 3.7.2 release, see ice/CHANGELOG-3.7.md.

* Fri Apr 13 2018 Bernard Normier <bernard@zeroc.com> 3.7.1
- Updates for the 3.7.1 release, see ice/CHANGELOG-3.7.md.

* Fri Jul 21 2017 Bernard Normier <bernard@zeroc.com> 3.7.0
- Updates for the 3.7.0 release, see ice/CHANGELOG-3.7.md.

* Thu Apr 13 2017 José Gutiérrez de la Concha <jose@zeroc.com> 3.7b0
- Added slice2js, slice2rb and slice2obj to ice-compilers

* Fri Mar 10 2017 Benoit Foucher <benoit@zeroc.com> 3.7b0
- Added icebridge package

* Tue Mar 7 2017 Benoit Foucher <benoit@zeroc.com> 3.7b0
- Version bump

* Fri Feb 17 2017 Bernard Normier <bernard@zeroc.com> 3.7a4
- Updates for Ice 3.7
 - Added python-ice package
 - Added slice2cs and slice2py to ice-compilers
 - Removed libice-java package
 - Build with recommended compiler flags
 - Enable debuginfo packages

* Wed Sep 14 2016 José Gutiérrez de la Concha <jose@zeroc.com> 3.7a3
- Rename ice-utils-java as icegridgui

* Thu Apr 14 2016 Mark Spruiell <mes@zeroc.com> 3.6.3
- x86-32 dependencies should only be required when building x86 packages on
  a bi-arch platform.

* Mon Feb 29 2016 Benoit Foucher <benoit@zeroc.com> 3.6.2
- Made the signing of the IceGridGUI jar file optional if JARSIGNER_KEYSTORE
  is not set.
- Added ice_license macro to allow customizing the licence.
- Added nameprefix macro to allow adding a prefix to the rpm package name.

* Fri Oct 31 2014 Mark Spruiell <mes@zeroc.com> 3.6b
- Updates for the Ice 3.6b release.

* Thu Jul 18 2013 Mark Spruiell <mes@zeroc.com> 3.5.1
- Adding man pages.

* Thu Feb 7 2013 Mark Spruiell <mes@zeroc.com> 3.5.0
- Updates for the Ice 3.5.0 release.

* Mon Nov 19 2012 Mark Spruiell <mes@zeroc.com> 3.5b
- Updates for the Ice 3.5b release.

* Tue Dec 15 2009 Mark Spruiell <mes@zeroc.com> 3.4b
- Updates for the Ice 3.4b release.

* Wed Mar 4 2009 Bernard Normier <bernard@zeroc.com> 3.3.1
- Minor updates for the Ice 3.3.1 release.

* Wed Feb 27 2008 Bernard Normier <bernard@zeroc.com> 3.3b-1
- Updates for Ice 3.3b release:
 - Split main ice rpm into ice noarch (license and Slice files), ice-libs
   (C++ runtime libraries), ice-utils (admin tools & utilities), ice-servers
   (icegridregistry, icebox etc.). This way, ice-libs 3.3.0 can coexist with
    ice-libs 3.4.0. The same is true for ice-mono, and to a lesser extent
    other ice runtime packages
- Many updates derived from Mary Ellen Foster (<mefoster at gmail.com>)'s
  Fedora RPM spec for Ice.
 - The Ice jar files are now installed in _javalibdir, with
   jpackage-compliant names
 - New icegridgui shell script to launch the IceGrid GUI
 - The .NET files are now packaged using gacutil with the -root option.
 - ice-servers creates a new user (ice) and installs three init.d services:
   icegridregistry, icegridnode and glacier2router.
 - Python, Ruby and PHP files are now installed in the correct directories.

* Fri Jul 27 2007 Bernard Normier <bernard@zeroc.com> 3.2.1-1
- Updated for Ice 3.2.1 release

* Wed Jun 13 2007 Bernard Normier <bernard@zeroc.com>
- Added patch with new IceGrid.Node.AllowRunningServersAsRoot property.

* Wed Dec 6 2006 ZeroC Staff <support@zeroc.com>
- See source distributions or the ZeroC website for more information
  about the changes in this release
