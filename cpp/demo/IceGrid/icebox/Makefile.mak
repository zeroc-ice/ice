# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

CLIENT		= client.exe

LIBNAME     	= helloservice$(LIBSUFFIX).lib
DLLNAME         = helloservice$(LIBSUFFIX).dll

TARGETS		= $(CLIENT) $(LIBNAME) $(DLLNAME)

OBJS		= Hello.obj

COBJS		= Client.obj

SOBJS		= HelloI.obj \
		  HelloServiceI.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp)

SLICE_SRCS	= Hello.ice

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
LINKWITH	= $(LIBS) icebox$(LIBSUFFIX).lib

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
CPDBFLAGS       = /pdb:$(CLIENT:.exe=.pdb)
!endif

$(LIBNAME) : $(DLLNAME)

$(DLLNAME): $(OBJS) $(SOBJS)
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(SETARGV) $(OBJS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(CLIENT): $(OBJS) $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(OBJS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Hello.cpp Hello.h

!include .depend
