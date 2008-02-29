# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

%if "%{dist}" != ".sles10"
%define ruby 1
%define mono 0
%else
%define ruby 0
%define mono 1
%endif

%define buildall 1
%define makeopts "-j 2"

%define core_arches %{ix86} x86_64

#
# See http://fedoraproject.org/wiki/Packaging/Python
# Since we build a single ice-python arch-specific package, we put everything in sitearch
#
%{!?python_sitearch: %define python_sitearch %(%{__python} -c "from distutils.sysconfig import get_python_lib; print get_python_lib(1)")}

%if %{ruby}
#
# See http://fedoraproject.org/wiki/Packaging/Ruby
# Since we build a single ice-ruby arch-specific package, we put everything in sitearch
#
%{!?ruby_sitearch: %define ruby_sitearch %(ruby -rrbconfig -e 'puts Config::CONFIG["sitearchdir"]')}
%endif

Name: ice
Version: 3.3.0
Summary: Files common to all Ice packages 
Release: 1%{?dist}
License: GPL with exceptions
Group: System Environment/Libraries
Vendor: ZeroC, Inc.
URL: http://www.zeroc.com/
Source0: Ice-%{version}.tar.gz
Source1: Ice-rpmbuild-%{version}.tar.gz

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%define soversion 33
%define dotnetversion 3.3.0
%define dotnetmainversion 3.3

BuildRequires: python >= 2.3.4, python-devel >= 2.3.4
BuildRequires: expat >= 1.95.7
BuildRequires: openssl >= 0.9.7a, openssl-devel >= 0.9.7a
BuildRequires: db46 >= 4.6.21, db46-devel >= 4.6.21, db46-java >= 4.6.21
BuildRequires: jpackage-utils
BuildRequires: mcpp-devel >= 2.6.4

%if %{ruby}
BuildRequires: ruby, ruby-devel
%endif

%if %{mono}
BuildRequires: mono-core >= 1.2.6
%endif

%if "%{dist}" == ".rhel4"
BuildRequires: nptl-devel
BuildRequires: bzip2-devel >= 1.0.2
BuildRequires: expat-devel >= 1.95.7
BuildRequires: php >= 5.1.4, php-devel >= 5.1.4
%endif
%if "%{dist}" == ".rhel5"
BuildRequires: bzip2-devel >= 1.0.3
BuildRequires: expat-devel >= 1.95.8
BuildRequires: php >= 5.1.6, php-devel >= 5.1.6
%endif
%if "%{dist}" == ".sles10"
BuildRequires: php5 >= 5.1.2, php5-devel >= 5.1.2
%endif

%description
Ice is a modern alternative to object middleware such as CORBA or
COM/DCOM/COM+.  It is easy to learn, yet provides a powerful network
infrastructure for demanding technical applications. It features an
object-oriented specification language, easy to use C++, C#, Java,
Python, Ruby, PHP, and Visual Basic mappings, a highly efficient
protocol, asynchronous method invocation and dispatch, dynamic
transport plug-ins, TCP/IP and UDP/IP support, SSL-based security, a
firewall solution, and much more.


#
# Arch-independent packages
#
%ifarch noarch
%package java
Summary: The Ice runtime for Java
Group: System Environment/Libraries
Requires: ice = %{version}-%{release}, db46-java
%description java
The Ice runtime for Java

%if %{mono}
%package dotnet
Summary: The Ice runtime for .NET (mono)
Group: System Environment/Libraries
Requires: ice = %{version}-%{release}, mono-core >= 1.2.6
%description dotnet
The Ice runtime for .NET (mono).
%endif
%endif

#
# Arch-dependent packages
#
%ifarch %{core_arches}
%package libs
Summary: The Ice runtime for C++
Group: System Environment/Libraries
Requires: ice = %{version}-%{release}, db46
%description libs
The Ice runtime for C++

%package utils
Summary: Ice utilities and admin tools.
Group: Applications/System
Requires: ice-libs = %{version}-%{release}
%description utils
Admin tools to manage Ice servers (IceGrid, IceStorm, IceBox etc.),
plus various Ice-related utilities.

%package servers
Summary: Ice servers and related files.
Group: System Environment/Daemons
Requires: ice-utils = %{version}-%{release}
%if %{mono}
Requires: ice-dotnet = %{version}-%{release}
%endif
# Requirements for the users
Requires(pre): shadow-utils
# Requirements for the init.d services
Requires(post): /sbin/chkconfig
Requires(preun): /sbin/chkconfig
Requires(preun): /sbin/service
%description servers
%if %{mono}
Ice servers: glacier2router, icebox, icegridnode, icegridregistry, 
icebox, iceboxnet, icepatch2server and related files.
%else
Ice servers: glacier2router, icebox, icegridnode, icegridregistry, 
icebox, icepatch2server and related files.
%endif

%package c++-devel
Summary: Tools, libraries and headers for developing Ice applications in C++
Group: Development/Tools
Requires: ice-libs = %{version}-%{release}
%description c++-devel
Tools, libraries and headers for developing Ice applications in C++.

%package java-devel
Summary: Tools for developing Ice applications in Java
Group: Development/Tools
Requires: ice-java = %{version}-%{release}, ice-libs = %{version}-%{release}
%description java-devel
Tools for developing Ice applications in Java.

%if %{mono}
%package csharp-devel
Summary: Tools for developing Ice applications in C#
Group: Development/Tools
Requires: ice-dotnet = %{version}-%{release}, pkgconfig
%description csharp-devel
Tools for developing Ice applications in C#.
%endif

%if %{ruby}
%package ruby
Summary: The Ice runtime for Ruby
Group: System Environment/Libraries
Requires: ice-libs = %{version}-%{release}, ruby
%description ruby
The Ice runtime for Ruby.

%package ruby-devel
Summary: Tools for developing Ice applications in Ruby
Group: Development/Tools
Requires: ice-ruby = %{version}-%{release}
%description ruby-devel
Tools for developing Ice applications in Ruby.
%endif

%package python
Summary: The Ice runtime for Python
Group: System Environment/Libraries
Requires: ice-libs = %{version}-%{release}, python >= 2.3.4
%description python
The Ice runtime for Python.

%package python-devel
Summary: Tools for developing Ice applications in Python
Group: Development/Tools
Requires: ice-python = %{version}-%{release}
%description python-devel
Tools for developing Ice applications in Python.

%package php
Summary: The Ice runtime for PHP
Group: System Environment/Libraries
Requires: ice = %{version}-%{release}
%description php
The Ice runtime for PHP.
%endif


%prep

%if !%{buildall}
%setup -n Ice-%{version} -q
%setup -q -n Ice-rpmbuild-%{version} -T -b 1
%endif

%build

#
# We build C++ all the time since we need slice2xxx
#
cd $RPM_BUILD_DIR/Ice-%{version}/cpp/src
make %{makeopts} OPTIMIZE=yes embedded_runpath_prefix=""

%ifarch %{core_arches}
cd $RPM_BUILD_DIR/Ice-%{version}/py
make %{makeopts} OPTIMIZE=yes embedded_runpath_prefix=""

cd $RPM_BUILD_DIR/Ice-%{version}/php
make %{makeopts} OPTIMIZE=yes embedded_runpath_prefix=""

%if %{ruby}
cd $RPM_BUILD_DIR/Ice-%{version}/rb
make %{makeopts} OPTIMIZE=yes embedded_runpath_prefix=""
%endif

%endif

#
# We build java5 all the time, since we include the GUI in a non-noarch package.
#
cd $RPM_BUILD_DIR/Ice-%{version}/java
export CLASSPATH=`build-classpath db-4.6.21 jgoodies-forms-1.1.0 jgoodies-looks-2.1.4 proguard`
JGOODIES_FORMS=`find-jar jgoodies-forms-1.1.0`
JGOODIES_LOOKS=`find-jar jgoodies-looks-2.1.4`

ant -Dice.mapping=java5 -Dbuild.suffix=java5 -Djgoodies.forms=$JGOODIES_FORMS -Djgoodies.looks=$JGOODIES_LOOKS jar

%ifarch noarch
ant -Dice.mapping=java2 -Dbuild.suffix=java2 jar
%if %{mono}
cd $RPM_BUILD_DIR/Ice-%{version}/cs/src
make %{makeopts} OPTIMIZE=yes
%endif
%endif

%install

rm -rf $RPM_BUILD_ROOT


#
# Arch-specific packages
#
%ifarch %{core_arches}

#
# C++
#
cd $RPM_BUILD_DIR/Ice-%{version}/cpp
make prefix=$RPM_BUILD_ROOT embedded_runpath_prefix="" install

