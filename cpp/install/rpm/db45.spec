%define java_arches %{ix86} x86_64

#
# TODO: Should fail when other installed Berkeley DB might collide. e.g.
# installs libdb-4.5.so too..
#

%{!?_lib:%define _lib	lib}

%define	__soversion	4.5
%define	_libdb_a	libdb-%{__soversion}.a
%define	_libcxx_a	libdb_cxx-%{__soversion}.a

Summary: The Berkeley DB database library (version 4.5) for C/C++
Name: db45
Version: 4.5.20
Release: 9%{?dist}
Source0: http://download.oracle.com/berkeley-db/db-%{version}.NC.tar.gz
URL: http://www.oracle.com/database/berkeley-db/index.html
License: GPL
Group: System Environment/Libraries
BuildRequires: perl, libtool, ed, 
%ifarch %{java_arches}
%endif
BuildRoot: %{_tmppath}/%{name}-root

%description
The Berkeley Database (Berkeley DB) is a programmatic toolkit that
provides embedded database support for both traditional and
client/server applications. The Berkeley DB includes B+tree, Extended
Linear Hashing, Fixed and Variable-length record access methods,
transactions, locking, logging, shared memory caching, and database
recovery. The Berkeley DB supports C, C++, Java, and Perl APIs. It is
used by many applications, including Python and Perl, so this should
be installed on all systems.

%package utils
Summary: Command line tools for managing Berkeley DB (version 4.5) databases.
Group: Applications/Databases
Requires: db45 = %{version}-%{release}

%description utils
The Berkeley Database (Berkeley DB) is a programmatic toolkit that
provides embedded database support for both traditional and
client/server applications. Berkeley DB includes B+tree, Extended
Linear Hashing, Fixed and Variable-length record access methods,
transactions, locking, logging, shared memory caching, and database
recovery. DB supports C, C++, Java and Perl APIs.

This package contains command line tools for managing Berkeley DB
(version 4) databases.

%package devel
Summary: Development files for the Berkeley DB (version 4.5) library.
Group: Development/Libraries
Requires: db45 = %{version}-%{release}

%description devel
The Berkeley Database (Berkeley DB) is a programmatic toolkit that
provides embedded database support for both traditional and
client/server applications. This package contains the header files,
libraries, and documentation for building programs which use the
Berkeley DB.

%package java
Summary: Development files for using the Berkeley DB (version 4.5) with Java.
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description java
The Berkeley Database (Berkeley DB) is a programmatic toolkit that
provides embedded database support for both traditional and
client/server applications. This package contains the libraries
for building programs which use the Berkeley DB in Java.

%prep
%setup -q -n db-4.5.20.NC 

cd dist
./s_config
cd ..
CFLAGS="$RPM_OPT_FLAGS -fno-strict-aliasing"; export CFLAGS

pushd build_unix 
../dist/configure -C \
	--enable-shared --enable-static --enable-rpc \
	--enable-cxx \
%ifarch %{java_arches}
	--enable-java \
%else
	--disable-java \
%endif

make   %{?_smp_mflags} \
	 LIBSO_LIBS='$(LIBS)' \
	LIBXSO_LIBS='$(LIBS)' \
	LIBJSO_LIBS='$(LIBS)' \
	LIBTSO_LIBS='$(LIBS)'

popd

%install
rm -rf ${RPM_BUILD_ROOT}
mkdir -p ${RPM_BUILD_ROOT}%{_includedir}
mkdir -p ${RPM_BUILD_ROOT}%{_libdir}

%makeinstall -C build_unix

# Nuke non-versioned archives and symlinks
rm -f ${RPM_BUILD_ROOT}%{_libdir}/{libdb.a,libdb_cxx.a}
rm -f ${RPM_BUILD_ROOT}%{_libdir}/libdb-4.so
rm -f ${RPM_BUILD_ROOT}%{_libdir}/libdb_cxx-4.so

