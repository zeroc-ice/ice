# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..


SLICE_OBJS	= .\CircularA.obj \
		  .\CircularB.obj

OBJS 		= $(SLICE_OBJS)

TARGETS		= $(OBJS)

!include $(top_srcdir)/config/Make.rules.mak

SLICE2CPPFLAGS	= -I. $(SLICE2CPPFLAGS)
CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

clean::
	del /q CircularA.cpp CircularA.h
	del /q CircularB.cpp CircularB.h
