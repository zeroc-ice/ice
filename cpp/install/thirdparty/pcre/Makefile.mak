# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Win32 makefile for PCRE (Perl-Compatible Regular Expression) library. Should
# work with Visual C++ 6.0, Visual Studio .NET 2003 and Visual Studio 2005.
# This makefile is roughly adapted from the makefile generated using the pcre
# configure script under cygwin on Windows.
#
# Notable differences:
#
#  * Only the posix version of the library is created.
#  * C++ code has been removed from the build.
#  * Works with Microsoft compilers.
#  * The makefile generates both the debug and release versions of the library.
#

top_srcdir = .

prefix=c:\pcre-6.6-for-ice

PCREGREP_OSTYPE=-DHAVE_WIN32API

EXEEXT = .exe
OBJEXT = .obj

CC = cl
CFLAGS = /nologo /D_CRT_SECURE_NO_DEPRECATE /DPCRE_STATIC /I. 

POSIX_MALLOC_THRESHOLD = -DPOSIX_MALLOC_THRESHOLD=10

DebugDir = .\Debug
ReleaseDir = .\Release

##############################################################################

OBJ =   DIR\pcre_chartables$(OBJEXT) \
	DIR\pcre_compile$(OBJEXT) \
	DIR\pcre_config$(OBJEXT) \
	DIR\pcre_dfa_exec$(OBJEXT) \
	DIR\pcre_exec$(OBJEXT) \
	DIR\pcre_fullinfo$(OBJEXT) \
	DIR\pcre_get$(OBJEXT) \
	DIR\pcre_globals$(OBJEXT) \
	DIR\pcre_info$(OBJEXT) \
	DIR\pcre_maketables$(OBJEXT) \
	DIR\pcre_ord2utf8$(OBJEXT) \
	DIR\pcre_refcount$(OBJEXT) \
	DIR\pcre_study$(OBJEXT) \
	DIR\pcre_tables$(OBJEXT) \
	DIR\pcre_try_flipped$(OBJEXT) \
	DIR\pcre_ucp_searchfuncs$(OBJEXT) \
	DIR\pcre_valid_utf8$(OBJEXT) \
	DIR\pcre_version$(OBJEXT) \
	DIR\pcre_xclass$(OBJEXT) \
	DIR\pcreposix$(OBJEXT)

TARGETS = pcred.lib pcre.lib pcretest.exe pcregrep.exe 

all:: builddirs $(TARGETS)

builddirs:: 
	if not exist .\Debug mkdir .\Debug
	if not exist .\Release mkdir .\Release
	if not exist unistd.h echo #define dummy_unistd_h > unistd.h

ReleaseObjs = $(OBJ:DIR=.\Release)

DebugObjs = $(OBJ:DIR=.\Debug)

pcred.lib: $(DebugObjs)
	-del $@ 
	lib /out:$@  $(DebugObjs)

pcre.lib: $(ReleaseObjs)
	-del $@ 
	lib /out:$@ $(ReleaseObjs)
 
clean::
	-del .\Debug\*.obj .\Release\*.obj
	-rd .\Debug
	-rd .\Release
	-del *.pdb *.ilk *.exe.manifest
	-del pcretest.obj pcretest.obj dftables.obj dftables.exe
	-del $(TARGETS)

install:: pcre.lib pcred.lib 
	if not exist $(prefix) mkdir $(prefix)
	if not exist $(prefix)\include mkdir $(prefix)\include
	if not exist $(prefix)\lib mkdir $(prefix)\lib
	copy pcre.lib $(prefix)\lib
	copy pcred.lib $(prefix)\lib
	copy pcreposix.h $(prefix)\include

pcretest$(EXEEXT):  $(top_srcdir)\pcretest.c $(top_srcdir)\pcre_internal.h \
	$(top_srcdir)\pcre_printint.src $(top_srcdir)\pcre.h config.h \
	pcred.lib
 	$(CC) $(CFLAGS) /Zi /MDd -I. -I$(top_srcdir) $(top_srcdir)/pcretest.c pcred.lib
 
