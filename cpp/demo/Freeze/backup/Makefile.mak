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

TARGETS		= $(CLIENT)

OBJS		= Client.obj IntLongMap.obj

SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

$(CLIENT): $(OBJS)
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(SETARGV) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) freeze$(LIBSUFFIX).lib
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

IntLongMap.h IntLongMap.cpp: $(SLICE2FREEZE)
	del /q IntLongMap.h IntLongMap.cpp
	$(SLICE2FREEZE) -I$(slicedir) --dict IntLongMap,int,long IntLongMap

cleandb:
        if exist hotbackup rmdir /q /s hotbackup
        if exist hotbackup.1 rmdir /q /s hotbackup.1
        del /q  db\data\IntLongMap db\data\__catalog db\logs\log.*

clean:: cleandb
	del /q IntLongMap.h IntLongMap.cpp

!include .depend
