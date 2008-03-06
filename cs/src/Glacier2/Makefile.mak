# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

PKG		= Glacier2
LIBNAME		= $(PKG).dll
TARGETS		= $(bindir)\$(LIBNAME)

SRCS		= AssemblyInfo.cs

GEN_SRCS	= $(GDIR)\PermissionsVerifier.cs \
		  $(GDIR)\Router.cs \
		  $(GDIR)\Session.cs \
		  $(GDIR)\SSLInfo.cs

SDIR		= $(slicedir)\Glacier2
GDIR		= generated

!include $(top_srcdir)/config/Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:library -out:$(TARGETS) -unsafe -warnaserror-
MCSFLAGS	= $(MCSFLAGS) -keyfile:$(top_srcdir)\config\IcecsKey.snk

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) -I$(slicedir)

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -r:$(bindir)\Ice.dll $(SRCS) $(GEN_SRCS)

!if "$(NOGAC)" == ""

install:: all
	$(GACUTIL) -i $(bindir)\$(LIBNAME)

!else

install:: all
	copy $(bindir)\$(LIBNAME) $(install_bindir)

!endif

!include .depend
