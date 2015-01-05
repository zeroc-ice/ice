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
SERVER		= server.exe

TARGETS		= $(CLIENT) $(SERVER)

SLICE_OBJS	= .\Hello.obj

COBJS		= $(SLICE_OBJS) \
		  .\Client.obj

SOBJS		= $(SLICE_OBJS) \
		  .\HelloI.obj \
		  .\Server.obj

OBJS		= $(COBJS) \
		  $(SOBJS)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

$(CLIENT): $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) \
	   
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Hello.cpp Hello.h

clean::
	-for %d in (master replica1 replica2) do \
	  if exist db\%d\__Freeze rmdir /q /s db\%d\__Freeze
	-for %d in (master replica1 replica2) do \
	  for %f in (db\%d\*) do if not %f == db\%d\.gitignore del /q %f
	-for %d in (node1 node2) do \
	  for %f in (db\%d\*) do if not %f == db\%d\.gitignore del /q %f
	-for %f in (distrib servers tmp) do if exist db\%d\%f rmdir /s /q db\%d\%f
