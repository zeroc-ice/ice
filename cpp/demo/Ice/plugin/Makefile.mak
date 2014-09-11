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
SERVER		= server.exe

LOGGERLIBNAME   = loggerplugin$(SOVERSION)$(LIBSUFFIX).lib
LOGGERDLLNAME   = loggerplugin$(SOVERSION)$(LIBSUFFIX).dll

HELLOLIBNAME   	= helloplugin$(SOVERSION)$(LIBSUFFIX).lib
HELLODLLNAME   	= helloplugin$(SOVERSION)$(LIBSUFFIX).dll

TARGETS		= $(CLIENT) $(SERVER) $(LOGGERDLLNAME) $(HELLODLLNAME)

OBJS		= Hello.obj

COBJS		= Client.obj

SOBJS		= Server.obj

LOBJS		= LoggerPluginI.obj

HOBJS		= HelloPluginI.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp) \
		  $(LOBJS:.obj=.cpp) \
		  $(HOBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
LPDBFLAGS        = /pdb:$(LOGGERDLLNAME:.dll=.pdb)
HPDBFLAGS        = /pdb:$(HELLODLLNAME:.dll=.pdb)
!endif

$(CLIENT): $(OBJS) $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(OBJS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(OBJS) $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(OBJS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(LOGGERLIBNAME) : $(LOGGERDLLNAME)

$(LOGGERDLLNAME): $(LOBJS)
	$(LINK) $(LD_DLLFLAGS) $(LPDBFLAGS) $(SETARGV) $(LOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(LOGGERDLLNAME:.dll=.exp) del /q $(LOGGERDLLNAME:.dll=.exp)

$(HELLOLIBNAME) : $(HELLODLLNAME)

$(HELLODLLNAME): $(OBJS) $(HOBJS)
	$(LINK) $(LD_DLLFLAGS) $(HPDBFLAGS) $(SETARGV) $(OBJS) $(HOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(LOGGERDLLNAME:.dll=.exp) del /q $(LOGGERDLLNAME:.dll=.exp)

clean::
	del /q Hello.cpp Hello.h
	del /q $(LOGGERLIBNAME) $(HELLOLIBNAME)

!include .depend.mak