mkdir -p $RPM_BUILD_ROOT%{_bindir}
mv $RPM_BUILD_ROOT/bin/* $RPM_BUILD_ROOT%{_bindir}
mkdir -p $RPM_BUILD_ROOT%{_libdir}
mv $RPM_BUILD_ROOT/lib/* $RPM_BUILD_ROOT%{_libdir}
mkdir -p $RPM_BUILD_ROOT%{_includedir}
mv $RPM_BUILD_ROOT/include/* $RPM_BUILD_ROOT%{_includedir}

#
# Python
#
cd $RPM_BUILD_DIR/Ice-%{version}/py
make prefix=$RPM_BUILD_ROOT embedded_runpath_prefix="" install

mkdir -p $RPM_BUILD_ROOT%{python_sitearch}/Ice
mv $RPM_BUILD_ROOT/python/* $RPM_BUILD_ROOT%{python_sitearch}/Ice

#
# PHP
#
cd $RPM_BUILD_DIR/Ice-%{version}/php
make prefix=$RPM_BUILD_ROOT install

%if "%{dist}" == ".rhel4" || "%{dist}" == ".rhel5"
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/php.d
cp -p $RPM_BUILD_DIR/Ice-rpmbuild-%{version}/ice.ini $RPM_BUILD_ROOT%{_sysconfdir}/php.d
mkdir -p $RPM_BUILD_ROOT%{_libdir}/php/modules
mv $RPM_BUILD_ROOT/lib/IcePHP.so $RPM_BUILD_ROOT%{_libdir}/php/modules
%endif

%if "%{dist}" == ".sles10"
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/php5/conf.d
mv $RPM_BUILD_ROOT/ice.ini $RPM_BUILD_ROOT%{_sysconfdir}/php5/conf.d
mkdir -p $RPM_BUILD_ROOT%{_libdir}/php5/extensions
mv $RPM_BUILD_ROOT/lib/IcePHP.so $RPM_BUILD_ROOT%{_libdir}/php5/extensions
%endif

#
# Ruby
# 
%if %{ruby}
cd $RPM_BUILD_DIR/Ice-%{version}/rb
make prefix=$RPM_BUILD_ROOT embedded_runpath_prefix="" install
mkdir -p $RPM_BUILD_ROOT%{ruby_sitearch}
mv $RPM_BUILD_ROOT/ruby/* $RPM_BUILD_ROOT%{ruby_sitearch}
%else
rm -f $RPM_BUILD_ROOT/bin/slice2rb
%endif

#
# IceGridGUI
#
mkdir -p $RPM_BUILD_ROOT%{_javadir}
cp -p $RPM_BUILD_DIR/Ice-%{version}/java/libjava5/IceGridGUI.jar $RPM_BUILD_ROOT%{_javadir}/IceGridGUI-%{version}.jar
ln -s IceGridGUI-%{version}.jar $RPM_BUILD_ROOT%{_javadir}/IceGridGUI.jar 
cp -p $RPM_BUILD_DIR/Ice-%{version}/java/bin/icegridgui.rpm $RPM_BUILD_ROOT%{_bindir}/icegridgui
mkdir -p $RPM_BUILD_ROOT%{_defaultdocdir}/Ice-%{version}/help
cp -Rp $RPM_BUILD_DIR/Ice-%{version}/java/resources/IceGridAdmin $RPM_BUILD_ROOT%{_defaultdocdir}/Ice-%{version}/help

%if %{mono}
#
# .NET spec files (for csharp-devel)
#
if test ! -d $RPM_BUILD_ROOT%{_libdir}/pkgconfig
then 
    mkdir $RPM_BUILD_ROOT%{_libdir}/pkgconfig
fi

for f in icecs glacier2cs iceboxcs icegridcs icepatch2cs icestormcs
do 
    cp $RPM_BUILD_DIR/Ice-%{version}/cs/lib/pkgconfig/$f.pc $RPM_BUILD_ROOT%{_libdir}/pkgconfig 
done
%endif

#
# initrd files (for servers)
#
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}
cp $RPM_BUILD_DIR/Ice-rpmbuild-%{version}/*.conf $RPM_BUILD_ROOT%{_sysconfdir}
mkdir -p $RPM_BUILD_ROOT%{_initrddir}
for i in icegridregistry icegridnode glacier2router
do
    cp $RPM_BUILD_DIR/Ice-rpmbuild-%{version}/$i.%{_vendor} $RPM_BUILD_ROOT%{_initrddir}/$i
done

#
# Some python scripts and related files
#
mkdir -p $RPM_BUILD_ROOT%{_datadir}
mv $RPM_BUILD_ROOT/config $RPM_BUILD_ROOT%{_datadir}/Ice-%{version}

#
# Cleanup extra files
#

rm -f $RPM_BUILD_ROOT/ICE_LICENSE
rm -f $RPM_BUILD_ROOT/LICENSE
rm -fr $RPM_BUILD_ROOT/doc/reference
rm -fr $RPM_BUILD_ROOT/slice

#temporary
rm -f $RPM_BUILD_ROOT%{_bindir}/ImportKey.class
rm -f $RPM_BUILD_ROOT%{_datadir}/Ice-%{version}/icegrid-slice.3.1.ice.gz

%if !%{mono}
rm -f $RPM_BUILD_ROOT%{_bindir}/slice2cs
%endif

%endif

#
# Arch-independent packages
#
%ifarch noarch

#
# Doc
#
mkdir -p $RPM_BUILD_ROOT%{_defaultdocdir}/Ice-%{version}
cp -p $RPM_BUILD_DIR/Ice-rpmbuild-%{version}/RELEASE_NOTES.txt $RPM_BUILD_ROOT%{_defaultdocdir}/Ice-%{version}/RELEASE_NOTES
cp -p $RPM_BUILD_DIR/Ice-rpmbuild-%{version}/README.Linux-RPM $RPM_BUILD_ROOT%{_defaultdocdir}/Ice-%{version}/README
cp -p $RPM_BUILD_DIR/Ice-rpmbuild-%{version}/THIRD_PARTY_LICENSE.Linux $RPM_BUILD_ROOT%{_defaultdocdir}/Ice-%{version}/THIRD_PARTY_LICENSE
cp -p $RPM_BUILD_DIR/Ice-rpmbuild-%{version}/SOURCES.Linux $RPM_BUILD_ROOT%{_defaultdocdir}/Ice-%{version}/SOURCES

#
# Java install (using jpackage conventions)
# 
cd $RPM_BUILD_DIR/Ice-%{version}/java
ant -Dice.mapping=java5 -Dbuild.suffix=java5 -Dprefix=$RPM_BUILD_ROOT install
ant -Dice.mapping=java2 -Dbuild.suffix=java2 -Dprefix=$RPM_BUILD_ROOT install

mkdir -p $RPM_BUILD_ROOT%{_javadir}
mv $RPM_BUILD_ROOT/lib/Ice.jar $RPM_BUILD_ROOT%{_javadir}/Ice-%{version}.jar
ln -s  Ice-%{version}.jar $RPM_BUILD_ROOT%{_javadir}/Ice.jar 
mv $RPM_BUILD_ROOT/lib/java2/Ice.jar $RPM_BUILD_ROOT%{_javadir}/Ice-java2-%{version}.jar
ln -s Ice-java2-%{version}.jar $RPM_BUILD_ROOT%{_javadir}/Ice-java2.jar


#
# License files
#
mv $RPM_BUILD_ROOT/ICE_LICENSE $RPM_BUILD_ROOT%{_defaultdocdir}/Ice-%{version}
mv $RPM_BUILD_ROOT/LICENSE $RPM_BUILD_ROOT%{_defaultdocdir}/Ice-%{version}


#
# Slice  files
#
mkdir -p $RPM_BUILD_ROOT%{_datadir}/Ice-%{version}
mv $RPM_BUILD_ROOT/slice $RPM_BUILD_ROOT%{_datadir}/Ice-%{version}

%if %{mono}
#
# DotNet
#
cd $RPM_BUILD_DIR/Ice-%{version}/cs
make NOGAC=yes prefix=$RPM_BUILD_ROOT install
%endif
%endif

#
# Cleanup extra files
#

rm -fr $RPM_BUILD_ROOT/help
rm -f $RPM_BUILD_ROOT/lib/IceGridGUI.jar

#temporary
rm -fr $RPM_BUILD_ROOT/ant

%clean
rm -rf $RPM_BUILD_ROOT

#
# noarch file packages
# 
%ifarch noarch
%files
%defattr(-, root, root, -)
%dir %{_datadir}/Ice-%{version}
%{_datadir}/Ice-%{version}/slice
%dir %{_defaultdocdir}/Ice-%{version}
%{_defaultdocdir}/Ice-%{version}

%files java
%defattr(-, root, root, -)
%{_javadir}/Ice-%{version}.jar
%{_javadir}/Ice.jar
%{_javadir}/Ice-java2-%{version}.jar
%{_javadir}/Ice-java2.jar

%if %{mono}
%files dotnet
%defattr(-, root, root, -)
%dir %{_libdir}/mono/gac/glacier2cs
%{_libdir}/mono/gac/glacier2cs/%{version}.*/
%{_libdir}/mono/glacier2cs/
%dir %{_libdir}/mono/gac/icecs
%{_libdir}/mono/gac/icecs/%{version}.*/
%{_libdir}/mono/icecs/
%dir %{_libdir}/mono/gac/iceboxcs
%{_libdir}/mono/gac/iceboxcs/%{version}.*/
%{_libdir}/mono/iceboxcs/
%dir %{_libdir}/mono/gac/icegridcs
%{_libdir}/mono/gac/icegridcs/%{version}.*/
%{_libdir}/mono/icegridcs/
%dir %{_libdir}/mono/gac/icepatch2cs
%{_libdir}/mono/gac/icepatch2cs/%{version}.*/
%{_libdir}/mono/icepatch2cs/
%dir %{_libdir}/mono/gac/icestormcs
%{_libdir}/mono/gac/icestormcs/%{version}.*/
%{_libdir}/mono/icestormcs/
%endif
%endif


