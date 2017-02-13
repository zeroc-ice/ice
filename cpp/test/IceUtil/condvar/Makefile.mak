# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

WQ		= workqueue.exe
MATCH		= match.exe

TARGETS		= $(WQ) $(MATCH)

OBJS		= .\WorkQueue.obj \
		  .\Match.obj

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
WQ_PDBFLAGS        = /pdb:$(WQ:.exe=.pdb)
MATCH_PDBFLAGS     = /pdb:$(MATCH:.exe=.pdb)
!endif

$(WQ): WorkQueue.obj
	$(LINK) $(LD_EXEFLAGS) $(WQ_PDBFLAGS) $(SETARGV) WorkQueue.obj $(PREOUT)$@ $(PRELIBS)$(BASELIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(MATCH): Match.obj
	$(LINK) $(LD_EXEFLAGS) $(MATCH_PDBFLAGS) $(SETARGV) Match.obj $(PREOUT)$@ $(PRELIBS)$(BASELIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest
