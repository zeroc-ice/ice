# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

PKG		= icecsssl
LIBNAME		= $(PKG).dll
TARGETS		= $(bindir)\$(LIBNAME)

SRCS		= AcceptorI.cs \
		  ConnectorI.cs \
		  EndpointI.cs \
		  Instance.cs \
		  Plugin.cs \
		  PluginI.cs \
		  RFC2253.cs \
		  TransceiverI.cs \
		  TrustManager.cs \
		  Util.cs

SDIR		= $(slicedir)\Ice
GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak

MCSFLAGS	= $(MCSFLAGS) -target:library -out:$(TARGETS) -unsafe -warnaserror-
!if "$(DOTNET_1)" != "yes"
MCSFLAGS	= $(MCSFLAGS) -keyfile:$(top_srcdir)\config\IcecsKey.snk
!endif

$(TARGETS):: $(SRCS)
	$(MCS) $(MCSFLAGS) -r:$(bindir)\icecs.dll $(SRCS)

!if "$(NOGAC)" == ""

install:: all
	$(GACUTIL) -i $(bindir)/$(LIBNAME)

!else

install:: all
	copy $(bindir)\icesslcs.dll $(install_bindir)

!endif

!include .depend
