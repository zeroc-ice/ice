# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\testcommon$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\testcommon$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

SLICE_OBJS      = .\Controller.obj

OBJS		= $(SLICE_OBJS) \
		  .\TestCommon.obj

HDIR		= ..\include
SDIR		= .

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. -I$(HDIR) $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN -DTEST_API_EXPORTS

SLICE2CPPFLAGS	= --dll-export TEST_API $(SLICE2CPPFLAGS)

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	$(LINK) $(BASE):0x30000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

clean::
	-del /q Controller.cpp $(HDIR)\Controller.h

install:: all