#
# arch-specific packages
#
%ifarch %{core_arches}
%files libs
%defattr(-, root, root, -)
%{_libdir}/libFreeze.so.%{version}
%{_libdir}/libFreeze.so.%{soversion}
%{_libdir}/libGlacier2.so.%{version}
%{_libdir}/libGlacier2.so.%{soversion}
%{_libdir}/libIceBox.so.%{version}
%{_libdir}/libIceBox.so.%{soversion}
%{_libdir}/libIcePatch2.so.%{version}
%{_libdir}/libIcePatch2.so.%{soversion}
%{_libdir}/libIce.so.%{version}
%{_libdir}/libIce.so.%{soversion}
%{_libdir}/libIceSSL.so.%{version}
%{_libdir}/libIceSSL.so.%{soversion}
%{_libdir}/libIceStorm.so.%{version}
%{_libdir}/libIceStorm.so.%{soversion}
%{_libdir}/libIceUtil.so.%{version}
%{_libdir}/libIceUtil.so.%{soversion}
%{_libdir}/libSlice.so.%{version}
%{_libdir}/libSlice.so.%{soversion}
%{_libdir}/libIceGrid.so.%{version}
%{_libdir}/libIceGrid.so.%{soversion}

%post libs -p /sbin/ldconfig
%postun libs -p /sbin/ldconfig

