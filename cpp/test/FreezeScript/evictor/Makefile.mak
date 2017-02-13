# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

CLIENT		= makedb.exe

TARGETS		= $(CLIENT)

SLICE_OBJS	= .\TestOld.obj

OBJS		= $(SLICE_OBJS) \
		  .\makedb.obj

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

$(CLIENT): $(OBJS)
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(SETARGV) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) 
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q TestOld.cpp TestOld.h

clean::
	if exist db\__Freeze rmdir /s /q db\__Freeze
	del /q db\*.db db\log.* db\__catalog db\__catalogIndexList
	if exist db_check rmdir /s /q db_check
	if exist db_tmp rmdir /s /q db_tmp
