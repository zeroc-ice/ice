# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

LIBNAME		= InterceptorTest$(LIBSUFFIX).lib
DLLNAME		= InterceptorTest$(SOVERSION)$(LIBSUFFIX).dll

CLIENT		= client.exe

!ifdef BUILD_CLIENT
TARGETS		= $(CLIENT)
!else
TARGETS		= $(LIBNAME) $(DLLNAME)
!endif

LOBJS           = Test.obj \
                  TestI.obj

COBJS		= Client.obj \
		  InterceptorI.obj \
		  AMDInterceptorI.obj \
		  MyObjectI.obj

!ifdef BUILD_CLIENT
SRCS		= $(COBJS:.obj=.cpp)
!else
SRCS		= $(LOBJS:.obj=.cpp)
!endif

!include $(top_srcdir)/config/Make.rules.mak

!ifdef BUILD_CLIENT
CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

!else
SLICE2CPPFLAGS	= --dll-export INTERCEPTOR_TEST_API $(SLICE2CPPFLAGS)
CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN -DINTERCEPTOR_TEST_API_EXPORTS

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif
!endif


$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(LOBJS)
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(LOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)


$(CLIENT): $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBNAME) $(LIBS) 
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

!ifdef BUILD_CLIENT
clean::
	del /q Test.cpp Test.h
!else

$(EVERYTHING)::
	@$(MAKE) -nologo /f Makefile.mak BUILD_CLIENT=1 $@

!endif

!include .depend
