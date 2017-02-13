# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

PUBLISHER	= publisher.exe
SUBSCRIBER	= subscriber.exe
SUB		= sub.exe

TARGETS		= $(PUBLISHER) $(SUBSCRIBER) $(SUB)

SLICE_OBJS	= .\Single.obj

POBJS		= $(SLICE_OBJS) \
		  .\Publisher.obj

SOBJS		= $(SLICE_OBJS) \
		  .\Subscriber.obj

SUB_OBJS	= $(SLICE_OBJS) \
		  .\Sub.obj

OBJS		= $(POBJS) \
		  $(SOBJS) \
		  $(SUB_OBJS)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
LIBS		=  $(LIBS)

!if "$(GENERATE_PDB)" == "yes"
PPDBFLAGS        = /pdb:$(PUBLISHER:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SUBSCRIBER:.exe=.pdb)
SUB_PDBFLAGS     = /pdb:$(SUB:.exe=.pdb)
!endif

$(PUBLISHER): $(POBJS)
	$(LINK) $(LD_EXEFLAGS) $(PPDBFLAGS) $(SETARGV) $(POBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SUBSCRIBER): $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SUB): $(SUB_OBJS)
	$(LINK) $(LD_EXEFLAGS) $(SUB_PDBFLAGS) $(SETARGV) $(SUB_OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

!if "$(OPTIMIZE)" == "yes"

all::
	@echo release > build.txt

!else

all::
	@echo debug > build.txt

!endif

clean::
	del /q build.txt
	del /q Single.cpp Single.h
	-if exist 0.db\__Freeze rmdir /q /s 0.db\__Freeze
	-for %f in (0.db\*) do if not %f == 0.db\.gitignore del /q %f
	-if exist 1.db\__Freeze rmdir /q /s 1.db\__Freeze
	-for %f in (1.db\*) do if not %f == 1.db\.gitignore del /q %f
	-if exist 2.db\__Freeze rmdir /q /s 2.db\__Freeze
	-for %f in (2.db\*) do if not %f == 2.db\.gitignore del /q %f
