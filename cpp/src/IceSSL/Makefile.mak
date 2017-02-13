# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\icessl$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icessl$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

SLICE_OBJS	= .\ConnectionInfo.obj \
                  .\EndpointInfo.obj

OBJS		= $(SLICE_OBJS) \
		  .\AcceptorI.obj \
		  .\Certificate.obj \
		  .\ConnectorI.obj \
		  .\EndpointI.obj \
		  .\Instance.obj \
		  .\PluginI.obj \
		  .\RFC2253.obj \
		  .\SChannelEngine.obj \
		  .\SChannelTransceiverI.obj \
		  .\SSLEngine.obj \
		  .\TrustManager.obj \
		  .\Util.obj

HDIR		= $(headerdir)\IceSSL
SDIR		= $(slicedir)\IceSSL

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I.. $(CPPFLAGS) -DICE_SSL_API_EXPORTS -DWIN32_LEAN_AND_MEAN
SLICE2CPPFLAGS	= --ice --include-dir IceSSL --dll-export ICE_SSL_API $(SLICE2CPPFLAGS)

LINKWITH	= $(LIBS) $(SSL_OS_LIBS) 

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS	= /pdb:$(DLLNAME:.dll=.pdb)
!endif

RES_FILE	= IceSSL.res

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) IceSSL.res
	$(LINK) $(BASE):0x24000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

clean::
	-del /q ConnectionInfo.cpp $(HDIR)\ConnectionInfo.h
	-del /q EndpointInfo.cpp $(HDIR)\EndpointInfo.h
	-del /q IceSSL.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"


info:
	"$(SLICE2CPP)" $(SLICE2CPPFLAGS) $(SDIR)/ConnectionInfo.ice
	move ConnectionInfo.h $(HDIR)

!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif
