# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

LIBNAME		= Generated$(LIBSUFFIX).lib
DLLNAME		= Generated$(SOVERSION)$(LIBSUFFIX).dll
CLIENT		= client.exe

TARGETS		= $(LIBNAME) $(DLLNAME) $(CLIENT) 

LOBJS 		= Test.obj

COBJS		= Client.obj \
		  AllTests.obj


SRCS		= $(LOBJS:.obj=.cpp) \
		   $(COBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

SLICE2CPPFLAGS	= --dll-export TEST_API $(SLICE2CPPFLAGS)

LINKWITH        = $(LIBS)

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(LOBJS)
	$(LINK) $(BASE):0x22000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(LOBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(CLIENT): $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBNAME) $(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Test.cpp Test.h

!include .depend.mak