%files utils
%defattr(-, root, root, -)
%{_libdir}/libIceXML.so.%{version}
%{_libdir}/libIceXML.so.%{soversion}
%{_bindir}/dumpdb
%{_bindir}/transformdb
%{_bindir}/iceboxadmin
%{_bindir}/icepatch2calc
%{_bindir}/icepatch2client
%{_bindir}/icestormadmin
%{_bindir}/slice2docbook
%{_bindir}/slice2html
%{_bindir}/icegridadmin
%{_bindir}/icegridgui
%{_bindir}/iceca
%{_javadir}/IceGridGUI-%{version}.jar
%{_javadir}/IceGridGUI.jar
%dir %{_defaultdocdir}/Ice-%{version}
%{_defaultdocdir}/Ice-%{version}/help
%dir %{_datadir}/Ice-%{version}
#%{_datadir}/Ice-%{version}/ImportKey.class
%attr(755,root,root) %{_datadir}/Ice-%{version}/convertssl.py*

%post utils -p /sbin/ldconfig
%postun utils -p /sbin/ldconfig

%files servers
%defattr(-, root, root, -)
%{_bindir}/glacier2router
%{_bindir}/icebox
%if %{mono}
%{_bindir}/iceboxnet.exe
%endif
%{_bindir}/icegridnode
%{_bindir}/icegridregistry
%{_bindir}/icepatch2server
%{_libdir}/libIceStormService.so.%{version}
%{_libdir}/libIceStormService.so.%{soversion}
%dir %{_datadir}/Ice-%{version}
%{_datadir}/Ice-%{version}/templates.xml
%attr(755,root,root) %{_datadir}/Ice-%{version}/upgradeicegrid.py*
%attr(755,root,root) %{_datadir}/Ice-%{version}/upgradeicestorm.py*
%{_initrddir}/icegridregistry
%{_initrddir}/icegridnode
%{_initrddir}/glacier2router
%config(noreplace) %{_sysconfdir}/icegridregistry.conf
%config(noreplace) %{_sysconfdir}/icegridnode.conf
%config(noreplace) %{_sysconfdir}/glacier2router.conf

