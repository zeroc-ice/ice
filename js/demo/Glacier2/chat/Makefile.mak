# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS = Chat.js

!include $(top_srcdir)\config\Make.rules.mak.js

SLICE2JSFLAGS	= $(SLICE2JSFLAGS) -I"$(slicedir)"
