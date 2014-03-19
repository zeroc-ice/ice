# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME     	= $(top_srcdir)\lib\icews$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icews$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

OBJS		= AcceptorI.obj \
                  ConnectorI.obj \
                  ConnectionInfo.obj \
                  EndpointInfo.obj \
                  EndpointI.obj \
                  Instance.obj \
                  PluginI.obj \
                  TransceiverI.obj \
                  Util.obj

SRCS		= $(OBJS:.obj=.cpp)

HDIR		= $(headerdir)\IceWS
SDIR		= $(slicedir)\IceWS

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I.. $(CPPFLAGS) -DICE_WS_API_EXPORTS -DWIN32_LEAN_AND_MEAN
SLICE2CPPFLAGS	= --ice --include-dir IceWS --dll-export ICE_WS_API $(SLICE2CPPFLAGS)

LINKWITH        = $(OPENSSL_LIBS) $(LIBS) ws2_32.lib

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

RES_FILE        = IceWS.res

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) IceWS.res
	$(LINK) $(BASE):0x24000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

clean::
	-del /q ConnectionInfo.cpp $(HDIR)\ConnectionInfo.h
	-del /q EndpointInfo.cpp $(HDIR)\EndpointInfo.h
	-del /q IceWS.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"


!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif

!include .depend.mak