%pre servers
getent group ice > /dev/null || groupadd -r iceu
getent passwd ice > /dev/null || \
        useradd -r -g ice -d %{_localstatedir}/lib/ice \
        -s /sbin/nologin -c "Ice Service account" ice
exit 0

%post servers
/sbin/ldconfig
/sbin/chkconfig --add icegridregistry
/sbin/chkconfig --add icegridnode
/sbin/chkconfig --add glacier2router

%preun servers
if [ $1 = 0 ]; then
        /sbin/service icegridregistry stop >/dev/null 2>&1 || :
        /sbin/chkconfig --del icegridregistry
        /sbin/service icegridnode stop >/dev/null 2>&1 || :
        /sbin/chkconfig --del icegridnode
        /sbin/service glacier2router stop >/dev/null 2>&1 || :
        /sbin/chkconfig --del glacier2router
fi

%postun servers
if [ "$1" -ge "1" ]; then
        /sbin/service icegridregistry condrestart >/dev/null 2>&1 || :
        /sbin/service icegridnode condrestart >/dev/null 2>&1 || :
        /sbin/service glacier2router condrestart >/dev/null 2>&1 || :
fi
/sbin/ldconfig

%files c++-devel
%defattr(-, root, root, -)

%{_bindir}/slice2cpp
%{_bindir}/slice2freeze
%{_includedir}/Freeze
%{_includedir}/Glacier2
%{_includedir}/Ice
%{_includedir}/IceBox
%{_includedir}/IceGrid
%{_includedir}/IcePatch2
%{_includedir}/IceSSL
%{_includedir}/IceStorm
%{_includedir}/IceUtil
%{_includedir}/IceXML
%{_includedir}/Slice
%{_libdir}/libFreeze.so
%{_libdir}/libGlacier2.so
%{_libdir}/libIceBox.so
%{_libdir}/libIceGrid.so
%{_libdir}/libIcePatch2.so
%{_libdir}/libIce.so
%{_libdir}/libIceSSL.so
%{_libdir}/libIceStorm.so
%{_libdir}/libIceUtil.so
%{_libdir}/libIceXML.so
%{_libdir}/libSlice.so


%if %{mono}
%files csharp-devel
%defattr(-, root, root, -)
%{_bindir}/slice2cs
%{_libdir}/pkgconfig/icecs.pc
%{_libdir}/pkgconfig/glacier2cs.pc
%{_libdir}/pkgconfig/iceboxcs.pc
%{_libdir}/pkgconfig/icegridcs.pc
%{_libdir}/pkgconfig/icepatch2cs.pc
%{_libdir}/pkgconfig/icestormcs.pc
%endif

%files java-devel
%defattr(-, root, root, -)
%{_bindir}/slice2java
%{_bindir}/slice2freezej
#%{_javadir}/ant-ice-%{version}.jar
#%{_javadir}/ant-ice.jar

%files python
%defattr(-, root, root, -)
%{python_sitearch}/Ice
#%{python_sitearch}/ice.pth

%files python-devel
%defattr(-, root, root, -)
%{_bindir}/slice2py

%if %{ruby}
%files ruby
%defattr(-, root, root, -)
%{ruby_sitearch}/*

%files ruby-devel
%defattr(-, root, root, -)
%{_bindir}/slice2rb
%endif

%files php
%defattr(-, root, root, -)

%if "%{dist}" == ".rhel4" || "%{dist}" == ".rhel5"
%{_libdir}/php/modules/IcePHP.so
%config(noreplace) %{_sysconfdir}/php.d/ice.ini
%endif

%if "%{dist}" == ".sles10"
%{_libdir}/php5/extensions
%config(noreplace) %{_sysconfdir}/php.d/ice.ini
%endif
%endif


%changelog
* Wed Feb 27 2008 Bernard Normier
- Updates for Ice 3.3.0 release

* Fri Jul 27 2007 Bernard Normier
- Updated for Ice 3.2.1 release

* Wed Jun 13 2007 Bernard Normier
- Added patch with new IceGrid.Node.AllowRunningServersAsRoot property.

* Fri Dec 6 2006 ZeroC Staff
- See source distributions or the ZeroC website for more information
  about the changes in this release



