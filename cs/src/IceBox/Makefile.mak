# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

PKG		= iceboxcs
LIBNAME		= $(bindir)\$(PKG).dll
ICEBOXNET	= $(bindir)\iceboxnet.exe
ICEBOXADMINNET	= $(bindir)\iceboxadminnet.exe
TARGETS		= $(LIBNAME) $(ICEBOXNET) $(ICEBOXADMINNET)

L_SRCS		= AssemblyInfo.cs
I_SRCS		= Server.cs ServiceManagerI.cs
A_SRCS		= Admin.cs

GEN_SRCS	= $(GDIR)\IceBox.cs

SDIR		= $(slicedir)\IceBox
GDIR		= generated

!include $(top_srcdir)/config/Make.rules.mak

MCSFLAGS	= $(MCSFLAGS) -target:exe

LIB_MCSFLAGS	= -target:library -out:$(LIBNAME) -unsafe
LIB_MCSFLAGS	= $(LIB_MCSFLAGS) -keyfile:$(top_srcdir)\config\IcecsKey.snk

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) --checksum --ice -I. -I$(slicedir)

$(ICEBOXNET): $(I_SRCS) $(LIBNAME)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(LIBNAME) -r:$(bindir)\icecs.dll $(I_SRCS)

$(ICEBOXADMINNET): $(A_SRCS) $(LIBNAME)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(LIBNAME) -r:$(bindir)\icecs.dll $(A_SRCS)

$(LIBNAME): $(L_SRCS) $(GEN_SRCS)
	$(MCS) $(LIB_MCSFLAGS) -r:$(bindir)\icecs.dll $(L_SRCS) $(GEN_SRCS)

!if "$(NOGAC)" == ""

install:: all
	$(GACUTIL) -i $(LIBNAME)

!else

install:: all
	copy $(LIBNAME) $(install_bindir)

!endif

install:: all
	copy $(ICEBOXNET) $(install_bindir)
	copy $(ICEBOXADMINNET) $(install_bindir)

!include .depend
