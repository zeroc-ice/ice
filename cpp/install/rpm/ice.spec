%if "%{_target_cpu}" != "noarch" && "%{dist}" == ".rhel4"
%define ruby_included 1
%else
%define ruby_included 0
%endif

%define core_arches %{ix86} x86_64
Summary: The Ice base runtime and services
Name: ice
Version: 3.2.0
Release: 1%{?dist}
License: GPL
Group:System Environment/Libraries
Vendor: ZeroC, Inc.
URL: http://www.zeroc.com/
Source0: Ice-%{version}.tar.gz
Source1: IceJ-%{version}-java2.tar.gz
Source2: IcePy-%{version}.tar.gz
Source3: IceCS-%{version}.tar.gz
Source4: IceJ-%{version}-java5.tar.gz
Source5: IcePHP-%{version}.tar.gz
Source6: Ice-rpmbuild-%{version}.tar.gz
%if %{ruby_included}
Source7:IceRuby-%{version}.tar.gz
%endif

BuildRoot: /var/tmp/Ice-%{version}-1-buildroot

%define soversion 32
%define dotnetversion 3.2.0

%ifarch x86_64
%define icelibdir lib64
%else
%define icelibdir lib
%endif

BuildRequires: mono-core >= 1.2.2
BuildRequires: python >= 2.3.4
BuildRequires: python-devel >= 2.3.4
BuildRequires: expat >= 1.95.7
BuildRequires: libstdc++ >= 3.4.4
BuildRequires: gcc >= 3.4.4
BuildRequires: gcc-c++ >= 3.4.4
BuildRequires: tar
BuildRequires: sed
BuildRequires: binutils >= 2.15
BuildRequires: openssl >= 0.9.7a
BuildRequires: openssl-devel >= 0.9.7a
BuildRequires: bzip2 >= 1.0.2
BuildRequires: expat >= 1.95.7
BuildRequires: db45 >= 4.5.20
BuildRequires: db45-devel >= 4.5.20
%if "%{dist}" == ".rhel4"
BuildRequires: bzip2-devel >= 1.0.2
BuildRequires: expat-devel >= 1.95.7
BuildRequires: ruby >= 1.8.1
BuildRequires: ruby-devel >= 1.8.1
BuildRequires: php >= 5.1.4
BuildRequires: php-devel >= 5.1.4
%endif
%if "%{dist}" == ".sles10"
BuildRequires: php5 >= 5.1.2
BuildRequires: php5-devel >= 5.1.2
%endif

Provides: ice-%{_target_cpu}

%description
Ice is a modern alternative to object middleware such as CORBA or
COM/DCOM/COM+.  It is easy to learn, yet provides a powerful network
infrastructure for demanding technical applications. It features an
object-oriented specification language, easy to use C++, C#, Java,
Python, Ruby, PHP, and Visual Basic mappings, a highly efficient
protocol, asynchronous method invocation and dispatch, dynamic
transport plug-ins, TCP/IP and UDP/IP support, SSL-based security, a
firewall solution, and much more.
%prep


#
# C++, Java2 and C# are needed for any arch
#
%setup -n Ice-%{version} -q -T -D -b 0
%setup -q -n IceJ-%{version}-java2 -T -D -b 1
%setup -q -n IceCS-%{version} -T -D -b 3

%ifarch noarch
#
# Since we also have Java2 for IceGridGUI.jar and the demos, Java5 is purely noarch
#
%setup -q -n IceJ-%{version}-java5 -T -D -b 4
%endif

%ifarch %{core_arches}
#
# There is no noarch python, php or ruby RPM
#
%setup -q -n IcePy-%{version} -T -D -b 2
%setup -q -n IcePHP-%{version} -T -D -b 5 
%setup -c -q -n Ice-rpmbuild-%{version} -T -D -b 6
%endif

%if %{ruby_included}
%setup -q -n IceRuby-%{version} -T -D -b 7
%endif


%build

#
# We need the slice2xxx translators all the time
#
cd $RPM_BUILD_DIR/Ice-%{version}/src
make OPTIMIZE=yes embedded_runpath_prefix=""

#
# Required by non-C++ builds
#
export ICE_HOME=$RPM_BUILD_DIR/Ice-%{version} 
export LD_LIBRARY_PATH=$ICE_HOME/lib:$LD_LIBRARY_PATH

