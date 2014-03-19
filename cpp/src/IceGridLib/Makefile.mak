# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\icegrid$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icegrid$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

TARGETS         = $(LIBNAME) $(DLLNAME)

LIB_OBJS	= Admin.obj \
		  Locator.obj \
		  Query.obj \
		  Exception.obj \
		  Descriptor.obj \
                  FileParser.obj \
		  Observer.obj \
		  Session.obj \
		  Registry.obj \
		  UserAccountMapper.obj

SRCS		= $(LIB_OBJS:.obj=.cpp)

HDIR		= $(headerdir)\IceGrid
SDIR		= $(slicedir)\IceGrid

!include $(top_srcdir)\config\Make.rules.mak

LINKWITH 	= $(LIBS) glacier2$(LIBSUFFIX).lib

SLICE2CPPFLAGS	= --checksum --ice --include-dir IceGrid --dll-export ICE_GRID_API $(SLICE2CPPFLAGS)
CPPFLAGS        = -I.. $(CPPFLAGS)

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

RES_FILE        = IceGrid.res

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(LIB_OBJS) IceGrid.res
	$(LINK) $(BASE):0x2B000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(LIB_OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

clean::
	-del /q FileParser.cpp $(HDIR)\FileParser.h
	-del /q Admin.cpp $(HDIR)\Admin.h
	-del /q Descriptor.cpp $(HDIR)\Descriptor.h
	-del /q Exception.cpp $(HDIR)\Exception.h
	-del /q Locator.cpp $(HDIR)\Locator.h
	-del /q Observer.cpp $(HDIR)\Observer.h
	-del /q Query.cpp $(HDIR)\Query.h
	-del /q Session.cpp $(HDIR)\Session.h
	-del /q Registry.cpp $(HDIR)\Registry.h
	-del /q UserAccountMapper.cpp $(HDIR)\UserAccountMapper.h
	-del /q IceGrid.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"


!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif

!include .depend.mak
