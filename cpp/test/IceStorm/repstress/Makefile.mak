# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

PUBLISHER	= publisher.exe
SUBSCRIBER	= subscriber.exe
CONTROL		= control.exe

TARGETS		= $(PUBLISHER) $(SUBSCRIBER) $(CONTROL)

OBJS		= Single.obj \
		  Controller.obj

POBJS		= Publisher.obj

SOBJS		= Subscriber.obj

COBJS		= Control.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(POBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
LIBS		= icestorm$(LIBSUFFIX).lib $(LIBS)

!if "$(GENERATE_PDB)" == "yes"
PPDBFLAGS        = /pdb:$(PUBLISHER:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SUBSCRIBER:.exe=.pdb)
CPDBFLAGS        = /pdb:$(CONTROL:.exe=.pdb)
!endif

$(PUBLISHER): $(OBJS) $(POBJS)
	$(LINK) $(LD_EXEFLAGS) $(PPDBFLAGS) $(SETARGV) $(OBJS) $(POBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SUBSCRIBER): $(OBJS) $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(OBJS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(CONTROL): $(OBJS) $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(OBJS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
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
	del /q Controller.cpp Controller.h
	-if exist 0.db\__Freeze rmdir /q /s 0.db\__Freeze
	-for %f in (0.db\*) do if not %f == 0.db\.gitignore del /q %f
	-if exist 1.db\__Freeze rmdir /q /s 1.db\__Freeze
	-for %f in (1.db\*) do if not %f == 1.db\.gitignore del /q %f
	-if exist 2.db\__Freeze rmdir /q /s 2.db\__Freeze
	-for %f in (2.db\*) do if not %f == 2.db\.gitignore del /q %f

!include .depend.mak
