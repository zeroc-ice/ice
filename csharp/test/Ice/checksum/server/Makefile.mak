# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..\..

TARGETS		= server.exe

SRCS		= Server.cs TestI.cs

SLICE_SRCS	= $(SDIR)\Test.ice \
		  $(SDIR)\Types.ice

GEN_SRCS	= $(GDIR)\Test.cs \
		  $(GDIR)\Types.cs

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) -I. -I"$(slicedir)"

server.exe: $(SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" $(SRCS) $(GEN_SRCS)
