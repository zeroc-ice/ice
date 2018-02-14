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
LIBNAME		= testtransport$(LIBSUFFIX).lib
DLLNAME		= testtransport$(SOVERSION)$(LIBSUFFIX).dll

TARGETS		= $(CLIENT) $(SERVER) $(LIBNAME) $(DLLNAME)

SLICE_OBJS 	= .\Test.obj

COBJS		= $(SLICE_OBJS) \
		  .\Configuration.obj \
		  .\Client.obj \
		  .\AllTests.obj

SOBJS		= $(SLICE_OBJS) \
		  .\Configuration.obj \
		  .\TestI.obj \
		  .\Server.obj

TRANSPORT_OBJS	= $(SLICE_OBJS) \
		  .\Configuration.obj \
		  .\Connector.obj \
		  .\Acceptor.obj \
		  .\EndpointI.obj \
		  .\Transceiver.obj \
		  .\EndpointFactory.obj \
		  .\PluginI.obj

OBJS		= $(TRANSPORT_OBJS) \
		  $(COBJS) \
		  $(SOBJS)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include -I../../../src $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
LINKWITH	= $(LIBS)

!if "$(GENERATE_PDB)" == "yes"
TPDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

$(CLIENT): $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(LIBNAME) : $(DLLNAME)

$(DLLNAME): $(TRANSPORT_OBJS)
	$(LINK) $(LD_DLLFLAGS) $(TPDBFLAGS) $(SETARGV) $(TRANSPORT_OBJS) $(PREOUT)$(DLLNAME) $(PRELIBS)$(LINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

clean::
	del /q Test.cpp Test.h
