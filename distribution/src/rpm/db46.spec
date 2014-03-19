#
# Copyright (c) 2007-2014 ZeroC, Inc. All rights reserved.
#
# This is a heavily modified version of the spec file in Fedora Core 7's db4-4.5.20-4.fc7.src.rpm   
#

# We build using Sun's JDK, not gcj
%define java_arches %{ix86} alpha ia64 ppc s390 s390x sparc sparcv9 x86_64

%define	__soversion	4.6

Summary: The Berkeley DB database library (version 4.6) for C and C++
Name: db46
Version: 4.6.21
Release: 3ice%{?dist}
Source0: http://download.oracle.com/berkeley-db/db-%{version}.NC.tar.gz
Patch1: http://www.oracle.com/technology/products/berkeley-db/db/update/4.6.21/patch.4.6.21.1
Patch2: http://www.oracle.com/technology/products/berkeley-db/db/update/4.6.21/patch.4.6.21.2
Patch3: http://www.oracle.com/technology/products/berkeley-db/db/update/4.6.21/patch.4.6.21.3
Patch4: patch.4.6.21.15663
URL: http://www.oracle.com/database/berkeley-db/
License: Berkeley DB open-source license
Group: System Environment/Libraries
Packager: ZeroC, Inc.
BuildRequires: perl, libtool
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

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
Summary: Command line tools for managing Berkeley DB (version 4.6) databases
Group: Applications/Databases
Requires: db46 = %{version}-%{release}

%description utils
The Berkeley Database (Berkeley DB) is a programmatic toolkit that
provides embedded database support for both traditional and
client/server applications. Berkeley DB includes B+tree, Extended
Linear Hashing, Fixed and Variable-length record access methods,
transactions, locking, logging, shared memory caching, and database
recovery. DB supports C, C++, Java and Perl APIs.

This package contains command line tools for managing Berkeley DB
(version 4.6) databases.

%package devel
Summary: Development files for the Berkeley DB (version 4.6) library
Group: Development/Libraries
Requires: db46 = %{version}-%{release}
Obsoletes: db1-devel, db2-devel, db3-devel

%description devel
The Berkeley Database (Berkeley DB) is a programmatic toolkit that
provides embedded database support for both traditional and
client/server applications. This package contains the header files,
libraries, and documentation for building programs which use the
Berkeley DB.


%package java
Summary: Development files for using the Berkeley DB (version 4.6) with Java
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description java
The Berkeley Database (Berkeley DB) is a programmatic toolkit that
provides embedded database support for both traditional and
client/server applications. This package contains the libraries
for building programs which use the Berkeley DB in Java.

%prep

%setup -q -n db-%{version}.NC
%patch1 -p0 -b .4.6.21.1
%patch2 -p0 -b .4.6.21.2
%patch3 -p0 -b .4.6.21.3
%patch4 -p0 -b .4.6.21.15663

# Remove tags files which we don't need.
find . -name tags | xargs rm -f

cd dist
./s_config

%build
cd build_unix

../dist/configure --prefix=%{_prefix} \
                  --libdir=%{_libdir} \
                  --enable-shared --disable-static \
		  --enable-cxx \
%ifarch %{java_arches}
		  --enable-java 
%else
		  --disable-java
%endif

# Remove libtool predep_objects and postdep_objects wonkiness so that
# building without -nostdlib doesn't include them twice.  Because we
# already link with g++, weird stuff happens if you don't let the
# compiler handle this.
perl -pi -e 's/^predep_objects=".*$/predep_objects=""/' libtool
perl -pi -e 's/^postdep_objects=".*$/postdep_objects=""/' libtool
perl -pi -e 's/-shared -nostdlib/-shared/' libtool
make LIBSO_LIBS='$(LIBS)' LIBXSO_LIBS='$(LIBS)'" -L%{_libdir} -lstdc++"

%install
rm -rf ${RPM_BUILD_ROOT}
mkdir -p ${RPM_BUILD_ROOT}%{_includedir}/%{name}
mkdir -p ${RPM_BUILD_ROOT}%{_libdir}/%{name}

cd build_unix
make prefix=${RPM_BUILD_ROOT}%{_prefix} libdir=${RPM_BUILD_ROOT}%{_libdir} install
cd ..

