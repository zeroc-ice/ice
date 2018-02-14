# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

CLIENT		= client.exe

TARGETS		= $(CLIENT)

SLICE_OBJS	= .\Test.obj \
		  .\Types.obj

OBJS		= $(SLICE_OBJS) \
		  .\Client.obj \
		  .\AllTests.obj

!include $(top_srcdir)/config/Make.rules.mak

SLICE2CPPFLAGS	= --checksum $(SLICE2CPPFLAGS)
CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

$(CLIENT): $(OBJS)
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(SETARGV) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Test.cpp Test.h
	del /q Types.cpp Types.h

SUBDIRS		= server

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
