#
# Copyright (c) 2008-2014 ZeroC, Inc. All rights reserved.
#

Summary: mcpp, a portable C/C++ preprocessor
Name: mcpp-devel
Version: 2.7.2
Release: 2ice%{?dist}
Source0: http://prdownloads.sourceforge.net/mcpp/mcpp-2.7.2.tar.gz
Patch1: patch.mcpp.2.7.2
URL: http://mcpp.sourceforge.net/
License: BSD
Group: System Environment/Libraries
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%description
mcpp is a C/C++ preprocessor with the following features.

    * Implements all of C90, C99 and C++98 specifications.
    * Provides a validation suite to test C/C++ preprocessor's conformance 
      and quality comprehensively. When this validation suite is applied,
       mcpp distinguishes itself among many existing preprocessors.
    * Has plentiful and accurate diagnostics to check all the preprocessing 
      problems such as latent bug or lack of portability in source code.
    * Has #pragma directives to output debugging information.
    * Is portable and has been ported to many compliler-systems, including 
      GCC and Visual C++, on UNIX-like systems and Windows.
    * Preprocessors of various behavior modes are able to be generated from 
      the source code.
    * Can be built either as a compiler-specific preprocessor to replace the
      resident preprocessor of a particular compiler system, or as a 
      compiler-independent command, or even as a subroutine called from some 
      other main program.
    * Provides comprehensive documents both in Japanese and in English.
    * Is an open source software released under BSD-style-license. 

It is probably number one C/C++ preprocessor now available in the world. 

%prep

%setup -q -n mcpp-%{version}
%patch1 -p0 -b .mcpp.2.7.2

%build

./configure CFLAGS=-fPIC --enable-mcpplib --disable-shared
make

%install
rm -rf ${RPM_BUILD_ROOT}
mkdir -p ${RPM_BUILD_ROOT}%{_libdir}

cp src/.libs/libmcpp.a ${RPM_BUILD_ROOT}%{_libdir}

%clean
rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root)
%doc LICENSE README
%{_libdir}/libmcpp.a

%changelog

* Wed Jan 14 2009 Dwayne Boone <dwayne@zeroc.com>
- mcpp 2.7.2 update

* Wed Apr 30 2008 Bernard Normier <bernard@zeroc.com>
- mcpp 2.7 update

* Thu Feb 7 2008  Bernard Normier <bernard@zeroc.com>
- Initial version