# XXX Nuke non-versioned archives and symlinks
rm -f ${RPM_BUILD_ROOT}%{_libdir}/libdb.so
rm -f ${RPM_BUILD_ROOT}%{_libdir}/libdb-4.so
rm -f ${RPM_BUILD_ROOT}%{_libdir}/libdb_cxx.so
rm -f ${RPM_BUILD_ROOT}%{_libdir}/libdb_cxx-4.so
rm -f ${RPM_BUILD_ROOT}%{_libdir}/libdb_tcl-4.so
rm -f ${RPM_BUILD_ROOT}%{_libdir}/libdb_tcl.so
rm -f ${RPM_BUILD_ROOT}%{_libdir}/libdb*.la

chmod +x ${RPM_BUILD_ROOT}%{_libdir}/*.so*

# Move the header files to a subdirectory, in case we're deploying on a
# system with multiple versions of DB installed.
mv ${RPM_BUILD_ROOT}%{_includedir}/*.h ${RPM_BUILD_ROOT}%{_includedir}/%{name}

# Rename the db_ utils files
cd ${RPM_BUILD_ROOT}%{_bindir}
for i in db_* ; do mv $i db46_${i#db_} ; done

ln -sf ../libdb-%{__soversion}.so ${RPM_BUILD_ROOT}%{_libdir}/%{name}/libdb.so
ln -sf ../libdb_cxx-%{__soversion}.so ${RPM_BUILD_ROOT}%{_libdir}/%{name}/libdb_cxx.so

%ifarch %{java_arches}
# Move java jar file to the correct place
mkdir -p ${RPM_BUILD_ROOT}%{_javadir}
mv ${RPM_BUILD_ROOT}%{_libdir}/db.jar ${RPM_BUILD_ROOT}%{_javadir}/db-%{version}.jar
rm -f ${RPM_BUILD_ROOT}%{_libdir}/libdb_java-4.so ${RPM_BUILD_ROOT}%{_libdir}/libdb_java.so
%endif

# Eliminate installed docs
rm -rf ${RPM_BUILD_ROOT}%{_prefix}/docs

# XXX Avoid Permission denied. strip when building as non-root.
chmod u+w ${RPM_BUILD_ROOT}%{_bindir}

%clean
rm -rf ${RPM_BUILD_ROOT}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
%doc LICENSE README
%{_libdir}/libdb-%{__soversion}.so
%{_libdir}/libdb_cxx-%{__soversion}.so

%files utils
%defattr(-,root,root)
%{_bindir}/db*_archive
%{_bindir}/db*_checkpoint
%{_bindir}/db*_codegen
%{_bindir}/db*_deadlock
%{_bindir}/db*_dump
%{_bindir}/db*_load
%{_bindir}/db*_printlog
%{_bindir}/db*_recover
%{_bindir}/db*_stat
%{_bindir}/db*_upgrade
%{_bindir}/db*_verify
%{_bindir}/db*_hotbackup

%files devel
%defattr(-,root,root)
%dir %{_libdir}/%{name}
%{_libdir}/%{name}/libdb.so
%{_libdir}/%{name}/libdb_cxx.so
%dir %{_includedir}/%{name}
%{_includedir}/%{name}/*.h


%ifarch %{java_arches}
%files java
%defattr(-,root,root)
%{_libdir}/libdb_java*
%{_javadir}/db-%{version}.jar
%endif

%changelog

* Mon Jan 26 2009 Bernard Normier <bernard@zeroc.com> 4.6.21-3ice
- Applied DB patches

* Wed Feb 27 2008 Bernard Normier <bernard@zeroc.com> 4.6.21-2ice
- Moved db.jar to a jpackage-compliant location

* Thu Feb 7 2008 Bernard Normier <bernard@zeroc.com> 4.6.21-1ice
- Updated Berkeley DB to version 4.6.21

* Fri Jul 27 2007 Bernard Normier <bernard@zeroc.com> 4.5.20-5ice
- Added db_hotbackup patch 15395-2 for Ice 3.2.1 release

* Mon Feb 12 2007 Bernard Normier <bernard@zeroc.com> 4.5.20-4ice
- Heavily modified (simplified) for Ice 3.2 release

* Mon Dec  4 2006 Jindrich Novy <jnovy@redhat.com> 4.5.20-4
- apply upstream patches for 4.5.20
  (Java API <-> core API related fixes)

* Fri Dec  1 2006 Jindrich Novy <jnovy@redhat.com> 4.5.20-3
- temporarily remove ppc64 from java arches

* Sun Nov 26 2006 Jindrich Novy <jnovy@redhat.com> 4.5.20-2
- sync db4 and compat-db licenses to BSD-style as the result of
  consultation with legal department
- fix some rpmlint warnings

* Fri Nov 10 2006 Jindrich Novy <jnovy@redhat.com> 4.5.20-1
- update to db-4.5.20 (#198038)
- fix BuildRoot
- drop .64bit patch
- patch/source URLs now point to correct location

* Tue Sep 12 2006 Jindrich Novy <jnovy@redhat.com> 4.3.29-9
- rebuild

* Wed Sep  6 2006 Jindrich Novy <jnovy@redhat.com> 4.3.29-8
- revert the previous fix, it crashes OOo help

* Sun Sep  3 2006 Jindrich Novy <jnovy@redhat.com> 4.3.29-7.fc6
- fix memleak caused by SET_TXN macro in xa_db.c, when opening
  database created with DB_XA_CREATE flag (#204920)

* Wed Jul 19 2006 Jindrich Novy <jnovy@redhat.com> 4.3.29-6
- fix sparc64 build (#199358)

* Mon Jul 17 2006 Jindrich Novy <jnovy@redhat.com> 4.3.29-5
- rebuild because of gnu_hash

* Wed Jul 12 2006 Jesse Keating <jkeating@redhat.com> - 4.3.29-4.1
- rebuild

* Fri Mar 24 2006 Jindrich Novy <jnovy@redhat.com> 4.3.29-4
- drop useless java, lfs patches

* Mon Mar 13 2006 Jindrich Novy <jnovy@redhat.com> 4.3.29-3
- apply x86_64 fix from Henrik Nordstrom (#184588)
- don't nuke non-versioned archives twice

* Wed Feb 15 2006 Jindrich Novy <jnovy@redhat.com> 4.3.29-2
- don't package /usr/share/doc/images in the main db4 package
  and move it to db4-devel (#33328)
- make db4 LFS capable (#33849)
- move db4-devel, db4-tcl, db4-java to Development/Libraries
  group instead of System Environment/Libraries (#54320)
- BuildPrereq -> BuildRequires
- don't use RPM_SOURCE_DIR
- Obsoletes db3, db2

* Fri Feb 10 2006 Jesse Keating <jkeating@redhat.com> - 4.3.29-1.2.1
- bump again for double-long bug on ppc(64)

* Tue Feb 07 2006 Jesse Keating <jkeating@redhat.com> - 4.3.29-1.2
- rebuilt for new gcc4.1 snapshot and glibc changes

* Fri Dec 09 2005 Jesse Keating <jkeating@redhat.com>
- rebuilt

* Fri Oct 07 2005 Paul Nasrat <pnasrat@redhat.com> 4.3.29-1
- New upstream release

* Fri Sep 30 2005 Paul Nasrat <pnasrat@redhat.com> 4.3.28-4
- Re-enable java for ppc64

* Wed Sep 21 2005 Paul Nasrat <pnasrat@redhat.com> 4.3.28-3
- Add fno-strict-aliasing for java (#168965)

* Tue Sep 20 2005 Paul Nasrat <pnasrat@redhat.com> 4.3.28-2
- no java for ppc64 for now (#166657)

* Tue Sep 20 2005 Paul Nasrat <pnasrat@redhat.com> 4.3.28-1
- FC5 is nptl only (derived from jbj's spec)
- upgrade to 4.3.28

* Thu Jul 14 2005 Paul Nasrat <pnasrat@redhat.com> 4.3.27-5
- re-enable db4-java

* Tue May 17 2005 Paul Nasrat <pnasrat@redhat.com> 4.3.27-4
- /usr/lib/tls/ix86 dirs (#151371)

* Mon Apr 25 2005 Bill Nottingham <notting@redhat.com> 4.3.27-3
- add libdb_cxx.so link (#149191)

* Fri Mar  4 2005 Jeff Johnson <jbj@jbj.org> 4.3.27-2
- rebuild with gcc4.

* Sat Jan  1 2005 Jeff Johnson <jbj@jbj.org> 4.3.27-1
- upgrade to 4.3.27.

* Thu Nov 11 2004 Jeff Johnson <jbj@jbj.org> 4.3.21-1
- upgrade to 4.3.21, no db4-java for the moment again again.

* Tue Sep 21 2004 Nalin Dahyabhai <nalin@redhat.com> 4.2.52-6
- on %%{ix86} systems, make the availability of an NPTL-requiring libdb match
  the availability of an NPTL libpthread in glibc > 2.3.3-48
- run ldconfig in db4-java's %%post/%%postun
- when building java support, assume that libgcj is equivalent enough to 1.3

* Tue Jun 15 2004 Elliot Lee <sopwith@redhat.com>
- rebuilt

* Mon Jun  7 2004 Jeff Johnson <jbj@jbj.org> 4.2.52-4
- remove dangling symlinks (#123721 et al).
- remove db_cxx.so and db_tcl.so symlinks, versioned equivs exist.
- apply 2 patches from sleepycat.
- resurrect db4-java using sun jvm-1.4.2.
- cripple autoconf sufficiently to build db4-java with gcj, without jvm.
- check javac first, gcj34 next, then gcj-ssa, finally gcj.
- add ed build dependency (#125180).

* Tue Mar 02 2004 Elliot Lee <sopwith@redhat.com>
- rebuilt

* Fri Feb 13 2004 Elliot Lee <sopwith@redhat.com>
- rebuilt

* Fri Jan 16 2004 Jeff Johnson <jbj@jbj.org> 4.2.52-2
- fix: automake *.lo wrapper, not elf, files included in *.a (#113572).

* Thu Dec 11 2003 Jeff Johnson <jbj@jbj.org> 4.2.52-1
- upgrade to db-4.2.52, no db4-java for the moment.

* Fri Nov 28 2003 Paul Nasrat <pauln@truemesh.com> 4.2.41-0.2
- Add build requires tcl-devel

* Fri Oct 24 2003 Nalin Dahyabhai <nalin@redhat.com> 4.1.25-14
- symlink from %%{_libdir}/tls/libdb-4.1.so to the copy in /%%{_lib}/tls, so
  that the run-time linker can find the right copy for of apps which use an
  RPATH to point at %%{_libdir}/libdb-4.1.so

* Thu Oct 23 2003 Nalin Dahyabhai <nalin@redhat.com> 4.1.25-13
- add another section to the ABI note for the TLS libdb so that it's marked as
  not needing an executable stack (from Arjan Van de Ven)

* Wed Oct 15 2003 Nalin Dahyabhai <nalin@redhat.com>
- build both with and without support for shared mutex locks, which require NPTL
- make behavior wrt where we put libdb the same for all OSs
- revert changes making tcl optional - nesting %%if tcl and %%ifarch nptl
  doesn't work
- fix dangling HREFs in utility docs (pointed to main docs dir, while they're
  actually in the -utils docs dir)
- run ldconfig when installing/removing the -utils subpackage, as it contains
  shared libraries

* Wed Oct 15 2003 Nalin Dahyabhai <nalin@redhat.com> 4.1.25-11
- fix multiple-inclusion problem of startup files when building shlibs without
  the -nostdlib flag

* Tue Oct 14 2003 Nalin Dahyabhai <nalin@redhat.com>
- link shared libraries without -nostdlib, which created an unresolvable dep
  on a hidden symbol

* Tue Sep 23 2003 Florian La Roche <Florian.LaRoche@redhat.de>
- allow compiling without tcl dependency

* Sat Sep 20 2003 Jeff Johnson <jbj@jbj.org> 4.2.42-0.1
- update to 4.2.42.
- build in build_unix subdir.
- eliminate --enable-dump185, db_dump185.c no longer compiles for libdb*.
- create db4-tcl sub-pkg to isolate libtcl dependencies.

* Thu Aug 21 2003 Nalin Dahyabhai <nalin@redhat.com> 4.1.25-9
- rebuild

* Tue Aug 19 2003 Nalin Dahyabhai <nalin@redhat.com> 4.1.25-8
- add missing tcl-devel buildrequires (#101814)

* Tue Jul 15 2003 Joe Orton <jorton@redhat.com> 4.1.25-7
- rebuild

* Fri Jun 27 2003 Jeff Johnson <jbj@redhat.com> 4.1.25-6
- build with libtool-1.5, which can't recognize the .so in libfoo*.so atm.
- whack out libtool predep_objects wonkiness.

* Thu Jun 26 2003 Jeff Johnson <jbj@redhat.com> 4.1.25-5
- rebuild.

* Tue Jun 24 2003 Jeff Johnson <jbj@redhat.com> 4.1.25-4
- hack out O_DIRECT support in db4 for now.

* Tue Jun 24 2003 Nalin Dahyabhai <nalin@redhat.com>
- replace libtool.ac with current libtool.m4 so that running libtoolize doesn't
  cause a mismatch

* Wed Jun 04 2003 Elliot Lee <sopwith@redhat.com>
- rebuilt

* Wed Jun  4 2003 Nalin Dahyabhai <nalin@redhat.com> 4.1.25-2
- change configure to only warn if JNI includes aren't found, assuming that
  the C compiler can find them
- remove build requirement on jdkgcj -- gcj is sufficient

* Mon May  5 2003 Jeff Johnson <jbj@redhat.com> 4.1.25-1
- upgrade to 4.1.25, crypto version.
- enable posix mutexes using nptl on all arches.

* Mon Mar  3 2003 Thoams Woerner <twoerner@redhat.com> 4.0.14-21
- enabled db4-java for x86_64

* Wed Feb  5 2003 Nalin Dahyabhai <nalin@redhat.com> 4.0.14-20
- add dynamic libdb-4.0.so link back to %%{_libdir} so that dynamically
  linking with -ldb-4.0 will work again

* Tue Feb  4 2003 Nalin Dahyabhai <nalin@redhat.com> 4.0.14-19
- rebuild to use link the shared object with the same libraries we use
  for the bundled utils, should pull in libpthread when needed
- move libdb.so from /%%{_lib} to %%{_libdir} where the linker can find it

* Sun Feb 02 2003 Florian La Roche <Florian.LaRoche@redhat.de> 4.0.14-18
- add java for s390x

* Wed Jan 22 2003 Tim Powers <timp@redhat.com> 4.0.14-17
- rebuilt

* Wed Nov 20 2002 Florian La Roche <Florian.LaRoche@redhat.de> 4.0.14-16
- add java for s390

* Tue Oct  8 2002 Nalin Dahyabhai <nalin@redhat.com> 4.0.14-15
- add java bits back in for x86 boxes

* Fri Sep 20 2002 Than Ngo <than@redhat.com> 4.0.14-14.1
- Added better fix for s390/s390x/x86_64

* Thu Sep 05 2002 Arjan van de Ven
- remove java bits for x86-64

* Tue Aug 27 2002 Jeff Johnson <jbj@redhat.com> 4.0.14-14
- include libdb_tcl-4.1.a library.
- obsolete db1 packages.

* Tue Aug 13 2002 Nalin Dahyabhai <nalin@redhat.com> 4.0.14-13
- include patch to avoid db_recover (#70362)

* Sat Aug 10 2002 Elliot Lee <sopwith@redhat.com>
- rebuilt with gcc-3.2 (we hope)

* Tue Jul 23 2002 Nalin Dahyabhai <nalin@redhat.com> 4.0.14-11
- own %%{_includedir}/%{name}

* Tue Jul 23 2002 Tim Powers <timp@redhat.com> 4.0.14-10
- build using gcc-3.2-0.1

* Fri Jun 21 2002 Tim Powers <timp@redhat.com>
- automated rebuild

* Wed Jun 12 2002 Bernhard Rosenkraenzer <bero@redhat.com> 4.0.14-8
- Add java bindings
- Fix C++ bindings

* Thu May 23 2002 Tim Powers <timp@redhat.com>
- automated rebuild

* Thu May  9 2002 Jeff Johnson <jbj@redhat.com>
- re-enable db.h symlink creation, db_util names, and db[23]-devel obsoletes.
- make sure that -ldb is functional.

* Thu Feb 21 2002 Jeff Johnson <jbj@redhat.com>
- avoid db_util name collisions with multiple versions installed.

* Mon Feb 18 2002 Nalin Dahyabhai <nalin@redhat.com> 4.0.14-3
- remove relocatability stuffs
- swallow a local copy of db1 and build db185_dump statically with it, to
  remove the build dependency and simplify bootstrapping new arches

* Mon Jan 27 2002 Nalin Dahyabhai <nalin@redhat.com> 4.0.14-2
- have subpackages obsolete their db3 counterparts, because they conflict anyway

* Tue Jan  8 2002 Jeff Johnson <jbj@redhat.com> db4-4.0.14-1
- upgrade to 4.0.14.

* Sun Aug  5 2001 Nalin Dahyabhai <nalin@redhat.com>
- fix dangling docs symlinks
- fix dangling doc HREFs (#33328)
- apply the two patches listed at http://www.sleepycat.com/update/3.2.9/patch.3.2.9.html

* Tue Jun 19 2001 Bill Nottingham <notting@redhat.com>
- turn off --enable-debug

* Thu May 10 2001 Than Ngo <than@redhat.com>
- fixed to build on s390x

* Mon Mar 19 2001 Jeff Johnson <jbj@redhat.com>
- update to 3.2.9.

* Tue Dec 12 2000 Jeff Johnson <jbj@redhat.com>
- rebuild to remove 777 directories.

* Sat Nov 11 2000 Jeff Johnson <jbj@redhat.com>
- don't build with --enable-diagnostic.
- add build prereq on tcl.
- default value for %%_lib macro if not found.

* Tue Oct 17 2000 Jeff Johnson <jbj@redhat.com>
- add /usr/lib/libdb-3.1.so symlink to %%files.
- remove dangling tags symlink from examples.

* Mon Oct  9 2000 Jeff Johnson <jbj@redhat.com>
- rather than hack *.la (see below), create /usr/lib/libdb-3.1.so symlink.
- turn off --enable-diagnostic for performance.

* Fri Sep 29 2000 Jeff Johnson <jbj@redhat.com>
- update to 3.1.17.
- disable posix mutexes Yet Again.

* Tue Sep 26 2000 Jeff Johnson <jbj@redhat.com>
- add c++ and posix mutex support.

* Thu Sep 14 2000 Jakub Jelinek <jakub@redhat.com>
- put nss_db into a separate package

* Wed Aug 30 2000 Matt Wilson <msw@redhat.com>
- rebuild to cope with glibc locale binary incompatibility, again

* Wed Aug 23 2000 Jeff Johnson <jbj@redhat.com>
- remove redundant strip of libnss_db* that is nuking symbols.
- change location in /usr/lib/libdb-3.1.la to point to /lib (#16776).

* Thu Aug 17 2000 Jeff Johnson <jbj@redhat.com>
- summaries from specspo.
- all of libdb_tcl* (including symlinks) in db3-utils, should be db3->tcl?

* Wed Aug 16 2000 Jakub Jelinek <jakub@redhat.com>
- temporarily build nss_db in this package, should be moved
  into separate nss_db package soon

* Wed Jul 19 2000 Jakub Jelinek <jakub@redhat.com>
- rebuild to cope with glibc locale binary incompatibility

* Wed Jul 12 2000 Prospector <bugzilla@redhat.com>
- automatic rebuild

* Sun Jun 11 2000 Jeff Johnson <jbj@redhat.com>
- upgrade to 3.1.14.
- create db3-utils sub-package to hide tcl dependency, enable tcl Yet Again.
- FHS packaging.

* Mon Jun  5 2000 Jeff Johnson <jbj@redhat.com>
- disable tcl Yet Again, base packages cannot depend on libtcl.so.

* Sat Jun  3 2000 Jeff Johnson <jbj@redhat.com>
- enable tcl, rebuild against tcltk 8.3.1 (w/o pthreads).

* Tue May 30 2000 Matt Wilson <msw@redhat.com>
- include /lib/libdb.so in the devel package

* Wed May 10 2000 Jeff Johnson <jbj@redhat.com>
- put in "System Environment/Libraries" per msw instructions.

* Tue May  9 2000 Jeff Johnson <jbj@redhat.com>
- install shared library in /lib, not /usr/lib.
- move API docs to db3-devel.

* Mon May  8 2000 Jeff Johnson <jbj@redhat.com>
- don't rename db_* to db3_*.

* Tue May  2 2000 Jeff Johnson <jbj@redhat.com>
- disable --enable-test --enable-debug_rop --enable-debug_wop.
- disable --enable-posixmutexes --enable-tcl as well, to avoid glibc-2.1.3
  problems.

* Mon Apr 24 2000 Jeff Johnson <jbj@redhat.com>
- add 3.0.55.1 alignment patch.
- add --enable-posixmutexes (linux threads has not pthread_*attr_setpshared).
- add --enable-tcl (needed -lpthreads).

* Sat Apr  1 2000 Jeff Johnson <jbj@redhat.com>
- add --enable-debug_{r,w}op for now.
- add variable to set shm perms.

* Sat Mar 25 2000 Jeff Johnson <jbj@redhat.com>
- update to 3.0.55

* Tue Dec 29 1998 Jeff Johnson <jbj@redhat.com>
- Add --enable-cxx to configure.

* Thu Jun 18 1998 Jeff Johnson <jbj@redhat.com>
- Create.
