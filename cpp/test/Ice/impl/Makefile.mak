# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

!if "$(WINRT)" != "yes"
NAME_PREFIX	=
EXT		= .exe
OBJDIR		= .
!else
NAME_PREFIX	= Ice_operations_
EXT		= .dll
OBJDIR		= winrt
!endif

SERVER		= $(NAME_PREFIX)server
SERVERAMD	= $(NAME_PREFIX)serveramd

TARGETS		= $(SERVER)$(EXT) $(SERVERAMD)$(EXT)

SLICE_OBJS	= $(OBJDIR)\Test.obj $(OBJDIR)\TestAMD.obj

SOBJS		= $(OBJDIR)\Test.obj \
		  $(OBJDIR)\TestI.obj \
		  $(OBJDIR)\Server.obj

SAMDOBJS	= $(OBJDIR)\TestAMD.obj \
		  $(OBJDIR)\TestAMDI.obj \
		  $(OBJDIR)\ServerAMD.obj

OBJS		= $(SOBJS) \
		  $(SAMDOBJS)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN -Zm200 -bigobj /wd4503
LINKWITH	= testcommon$(LIBSUFFIX).lib $(LIBS)

!if "$(GENERATE_PDB)" == "yes"
SPDBFLAGS        = /pdb:$(SERVER).pdb
SAPDBFLAGS       = /pdb:$(SERVERAMD).pdb
!endif

!if "$(CPP11_MAPPING)" == "yes"
SLICE2CPPFLAGS = --impl-c++11 $(SLICE2CPPFLAGS)
!else
SLICE2CPPFLAGS = --impl-c++98 $(SLICE2CPPFLAGS)
!endif

Test.cpp Test.h:
	del /q (*F).h $(*F).cpp (*F)I.h $(*F)I.cpp
	"$(SLICE2CPP)" $(SLICE2CPPFLAGS) $<

TestAMD.cpp TestAMD.h:
	del /q (*F).h $(*F).cpp (*F)I.h $(*F)I.cpp
	"$(SLICE2CPP)" $(SLICE2CPPFLAGS) $<

$(SERVER)$(EXT): $(SOBJS)
	$(LINK) $(LD_TESTFLAGS) $(SPDBFLAGS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVERAMD)$(EXT): $(SAMDOBJS)
	$(LINK) $(LD_TESTFLAGS) $(SAPDBFLAGS) $(SAMDOBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Test.cpp Test.h
	del /q TestI.cpp TestI.h
	del /q TestAMD.cpp TestAMD.h
	del /q TestAMDI.cpp TestAMDI.h
