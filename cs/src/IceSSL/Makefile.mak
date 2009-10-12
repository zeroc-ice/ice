# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

PKG		= IceSSL
LIBNAME		= $(PKG).dll
TARGETS		= $(bindir)\$(LIBNAME)
POLICY_TARGET   = $(POLICY).dll

SRCS		= AcceptorI.cs \
		  AssemblyInfo.cs \
		  ConnectorI.cs \
		  EndpointI.cs \
		  Instance.cs \
		  Plugin.cs \
		  PluginI.cs \
		  RFC2253.cs \
		  TransceiverI.cs \
		  TrustManager.cs \
		  Util.cs

GEN_SRCS	= $(GDIR)\EndpointInfo.cs \
		  $(GDIR)\ConnectionInfo.cs

SDIR		= $(slicedir)\IceSSL
GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:library -out:$(TARGETS) -warnaserror-
MCSFLAGS	= $(MCSFLAGS) -keyfile:$(KEYFILE)
MCSFLAGS	= $(MCSFLAGS) /doc:$(bindir)\$(PKG).xml /nowarn:1591

SLICE2CSFLAGS   = $(SLICE2CSFLAGS) --ice -I$(slicedir)

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -r:$(refdir)\Ice.dll $(SRCS) $(GEN_SRCS)

!if "$(DEBUG)" == "yes"
clean::
	del /q $(bindir)\$(PKG).pdb
!endif

install:: all
	copy $(bindir)\$(LIBNAME) $(install_bindir)
	copy $(bindir)\$(PKG).xml $(install_bindir)
	copy $(bindir)\$(POLICY) $(install_bindir)
	copy $(bindir)\$(POLICY_TARGET) $(install_bindir)
!if "$(DEBUG)" == "yes"
	copy $(bindir)\$(PKG).pdb $(install_bindir)
!endif

!include .depend
