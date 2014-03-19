# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\icestorm$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icestorm$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

TARGETS         = $(LIBNAME) $(DLLNAME)

OBJS		= IceStorm.obj \
		  Metrics.obj

SRCS		= $(OBJS:.obj=.cpp)

HDIR		= $(headerdir)\IceStorm
SDIR		= $(slicedir)\IceStorm

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I.. -Idummyinclude $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
ICECPPFLAGS	= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS	= --ice --include-dir IceStorm --checksum --dll-export ICE_STORM_LIB_API $(SLICE2CPPFLAGS)
LINKWITH 	= $(LIBS) $(DBLINKWITH) icestorm$(LIBSUFFIX).lib icegrid$(LIBSUFFIX).lib icebox$(LIBSUFFIX).lib

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

RES_FILE        = IceStorm.res

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) IceStorm.res
	$(LINK) $(BASE):0x2A000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

clean::
	-del /q IceStorm.cpp $(HDIR)\IceStorm.h
	-del /q Metrics.cpp $(HDIR)\Metrics.h
	-del /q IceStorm.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"


!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif

!include .depend.mak
