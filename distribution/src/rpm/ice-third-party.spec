%define _unpackaged_files_terminate_build 0

Summary: Third party JAR files used by the IceGrid Admin GUI
Name: ice-third-party
Version: 3.2.1
Release: 1
License: GPL
Group:System Environment/Libraries
Vendor: ZeroC, Inc
URL: http://www.zeroc.com/
Source0: http://www.zeroc.com/download/Ice/3.2/forms-1.0.7.jar
Source1: http://www.zeroc.com/download/Ice/3.2/looks-2.0.4.jar

%description
Ice is a modern alternative to object middleware
such as CORBA or COM/DCOM/COM+.  It is easy to learn, yet provides a
powerful network infrastructure for demanding technical applications. It
features an object-oriented specification language, easy to use C++,
Java, Python, PHP, C#, and Visual Basic mappings, a highly efficient
protocol, asynchronous method invocation and dispatch, dynamic transport
plug-ins, TCP/IP and UDP/IP support, SSL-based security, a firewall
solution, and much more. This package contains third party libraries
that are not currently available in RPM form but are required to get
full functionality from Ice.

%prep

%build

%install
mkdir -p $RPM_BUILD_DIR/usr/share/java
cp $RPM_SOURCE_DIR/forms-1.0.7.jar $RPM_BUILD_DIR/usr/share/java
cp $RPM_SOURCE_DIR/looks-2.0.4.jar $RPM_BUILD_DIR/usr/share/java

%clean

%changelog
* Fri Jan 10 2007 ZeroC Staff
- See JGoodies website for details. 

%files
%defattr(644, root, root, 755)
/usr/share/java/forms-1.0.7.jar
/usr/share/java/looks-2.0.4.jar