%ifarch %{core_arches}
cd $RPM_BUILD_DIR/IcePy-%{version}
make OPTIMIZE=yes embedded_runpath_prefix=""

cd $RPM_BUILD_DIR/IcePHP-%{version}
make OPTIMIZE=yes embedded_runpath_prefix=""
%endif

%if %{ruby_included}
cd $RPM_BUILD_DIR/IceRuby-%{version}
make OPTIMIZE=yes embedded_runpath_prefix=""
%endif

%ifarch noarch
#
# We only build C# for noarch
#
cd $RPM_BUILD_DIR/IceCS-%{version}/src
export PATH=$ICE_HOME/bin:$PATH
make OPTIMIZE=yes
%endif

%install

rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/lib
if test ! -d $RPM_BUILD_ROOT/%{icelibdir};
then
    mkdir -p $RPM_BUILD_ROOT/%{icelibdir}
fi

%ifarch %{core_arches}
cd $RPM_BUILD_DIR/Ice-%{version}
make prefix=$RPM_BUILD_ROOT embedded_runpath_prefix="" install
rm $RPM_BUILD_ROOT/bin/slice2vb

cd $RPM_BUILD_DIR/IcePy-%{version}
make ICE_HOME=$RPM_BUILD_DIR/Ice-%{version} prefix=$RPM_BUILD_ROOT embedded_runpath_prefix="" install

cd $RPM_BUILD_DIR/IcePHP-%{version}
make ICE_HOME=$RPM_BUILD_DIR/Ice-%{version} prefix=$RPM_BUILD_ROOT install

cp -p $RPM_BUILD_DIR/IceJ-%{version}-java2/lib/IceGridGUI.jar $RPM_BUILD_ROOT/lib/IceGridGUI.jar
cp -pR $RPM_BUILD_DIR/IceJ-%{version}-java2/ant $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/help
cp -pR $RPM_BUILD_DIR/IceJ-%{version}-java2/resources/IceGridAdmin $RPM_BUILD_ROOT/help

#
# .NET spec files (for csharp-devel)
#
if test ! -d $RPM_BUILD_ROOT/%{icelibdir}/pkgconfig ; 
then 
    mkdir $RPM_BUILD_ROOT/%{icelibdir}/pkgconfig
fi

for f in icecs glacier2cs iceboxcs icegridcs icepatch2cs icestormcs; 
do 
    cp $RPM_BUILD_DIR/IceCS-%{version}/lib/pkgconfig/$f.pc $RPM_BUILD_ROOT/%{icelibdir}/pkgconfig 
done

#
# RPM-support files
#
cp $RPM_BUILD_DIR/Ice-rpmbuild-%{version}/README.Linux-RPM $RPM_BUILD_ROOT/README
cp $RPM_BUILD_DIR/Ice-rpmbuild-%{version}/THIRD_PARTY_LICENSE.Linux $RPM_BUILD_ROOT/THIRD_PARTY_LICENSE
cp $RPM_BUILD_DIR/Ice-rpmbuild-%{version}/SOURCES.Linux $RPM_BUILD_ROOT/SOURCES
cp $RPM_BUILD_DIR/Ice-rpmbuild-%{version}/ice.ini $RPM_BUILD_ROOT/ice.ini