chmod +x ${RPM_BUILD_ROOT}%{_libdir}/*.so*

# Move the main shared library from /usr/lib* to /lib* directory.
if [ "%{_libdir}" != "/%{_lib}" ]; then
  mkdir -p $RPM_BUILD_ROOT/%{_lib}/
  mv $RPM_BUILD_ROOT/%{_libdir}/libdb-%{__soversion}.so $RPM_BUILD_ROOT/%{_lib}/

# Leave relative symlinks in %{_libdir}.
  touch $RPM_BUILD_ROOT/rootfile
  root=..
  while [ ! -e $RPM_BUILD_ROOT/%{_libdir}/${root}/rootfile ] ; do
    root=${root}/..
  done
  rm $RPM_BUILD_ROOT/rootfile

  mkdir -p ${RPM_BUILD_ROOT}%{_libdir}/db45
  ln -sf ${root}/../%{_lib}/libdb-%{__soversion}.so $RPM_BUILD_ROOT/%{_libdir}/db45/libdb.so
  ln -sf ${root}/%{_lib}/libdb-%{__soversion}.so $RPM_BUILD_ROOT/%{_libdir}/
  ln -sf ${root}/..%{_libdir}/libdb_cxx-%{__soversion}.so $RPM_BUILD_ROOT/%{_libdir}/db45/libdb_cxx.so
fi

# Move the header files to a subdirectory, in case we're deploying on a
# system with multiple versions of DB installed.
mkdir -p ${RPM_BUILD_ROOT}%{_includedir}/db45
mv ${RPM_BUILD_ROOT}%{_includedir}/*.h ${RPM_BUILD_ROOT}%{_includedir}/db45/

%ifarch %{java_arches}
# Move java jar file to the correct place
mkdir -p ${RPM_BUILD_ROOT}%{_datadir}/java/db45
mv ${RPM_BUILD_ROOT}%{_libdir}/*.jar ${RPM_BUILD_ROOT}%{_datadir}/java/db45
%endif

rm -f ${RPM_BUILD_ROOT}%{_libdir}/libdb*.a
rm -f ${RPM_BUILD_ROOT}%{_libdir}/libdb*.la

mkdir -p ${RPM_BUILD_ROOT}%{_bindir}/db45
mv ${RPM_BUILD_ROOT}%{_bindir}/db_* ${RPM_BUILD_ROOT}%{_bindir}/db45
mv ${RPM_BUILD_ROOT}%{_bindir}/berkeley_db* ${RPM_BUILD_ROOT}%{_bindir}/db45

#
# We are not packaging any docs, so remove all of that.
#
rm -rf ${RPM_BUILD_ROOT}%{_prefix}/docs

%clean
rm -rf ${RPM_BUILD_ROOT}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%post -p /sbin/ldconfig java

%postun -p /sbin/ldconfig java

%files
%defattr(-,root,root)
%doc LICENSE README
/%{_lib}/libdb-%{__soversion}.so
%{_libdir}/libdb_cxx-%{__soversion}.so
%{_libdir}/libdb-%{__soversion}.so

%files utils
%{_bindir}/db45/berkeley_db*_svc
%{_bindir}/db45/db*_archive
%{_bindir}/db45/db*_checkpoint
%{_bindir}/db45/db*_deadlock
%{_bindir}/db45/db*_dump*
%{_bindir}/db45/db*_load
%{_bindir}/db45/db*_printlog
%{_bindir}/db45/db*_recover
%{_bindir}/db45/db*_stat
%{_bindir}/db45/db*_upgrade
%{_bindir}/db45/db*_verify
%{_bindir}/db45/db*_hotbackup

%files devel
%defattr(-,root,root)
%{_includedir}/%{name}
%{_libdir}/db45/libdb.so
%{_libdir}/db45/libdb_cxx.so

%ifarch %{java_arches}
%files java
%defattr(-,root,root)
%{_libdir}/libdb_java-4.5.so
%{_libdir}/libdb_java-4.5_g.so
%{_datadir}/java/db45/*.jar
%endif

%changelog
* Tue Dec 5 2006 ZeroC,Inc <info@zeroc.com> 4.5.20-1 
- Created spec file for Berkeley DB 4.5 for RPMs that will not interfere
  with pre-existing db4 installations.
