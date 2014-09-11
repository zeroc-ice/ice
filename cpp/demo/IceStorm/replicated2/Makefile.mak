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

TARGETS		= $(PUBLISHER) $(SUBSCRIBER)

OBJS		= Clock.obj

POBJS		= Publisher.obj

SOBJS		= Subscriber.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(POBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp)


!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
PPDBFLAGS        = /pdb:$(PUBLISHER:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SUBSCRIBER:.exe=.pdb)
!endif

$(PUBLISHER): $(OBJS) $(POBJS)
	$(LINK) $(LD_EXEFLAGS) $(PPDBFLAGS) $(SETARGV) $(OBJS) $(POBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) \
	    icestorm$(LIBSUFFIX).lib
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SUBSCRIBER): $(OBJS) $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(OBJS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) \
	    icestorm$(LIBSUFFIX).lib
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	-del /q Clock.cpp Clock.h

clean::
	-if exist db1\__Freeze rmdir /q /s db1\__Freeze
	-for %f in (db1\*) do if not %f == db1\.gitignore del /q %f
	-if exist db2\__Freeze rmdir /q /s db2\__Freeze
	-for %f in (db2\*) do if not %f == db2\.gitignore del /q %f
	-if exist db3\__Freeze rmdir /q /s db3\__Freeze
	-for %f in (db3\*) do if not %f == db3\.gitignore del /q %f

!include .depend.mak
