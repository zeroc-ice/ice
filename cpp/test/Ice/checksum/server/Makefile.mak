# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..\..

SERVER		= server.exe

TARGETS		= $(SERVER)

SLICE_OBJS	= .\Test.obj \
		  .\Types.obj

OBJS		= $(SLICE_OBJS) \
		  .\TestI.obj \
		  .\Server.obj

SRCS		= $(SOBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

SLICE2CPPFLAGS	= --checksum $(SLICE2CPPFLAGS)
CPPFLAGS	= -I. -I../../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

$(SERVER): $(OBJS)
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(SETARGV) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Test.cpp Test.h
	del /q Types.cpp Types.h
