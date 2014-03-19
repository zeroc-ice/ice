# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

CLIENT		= client.exe

TARGETS		= $(CLIENT)

OBJS		= Client.obj IntLongMap.obj

SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

$(CLIENT): $(OBJS)
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(SETARGV) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) freeze$(LIBSUFFIX).lib
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

IntLongMap.h IntLongMap.cpp: "$(SLICE2FREEZE)" "$(SLICEPARSERLIB)"
	del /q IntLongMap.h IntLongMap.cpp
	"$(SLICE2FREEZE)" -I"$(slicedir)" --dict IntLongMap,int,long IntLongMap

clean::
	-del /q IntLongMap.h IntLongMap.cpp
	-del /q oldlogs.txt
	if exist hotbackup rmdir /q /s hotbackup
	if exist hotbackup.1 rmdir /q /s hotbackup.1
	-if exist db\__Freeze rmdir /q /s db\__Freeze
	-del /q  db\__*.*
	-for %f in (db\data\*) do if not %f == db\data\.gitignore del /q %f
	-for %f in (db\logs\*) do if not %f == db\logs\.gitignore del /q %f

!include .depend.mak
