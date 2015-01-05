# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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

SLICE_OBJS	= .\Hello.obj

COBJS		= $(SLICE_OBJS) \
		  .\Client.obj

SOBJS		= $(SLICE_OBJS) \
		  .\HelloI.obj \
		  .\HelloServiceI.obj

OBJS		= $(COBJS) \
		  $(SOBJS)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
LINKWITH	= $(LIBS) 

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
CPDBFLAGS       = /pdb:$(CLIENT:.exe=.pdb)
!endif

$(LIBNAME) : $(DLLNAME)

$(DLLNAME): $(SOBJS)
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(SETARGV) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(CLIENT):  $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Hello.cpp Hello.h
