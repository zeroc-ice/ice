# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

LIBNAME		= InterceptorTest$(LIBSUFFIX).lib
DLLNAME		= InterceptorTest$(SOVERSION)$(LIBSUFFIX).dll

CLIENT		= client.exe

TARGETS		= $(LIBNAME) $(DLLNAME) $(CLIENT)

SLICE_OBJS	= .\Test.obj

LOBJS           = $(SLICE_OBJS) \
                  .\TestI.obj

COBJS		= .\Client.obj \
		  .\InterceptorI.obj \
		  .\AMDInterceptorI.obj \
		  .\MyObjectI.obj

OBJS		= $(COBJS) \
		  $(LOBJS)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
CPDBFLAGS       = /pdb:$(CLIENT:.exe=.pdb)
!endif

SLICE2CPPFLAGS	= --dll-export INTERCEPTOR_TEST_API $(SLICE2CPPFLAGS)

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(LOBJS)
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(LOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)


$(CLIENT): $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBNAME) $(LIBS) 
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Test.cpp Test.h
