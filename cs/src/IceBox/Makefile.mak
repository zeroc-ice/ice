# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

PKG		= iceboxcs
LIBNAME		= $(PKG).dll
TARGETS		= $(bindir)\iceboxnet.exe $(bindir)\$(LIBNAME)

L_SRCS		= AssemblyInfo.cs
I_SRCS		= Server.cs ServiceManagerI.cs

SLICE_SRCS	= $(SDIR)\IceBox.ice

GEN_SRCS	= $(GDIR)\IceBox.cs

SDIR		= $(slicedir)\IceBox
GDIR		= generated

!include $(top_srcdir)/config/Make.rules.mak

MCSFLAGS	= $(MCSFLAGS) -target:exe

LIB_MCSFLAGS	= -target:library -out:$(bindir)\$(LIBNAME) -unsafe

!if "$(DOTNET_1)" != "yes"
LIB_MCSFLAGS	= $(LIB_MCSFLAGS) -keyfile:$(top_srcdir)\config\IcecsKey.snk
!endif

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) --checksum --ice -I. -I$(slicedir)

$(bindir)\iceboxnet.exe: $(I_SRCS) $(bindir)\$(LIBNAME)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(bindir)\$(LIBNAME) -r:$(bindir)\icecs.dll $(I_SRCS)

$(bindir)\$(LIBNAME): $(L_SRCS) $(GEN_SRCS)
	$(MCS) $(LIB_MCSFLAGS) -r:$(bindir)\icecs.dll $(L_SRCS) $(GEN_SRCS)

!if "$(NOGAC)" == ""

install:: all
	$(GACUTIL) -i $(bindir)/$(LIBNAME)

!else

install:: all
	copy $(bindir)\iceboxcs.dll $(install_bindir)

!endif

install:: all
	copy $(bindir)\iceboxnet.exe $(install_bindir)

!include .depend