mkdir -p $RPM_BUILD_ROOT/etc
cp $RPM_BUILD_DIR/Ice-rpmbuild-%{version}/*.conf $RPM_BUILD_ROOT/etc
mkdir -p $RPM_BUILD_ROOT/etc/init.d
for i in icegridregistry icegridnode glacier2router
do
    cp $RPM_BUILD_DIR/Ice-rpmbuild-%{version}/$i.%{_vendor} $RPM_BUILD_ROOT/etc/init.d/$i
done
%endif

%if %{ruby_included}
cd $RPM_BUILD_DIR/IceRuby-%{version}
make OPTIMIZE=yes ICE_HOME=$RPM_BUILD_DIR/Ice-%{version} prefix=$RPM_BUILD_ROOT embedded_runpath_prefix="" install
%else
rm -f  $RPM_BUILD_ROOT/bin/slice2rb
%endif


%ifarch noarch
cp -p $RPM_BUILD_DIR/IceJ-%{version}-java5/lib/Ice.jar $RPM_BUILD_ROOT/lib/Ice.jar
mkdir -p $RPM_BUILD_ROOT/lib/java2
cp -p $RPM_BUILD_DIR/IceJ-%{version}-java2/lib/Ice.jar $RPM_BUILD_ROOT/lib/java2/Ice.jar

cd $RPM_BUILD_DIR/IceCS-%{version}
export PATH=$RPM_BUILD_DIR/Ice-%{version}/bin:$PATH
export LD_LIBRARY_PATH=$RPM_BUILD_DIR/Ice-%{version}/lib:$LD_LIBRARY_PATH
make NOGAC=yes ICE_HOME=$RPM_BUILD_DIR/Ice-%{version} prefix=$RPM_BUILD_ROOT install

for f in icecs glacier2cs iceboxcs icegridcs icepatch2cs icestormcs; 
do 
    cp $RPM_BUILD_DIR/IceCS-%{version}/bin/$f.dll $RPM_BUILD_ROOT/bin
done
%endif

#
# The following commands transform a standard Ice installation directory
# structure to a directory structure more suited to integrating into a
# Linux system.
#

mkdir -p $RPM_BUILD_ROOT/usr
mv $RPM_BUILD_ROOT/lib $RPM_BUILD_ROOT/usr/lib

if test -d $RPM_BUILD_ROOT/%{icelibdir}
then
    mv $RPM_BUILD_ROOT/%{icelibdir} ${RPM_BUILD_ROOT}%{_libdir}
fi

%ifarch %{core_arches}

#
# Move ice.ini and IcePHP.so to distribution-dependent directories
#

%if "%{dist}" == ".rhel4"
mkdir -p $RPM_BUILD_ROOT/etc/php.d
mv $RPM_BUILD_ROOT/ice.ini $RPM_BUILD_ROOT/etc/php.d
mkdir -p ${RPM_BUILD_ROOT}%{_libdir}/php/modules
mv ${RPM_BUILD_ROOT}%{_libdir}/IcePHP.so ${RPM_BUILD_ROOT}%{_libdir}/php/modules
%endif

%if "%{dist}" == ".sles10"
mkdir -p $RPM_BUILD_ROOT/etc/php5/conf.d
mv $RPM_BUILD_ROOT/ice.ini $RPM_BUILD_ROOT/etc/php5/conf.d
mkdir -p ${RPM_BUILD_ROOT}%{_libdir}/php5/extensions
mv ${RPM_BUILD_ROOT}%{_libdir}/IcePHP.so ${RPM_BUILD_ROOT}%{_libdir}/php5/extensions
%endif

mkdir -p ${RPM_BUILD_ROOT}%{_datadir}
mv $RPM_BUILD_ROOT/config ${RPM_BUILD_ROOT}%{_datadir}/Ice-%{version}
mv $RPM_BUILD_ROOT/slice ${RPM_BUILD_ROOT}%{_datadir}/Ice-%{version}

mkdir -p $RPM_BUILD_ROOT/usr
mv $RPM_BUILD_ROOT/include ${RPM_BUILD_ROOT}%{_includedir}

mkdir -p ${RPM_BUILD_ROOT}%{_libdir}/Ice-%{version}
mv $RPM_BUILD_ROOT/python ${RPM_BUILD_ROOT}%{_libdir}/Ice-%{version}/python

mkdir -p ${RPM_BUILD_ROOT}%{_defaultdocdir}
mv $RPM_BUILD_ROOT/help ${RPM_BUILD_ROOT}%{_defaultdocdir}/Ice-%{version}
mv $RPM_BUILD_ROOT/README ${RPM_BUILD_ROOT}%{_defaultdocdir}/Ice-%{version}/README
mv $RPM_BUILD_ROOT/ICE_LICENSE ${RPM_BUILD_ROOT}%{_defaultdocdir}/Ice-%{version}/ICE_LICENSE
mv $RPM_BUILD_ROOT/LICENSE ${RPM_BUILD_ROOT}%{_defaultdocdir}/Ice-%{version}/LICENSE
mv $RPM_BUILD_ROOT/THIRD_PARTY_LICENSE ${RPM_BUILD_ROOT}%{_defaultdocdir}/Ice-%{version}/THIRD_PARTY_LICENSE
mv $RPM_BUILD_ROOT/SOURCES ${RPM_BUILD_ROOT}%{_defaultdocdir}/Ice-%{version}/SOURCES

mkdir -p $RPM_BUILD_ROOT/usr/lib/Ice-%{version}
mv $RPM_BUILD_ROOT/ant $RPM_BUILD_ROOT/usr/lib/Ice-%{version}/ant
mv $RPM_BUILD_ROOT/usr/lib/IceGridGUI.jar $RPM_BUILD_ROOT/usr/lib/Ice-%{version}/IceGridGUI.jar
%endif

%if %{ruby_included}
mkdir -p ${RPM_BUILD_ROOT}%{_libdir}/Ice-%{version}
mv $RPM_BUILD_ROOT/ruby ${RPM_BUILD_ROOT}%{_libdir}/Ice-%{version}/ruby
%endif

%ifarch noarch
mkdir -p $RPM_BUILD_ROOT/usr/lib/Ice-%{version}
mv $RPM_BUILD_ROOT/usr/lib/Ice.jar $RPM_BUILD_ROOT/usr/lib/Ice-%{version}/Ice.jar
mv $RPM_BUILD_ROOT/usr/lib/java2 $RPM_BUILD_ROOT/usr/lib/Ice-%{version}/java2

mkdir -p $RPM_BUILD_ROOT/usr/lib/mono/gac/icecs/%{dotnetversion}.0__1f998c50fec78381
mv $RPM_BUILD_ROOT/bin/icecs.dll $RPM_BUILD_ROOT/usr/lib/mono/gac/icecs/%{dotnetversion}.0__1f998c50fec78381/icecs.dll

mkdir -p $RPM_BUILD_ROOT/usr/lib/mono/gac/glacier2cs/%{dotnetversion}.0__1f998c50fec78381
mv $RPM_BUILD_ROOT/bin/glacier2cs.dll $RPM_BUILD_ROOT/usr/lib/mono/gac/glacier2cs/%{dotnetversion}.0__1f998c50fec78381/glacier2cs.dll

mkdir -p $RPM_BUILD_ROOT/usr/lib/mono/gac/iceboxcs/%{dotnetversion}.0__1f998c50fec78381
mv $RPM_BUILD_ROOT/bin/iceboxcs.dll $RPM_BUILD_ROOT/usr/lib/mono/gac/iceboxcs/%{dotnetversion}.0__1f998c50fec78381/iceboxcs.dll

mkdir -p $RPM_BUILD_ROOT/usr/lib/mono/gac/icegridcs/%{dotnetversion}.0__1f998c50fec78381
mv $RPM_BUILD_ROOT/bin/icegridcs.dll $RPM_BUILD_ROOT/usr/lib/mono/gac/icegridcs/%{dotnetversion}.0__1f998c50fec78381/icegridcs.dll

mkdir -p $RPM_BUILD_ROOT/usr/lib/mono/gac/icepatch2cs/%{dotnetversion}.0__1f998c50fec78381
mv $RPM_BUILD_ROOT/bin/icepatch2cs.dll $RPM_BUILD_ROOT/usr/lib/mono/gac/icepatch2cs/%{dotnetversion}.0__1f998c50fec78381/icepatch2cs.dll

mkdir -p $RPM_BUILD_ROOT/usr/lib/mono/gac/icestormcs/%{dotnetversion}.0__1f998c50fec78381
mv $RPM_BUILD_ROOT/bin/icestormcs.dll $RPM_BUILD_ROOT/usr/lib/mono/gac/icestormcs/%{dotnetversion}.0__1f998c50fec78381/icestormcs.dll

#
# Cleanup extra files
#
rm -r $RPM_BUILD_ROOT/slice
rm    $RPM_BUILD_ROOT/config/Make.rules.cs
%endif

mkdir -p $RPM_BUILD_ROOT/usr
mv $RPM_BUILD_ROOT/bin $RPM_BUILD_ROOT/usr/bin

%clean
rm -rf ${RPM_BUILD_ROOT}

%changelog
* Fri Dec 6 2006 ZeroC Staff
- See source distributions or the ZeroC website for more information
  about the changes in this release


%ifarch %{core_arches}
%package c++-devel
Summary: Tools for developing Ice applications in C++
Group: Development/Tools
Requires: ice = %{version}
Requires: ice-%{_target_cpu}
%description c++-devel
Ice is a modern alternative to object middleware such as CORBA or
COM/DCOM/COM+.  It is easy to learn, yet provides a powerful network
infrastructure for demanding technical applications. It features an
object-oriented specification language, easy to use C++, C#, Java,
Python, Ruby, PHP, and Visual Basic mappings, a highly efficient
protocol, asynchronous method invocation and dispatch, dynamic
transport plug-ins, TCP/IP and UDP/IP support, SSL-based security, a
firewall solution, and much more.
%endif




%ifarch %{core_arches}
%package csharp-devel
Summary: Tools for developing Ice applications in C#
Group: Development/Tools
Requires: ice-dotnet = %{version}
Requires: ice-%{_target_cpu}
%description csharp-devel
Ice is a modern alternative to object middleware such as CORBA or
COM/DCOM/COM+.  It is easy to learn, yet provides a powerful network
infrastructure for demanding technical applications. It features an
object-oriented specification language, easy to use C++, C#, Java,
Python, Ruby, PHP, and Visual Basic mappings, a highly efficient
protocol, asynchronous method invocation and dispatch, dynamic
transport plug-ins, TCP/IP and UDP/IP support, SSL-based security, a
firewall solution, and much more.
%endif




%ifarch %{core_arches}
%package java-devel
Summary: Tools for developing Ice applications in Java
Group: Development/Tools
Requires: ice-java = %{version}
Requires: ice-%{_target_cpu}
%description java-devel
Ice is a modern alternative to object middleware such as CORBA or
COM/DCOM/COM+.  It is easy to learn, yet provides a powerful network
infrastructure for demanding technical applications. It features an
object-oriented specification language, easy to use C++, C#, Java,
Python, Ruby, PHP, and Visual Basic mappings, a highly efficient
protocol, asynchronous method invocation and dispatch, dynamic
transport plug-ins, TCP/IP and UDP/IP support, SSL-based security, a
firewall solution, and much more.
%endif




%ifarch %{core_arches}
%package python
Summary: The Ice runtime for Python applications
Group: System Environment/Libraries
Provides: ice-python-%{_target_cpu}
Requires: ice = %{version}, python >= 2.3.4
Requires: ice-%{_target_cpu}
%description python
Ice is a modern alternative to object middleware such as CORBA or
COM/DCOM/COM+.  It is easy to learn, yet provides a powerful network
infrastructure for demanding technical applications. It features an
object-oriented specification language, easy to use C++, C#, Java,
Python, Ruby, PHP, and Visual Basic mappings, a highly efficient
protocol, asynchronous method invocation and dispatch, dynamic
transport plug-ins, TCP/IP and UDP/IP support, SSL-based security, a
firewall solution, and much more.
%endif


%ifarch %{core_arches}
%package python-devel
Summary: Tools for developing Ice applications in Python
Group: Development/Tools
Requires: ice-python = %{version}
Requires: ice-python-%{_target_cpu}
%description python-devel
Ice is a modern alternative to object middleware such as CORBA or
COM/DCOM/COM+.  It is easy to learn, yet provides a powerful network
infrastructure for demanding technical applications. It features an
object-oriented specification language, easy to use C++, C#, Java,
Python, Ruby, PHP, and Visual Basic mappings, a highly efficient
protocol, asynchronous method invocation and dispatch, dynamic
transport plug-ins, TCP/IP and UDP/IP support, SSL-based security, a
firewall solution, and much more.
%endif




%if %{ruby_included}
%package ruby
Summary: The Ice runtime for Ruby applications
Group: System Environment/Libraries
Provides: ice-ruby-%{_target_cpu}
Requires: ice = %{version}, ruby >= 1.8.1
Requires: ice-%{_target_cpu}
%description ruby
Ice is a modern alternative to object middleware such as CORBA or
COM/DCOM/COM+.  It is easy to learn, yet provides a powerful network
infrastructure for demanding technical applications. It features an
object-oriented specification language, easy to use C++, C#, Java,
Python, Ruby, PHP, and Visual Basic mappings, a highly efficient
protocol, asynchronous method invocation and dispatch, dynamic
transport plug-ins, TCP/IP and UDP/IP support, SSL-based security, a
firewall solution, and much more.
%endif




%if %{ruby_included}
%package ruby-devel
Summary: Tools for developing Ice applications in Python
Group: Development/Tools
Requires: ice-ruby = %{version}
Requires: ice-ruby-%{_target_cpu}
%description ruby-devel
Ice is a modern alternative to object middleware such as CORBA or
COM/DCOM/COM+.  It is easy to learn, yet provides a powerful network
infrastructure for demanding technical applications. It features an
object-oriented specification language, easy to use C++, C#, Java,
Python, Ruby, PHP, and Visual Basic mappings, a highly efficient
protocol, asynchronous method invocation and dispatch, dynamic
transport plug-ins, TCP/IP and UDP/IP support, SSL-based security, a
firewall solution, and much more.
%endif




%ifarch %{core_arches}
%package php
Summary: The Ice runtime for PHP applications
Group: System Environment/Libraries
%if "%{dist}" == ".rhel4"
Requires: ice = %{version}, php >= 5.1.4
%endif
%if "%{dist}" == ".sles10"
Requires: ice-%{_target_cpu}, php5 >= 5.1.2
%endif
%description php
Ice is a modern alternative to object middleware such as CORBA or
COM/DCOM/COM+.  It is easy to learn, yet provides a powerful network
infrastructure for demanding technical applications. It features an
object-oriented specification language, easy to use C++, C#, Java,
Python, Ruby, PHP, and Visual Basic mappings, a highly efficient
protocol, asynchronous method invocation and dispatch, dynamic
transport plug-ins, TCP/IP and UDP/IP support, SSL-based security, a
firewall solution, and much more.
%endif




%ifarch noarch
%package java
Summary: The Ice runtime for Java
Group: System Environment/Libraries
Requires: ice = %{version}, db45-java >= 4.5.20
%description java
Ice is a modern alternative to object middleware such as CORBA or
COM/DCOM/COM+.  It is easy to learn, yet provides a powerful network
infrastructure for demanding technical applications. It features an
object-oriented specification language, easy to use C++, C#, Java,
Python, Ruby, PHP, and Visual Basic mappings, a highly efficient
protocol, asynchronous method invocation and dispatch, dynamic
transport plug-ins, TCP/IP and UDP/IP support, SSL-based security, a
firewall solution, and much more.
%endif




%ifarch noarch
%package dotnet
Summary: The Ice runtime for C# applications
Group: System Environment/Libraries
Requires: ice = %{version}, mono-core >= 1.2.2
%description dotnet
Ice is a modern alternative to object middleware such as CORBA or
COM/DCOM/COM+.  It is easy to learn, yet provides a powerful network
infrastructure for demanding technical applications. It features an
object-oriented specification language, easy to use C++, C#, Java,
Python, Ruby, PHP, and Visual Basic mappings, a highly efficient
protocol, asynchronous method invocation and dispatch, dynamic
transport plug-ins, TCP/IP and UDP/IP support, SSL-based security, a
firewall solution, and much more.
%endif




%ifarch %{core_arches}
%files
%defattr(644, root, root, 755)

%{_defaultdocdir}/Ice-%{version}
%attr(755, root, root) /usr/bin/dumpdb
%attr(755, root, root) /usr/bin/transformdb
%attr(755, root, root) /usr/bin/glacier2router
%attr(755, root, root) /usr/bin/icebox
%attr(755, root, root) /usr/bin/iceboxadmin
%attr(755, root, root) /usr/bin/icecpp
%attr(755, root, root) /usr/bin/icepatch2calc
%attr(755, root, root) /usr/bin/icepatch2client
%attr(755, root, root) /usr/bin/icepatch2server
%attr(755, root, root) /usr/bin/icestormadmin
%attr(755, root, root) /usr/bin/slice2docbook
%attr(755, root, root) /usr/bin/slice2html
%attr(755, root, root) /usr/bin/icegridadmin
%attr(755, root, root) /usr/bin/icegridnode
%attr(755, root, root) /usr/bin/icegridregistry
%attr(755, root, root) /usr/bin/iceca
/usr/bin/ImportKey.class
%attr(755, root, root) %{_libdir}/libFreeze.so.%{version}
%attr(755, root, root) %{_libdir}/libFreeze.so.%{soversion}
%attr(755, root, root) %{_libdir}/libGlacier2.so.%{version}
%attr(755, root, root) %{_libdir}/libGlacier2.so.%{soversion}
%attr(755, root, root) %{_libdir}/libIceBox.so.%{version}
%attr(755, root, root) %{_libdir}/libIceBox.so.%{soversion}
%attr(755, root, root) %{_libdir}/libIcePatch2.so.%{version}
%attr(755, root, root) %{_libdir}/libIcePatch2.so.%{soversion}
%attr(755, root, root) %{_libdir}/libIce.so.%{version}
%attr(755, root, root) %{_libdir}/libIce.so.%{soversion}
%attr(755, root, root) %{_libdir}/libIceSSL.so.%{version}
%attr(755, root, root) %{_libdir}/libIceSSL.so.%{soversion}
%attr(755, root, root) %{_libdir}/libIceStormService.so.%{version}
%attr(755, root, root) %{_libdir}/libIceStormService.so.%{soversion}
%attr(755, root, root) %{_libdir}/libIceStorm.so.%{version}
%attr(755, root, root) %{_libdir}/libIceStorm.so.%{soversion}
%attr(755, root, root) %{_libdir}/libIceUtil.so.%{version}
%attr(755, root, root) %{_libdir}/libIceUtil.so.%{soversion}
%attr(755, root, root) %{_libdir}/libIceXML.so.%{version}
%attr(755, root, root) %{_libdir}/libIceXML.so.%{soversion}
%attr(755, root, root) %{_libdir}/libSlice.so.%{version}
%attr(755, root, root) %{_libdir}/libSlice.so.%{soversion}
%attr(755, root, root) %{_libdir}/libIceGrid.so.%{version}
%attr(755, root, root) %{_libdir}/libIceGrid.so.%{soversion}
%dir /usr/lib/Ice-%{version}
/usr/lib/Ice-%{version}/IceGridGUI.jar
%dir %{_datadir}/Ice-%{version}
%{_datadir}/Ice-%{version}/slice
%{_datadir}/Ice-%{version}/templates.xml
%attr(755, root, root) %{_datadir}/Ice-%{version}/convertssl.py
%attr(755, root, root) %{_datadir}/Ice-%{version}/upgradeicegrid.py
%attr(755, root, root) %{_datadir}/Ice-%{version}/upgradeicestorm.py
%{_datadir}/Ice-%{version}/icegrid-slice.3.1.ice.gz
%attr(755, root, root) /etc/init.d/icegridregistry
%attr(755, root, root) /etc/init.d/icegridnode
%attr(755, root, root) /etc/init.d/glacier2router
/etc/icegridregistry.conf
/etc/icegridnode.conf
/etc/glacier2router.conf

%post
%postun


%else
%files

%endif


%ifarch %{core_arches}
%files c++-devel
%defattr(644, root, root, 755)

%attr(755, root, root) /usr/bin/slice2cpp
%attr(755, root, root) /usr/bin/slice2freeze
%{_includedir}
%attr(755, root, root) %{_libdir}/libFreeze.so
%attr(755, root, root) %{_libdir}/libGlacier2.so
%attr(755, root, root) %{_libdir}/libIceBox.so
%attr(755, root, root) %{_libdir}/libIceGrid.so
%attr(755, root, root) %{_libdir}/libIcePatch2.so
%attr(755, root, root) %{_libdir}/libIce.so
%attr(755, root, root) %{_libdir}/libIceSSL.so
%attr(755, root, root) %{_libdir}/libIceStormService.so
%attr(755, root, root) %{_libdir}/libIceStorm.so
%attr(755, root, root) %{_libdir}/libIceUtil.so
%attr(755, root, root) %{_libdir}/libIceXML.so
%attr(755, root, root) %{_libdir}/libSlice.so


%post c++-devel
%postun c++-devel


%endif


%ifarch %{core_arches}
%files csharp-devel
%defattr(644, root, root, 755)

%attr(755, root, root) /usr/bin/slice2cs
%dir %{_libdir}/pkgconfig
%{_libdir}/pkgconfig/icecs.pc
%{_libdir}/pkgconfig/glacier2cs.pc
%{_libdir}/pkgconfig/iceboxcs.pc
%{_libdir}/pkgconfig/icegridcs.pc
%{_libdir}/pkgconfig/icepatch2cs.pc
%{_libdir}/pkgconfig/icestormcs.pc


%post csharp-devel

%ifnarch noarch

pklibdir="lib"

%ifarch x86_64
pklibdir="lib64"
%endif
        
%endif
%postun csharp-devel


%endif


%ifarch %{core_arches}
%files java-devel
%defattr(644, root, root, 755)

%attr(755, root, root) /usr/bin/slice2java
%attr(755, root, root) /usr/bin/slice2freezej
%dir /usr/lib/Ice-%{version}
/usr/lib/Ice-%{version}/ant


%post java-devel
%postun java-devel


%endif


%ifarch %{core_arches}
%files python
%defattr(644, root, root, 755)

%dir %{_libdir}/Ice-%{version}
%{_libdir}/Ice-%{version}/python


%post python
%postun python


%endif


%ifarch %{core_arches}
%files python-devel
%defattr(644, root, root, 755)

%attr(755, root, root) /usr/bin/slice2py


%post python-devel
%postun python-devel


%endif


%if %{ruby_included}
%files ruby
%defattr(644, root, root, 755)

%dir %{_libdir}/Ice-%{version}
%{_libdir}/Ice-%{version}/ruby


%post ruby
%postun ruby


%endif


%if %{ruby_included}
%files ruby-devel
%defattr(644, root, root, 755)

%attr(755, root, root) /usr/bin/slice2rb


%post ruby-devel
%postun ruby-devel


%endif


%ifarch %{core_arches}
%files php
%defattr(644, root, root, 755)

%if "%{dist}" == ".rhel4"
%attr(755, root, root) %{_libdir}/php/modules
/etc/php.d/ice.ini
%endif

%if "%{dist}" == ".sles10"
%attr(755, root, root) %{_libdir}/php5/extensions
/etc/php5/conf.d/ice.ini
%endif

%post php
%postun php


%endif


%ifarch noarch
%files java
%defattr(644, root, root, 755)

%dir /usr/lib/Ice-%{version}
/usr/lib/Ice-%{version}/Ice.jar
%dir /usr/lib/Ice-%{version}/java2
/usr/lib/Ice-%{version}/java2/Ice.jar


%post java
%postun java


%endif


%ifarch noarch
%files dotnet
%defattr(644, root, root, 755)

%dir /usr/lib/mono/gac/glacier2cs
%dir /usr/lib/mono/gac/glacier2cs/%{dotnetversion}.0__1f998c50fec78381
/usr/lib/mono/gac/glacier2cs/%{dotnetversion}.0__1f998c50fec78381/glacier2cs.dll
%dir /usr/lib/mono/gac/icecs
%dir /usr/lib/mono/gac/icecs/%{dotnetversion}.0__1f998c50fec78381
/usr/lib/mono/gac/icecs/%{dotnetversion}.0__1f998c50fec78381/icecs.dll
%dir /usr/lib/mono/gac/iceboxcs
%dir /usr/lib/mono/gac/iceboxcs/%{dotnetversion}.0__1f998c50fec78381
/usr/lib/mono/gac/iceboxcs/%{dotnetversion}.0__1f998c50fec78381/iceboxcs.dll
%dir /usr/lib/mono/gac/icegridcs
%dir /usr/lib/mono/gac/icegridcs/%{dotnetversion}.0__1f998c50fec78381
/usr/lib/mono/gac/icegridcs/%{dotnetversion}.0__1f998c50fec78381/icegridcs.dll
%dir /usr/lib/mono/gac/icepatch2cs
%dir /usr/lib/mono/gac/icepatch2cs/%{dotnetversion}.0__1f998c50fec78381
/usr/lib/mono/gac/icepatch2cs/%{dotnetversion}.0__1f998c50fec78381/icepatch2cs.dll
%dir /usr/lib/mono/gac/icestormcs
%dir /usr/lib/mono/gac/icestormcs/%{dotnetversion}.0__1f998c50fec78381
/usr/lib/mono/gac/icestormcs/%{dotnetversion}.0__1f998c50fec78381/icestormcs.dll
%attr(755, root, root) /usr/bin/iceboxnet.exe


%post dotnet
%postun dotnet


%endif