pcregrep$(EXEEXT):  $(top_srcdir)\pcregrep.c $(top_srcdir)\pcre.h config.h \
	pcred.lib
 	$(CC) $(CFLAGS) /Zi /MDd /I. -I$(top_srcdir) $(PCREGREP_OSTYPE) $(top_srcdir)/pcregrep.c pcred.lib

pcre_chartables.c: dftables$(EXEEXT)
		dftables$(EXEEXT) pcre_chartables.c

dftables$(EXEEXT): $(top_srcdir)\dftables.c $(top_srcdir)\pcre_maketables.c \
		$(top_srcdir)\pcre_internal.h $(top_srcdir)\pcre.h config.h 
		
	$(CC) $(CFLAGS) -I. $(top_srcdir)/dftables.c


#
# Generated rules start here!
#

.\Debug\pcre_chartables.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_chartables.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_chartables.c

.\Release\pcre_chartables.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_chartables.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_chartables.c

.\Debug\pcre_compile.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_compile.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_compile.c

.\Release\pcre_compile.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_compile.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_compile.c

.\Debug\pcre_config.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_config.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_config.c

.\Release\pcre_config.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_config.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_config.c

.\Debug\pcre_dfa_exec.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_dfa_exec.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_dfa_exec.c

.\Release\pcre_dfa_exec.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_dfa_exec.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_dfa_exec.c

.\Debug\pcre_exec.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_exec.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_exec.c

.\Release\pcre_exec.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_exec.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_exec.c

.\Debug\pcre_fullinfo.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_fullinfo.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_fullinfo.c

.\Release\pcre_fullinfo.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_fullinfo.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_fullinfo.c

.\Debug\pcre_get.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_get.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_get.c

.\Release\pcre_get.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_get.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_get.c

.\Debug\pcre_globals.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_globals.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_globals.c

.\Release\pcre_globals.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_globals.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_globals.c

.\Debug\pcre_info.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_info.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_info.c

.\Release\pcre_info.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_info.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_info.c

.\Debug\pcre_maketables.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_maketables.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_maketables.c

.\Release\pcre_maketables.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_maketables.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_maketables.c

.\Debug\pcre_ord2utf8.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_ord2utf8.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_ord2utf8.c

.\Release\pcre_ord2utf8.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_ord2utf8.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_ord2utf8.c

.\Debug\pcre_refcount.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_refcount.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_refcount.c

.\Release\pcre_refcount.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_refcount.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_refcount.c

.\Debug\pcre_study.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_study.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_study.c

.\Release\pcre_study.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_study.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_study.c

.\Debug\pcre_tables.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_tables.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_tables.c

.\Release\pcre_tables.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_tables.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_tables.c

.\Debug\pcre_try_flipped.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_try_flipped.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_try_flipped.c

.\Release\pcre_try_flipped.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_try_flipped.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_try_flipped.c

.\Debug\pcre_ucp_searchfuncs.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_ucp_searchfuncs.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_ucp_searchfuncs.c

.\Release\pcre_ucp_searchfuncs.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_ucp_searchfuncs.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_ucp_searchfuncs.c

.\Debug\pcre_valid_utf8.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_valid_utf8.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_valid_utf8.c

.\Release\pcre_valid_utf8.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_valid_utf8.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_valid_utf8.c

.\Debug\pcre_version.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_version.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_version.c

.\Release\pcre_version.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_version.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_version.c

.\Debug\pcre_xclass.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcre_xclass.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcre_xclass.c

.\Release\pcre_xclass.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcre_xclass.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcre_xclass.c

.\Debug\pcreposix.obj: config.h pcre_internal.h \
	        pcre.h pcre_config.c  pcreposix.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MDd /Zi /Od pcreposix.c

.\Release\pcreposix.obj: config.h pcre_internal.h \
		pcre.h pcre_config.c  pcreposix.c
	$(CC) /c $(POSIX_MALLOC_THRESHOLD) $(CFLAGS) /Fo$@ /MD /Ox pcreposix.c

