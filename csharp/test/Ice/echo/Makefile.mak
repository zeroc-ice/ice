# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir  = ..\..\..

TARGETS   = server.exe

S_SRCS    = Server.cs BlobjectI.cs

GEN_SRCS  = $(GDIR)\Test.cs

SDIR      = .

GDIR      = generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS  = $(MCSFLAGS) -target:exe

client.exe: $(C_SRCS) $(GEN_SRCS)
  $(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" $(C_SRCS) $(GEN_SRCS)

server.exe: $(S_SRCS) $(GEN_SRCS)
  $(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" $(S_SRCS) $(GEN_SRCS)
