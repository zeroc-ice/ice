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
SERVER		= server.exe
SERVERAMD	= serveramd.exe
COLLOCATED	= collocated.exe

TARGETS		= $(CLIENT) $(SERVER) $(SERVERAMD) $(COLLOCATED)

SLICE_OBJS	= .\Test.obj \
		  .\TestAMD.obj \
		  .\Wstring.obj \
		  .\WstringAMD.obj

COBJS           = .\Test.obj \
		  .\Wstring.obj \
		  .\Client.obj \
		  .\AllTests.obj \
		  .\MyByteSeq.obj \
		  .\StringConverterI.obj

SOBJS           = .\Test.obj \
		  .\Wstring.obj \
		  .\TestI.obj \
		  .\WstringI.obj \
		  .\Server.obj \
		  .\MyByteSeq.obj \
		  .\StringConverterI.obj

SAMDOBJS        = .\TestAMD.obj \
		  .\TestAMDI.obj \
		  .\WstringAMD.obj \
		  .\WstringAMDI.obj \
		  .\ServerAMD.obj \
		  .\MyByteSeq.obj \
		  .\StringConverterI.obj

COLOBJS         = .\Test.obj \
		  .\Wstring.obj \
		  .\TestI.obj \
		  .\WstringI.obj \
		  .\Collocated.obj \
		  .\AllTests.obj \
		  .\MyByteSeq.obj \
		  .\StringConverterI.obj

OBJS 		= $(COBJS) \
		  $(SOBJS) \
		  $(SAMDOBJS) \
		  $(COLOBJS)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DSTRING_VIEW_IGNORE_STRING_CONVERTER -DWIN32_LEAN_AND_MEAN -Zm300 -bigobj

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
SAPDBFLAGS       = /pdb:$(SERVERAMD:.exe=.pdb)
COPDBFLAGS       = /pdb:$(COLLOCATED:.exe=.pdb)
!endif

$(CLIENT): $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVERAMD): $(SAMDOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SAPDBFLAGS) $(SETARGV) $(SAMDOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(COLLOCATED): $(COLOBJS)
	$(LINK) $(LD_EXEFLAGS) $(COPDBFLAGS) $(SETARGV) $(COLOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Test.cpp Test.h
	del /q TestAMD.cpp TestAMD.h
	del /q Wstring.cpp Wstring.h
	del /q WstringAMD.cpp WstringAMD.h
