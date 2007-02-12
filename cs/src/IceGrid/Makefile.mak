# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

PKG		= icegridcs
LIBNAME		= $(PKG).dll
TARGETS		= $(bindir)\$(LIBNAME)

SRCS		= AssemblyInfo.cs

GEN_SRCS	= $(GDIR)\Admin.cs \
		  $(GDIR)\Observer.cs \
		  $(GDIR)\Descriptor.cs \
		  $(GDIR)\Exception.cs \
		  $(GDIR)\FileParser.cs \
		  $(GDIR)\Query.cs \
		  $(GDIR)\Registry.cs \
		  $(GDIR)\Session.cs \
		  $(GDIR)\UserAccountMapper.cs

SDIR		= $(slicedir)\IceGrid
GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:library -out:$(TARGETS) -unsafe
MCSFLAGS	= $(MCSFLAGS) -keyfile:$(top_srcdir)\config\IcecsKey.snk

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) --ice -I$(slicedir)

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -r:$(bindir)\glacier2cs.dll -r:$(bindir)\icecs.dll $(SRCS) $(GEN_SRCS)

!if "$(NOGAC)" == ""

install:: all
	$(GACUTIL) -i $(bindir)\$(LIBNAME)

!else

install:: all
	copy $(bindir)\icegridcs.dll $(install_bindir)

!endif

!include .depend
