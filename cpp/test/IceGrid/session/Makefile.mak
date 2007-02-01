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
SERVER		= server.exe
VERIFIER        = verifier.exe

TARGETS		= $(CLIENT) $(SERVER) $(VERIFIER)

COBJS		= Client.obj \
		  AllTests.obj

SOBJS		= Server.obj

VOBJS		= PermissionsVerifier.obj

SRCS		= $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp) \
		  $(VOBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
LINKWITH	= $(LIBS) glacier2$(LIBSUFFIX).lib

!if "$(CPP_COMPILER)" != "BCC2006" & "$(OPTIMIZE)" != "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
VPDBFLAGS        = /pdb:$(VERIFIER:.exe=.pdb)
!endif

$(CLIENT): $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) icegrid$(LIBSUFFIX).lib
	-if exist $(CLIENT).manifest \
	    mt -nologo -manifest $(CLIENT).manifest -outputresource:$(CLIENT);#1 & del /q $(CLIENT).manifest

$(VERIFIER): $(VOBJS)
	$(LINK) $(LD_EXEFLAGS) $(VPDBFLAGS) $(VOBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH)
	-if exist $(VERIFIER).manifest \
	    mt -nologo -manifest $(VERIFIER).manifest -outputresource:$(VERIFIER);#1 & del /q $(VERIFIER).manifest

$(SERVER): $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) icessl$(LIBSUFFIX).lib
	-if exist $(SERVER).manifest \
	    mt -nologo -manifest $(SERVER).manifest -outputresource:$(SERVER);#1 & del /q $(SERVER).manifest

!include .depend
