# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBNAME		= glaciercs.dll
TARGETS		= $(libdir)/$(LIBNAME)

SRCS		=

SLICE_SRCS	= $(SDIR)/Router.ice \
		  $(SDIR)/Session.ice \
		  $(SDIR)/SessionF.ice \
		  $(SDIR)/SessionManager.ice \
		  $(SDIR)/SessionManagerF.ice \
		  $(SDIR)/Starter.ice

SDIR		= $(slicedir)/Glacier
GDIR		= generated

include $(top_srcdir)/config/Make.rules

MCSFLAGS	:= $(MCSFLAGS) -target:library -out:$(TARGETS) -unsafe

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) -I$(slicedir)

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) $^

install:: all
	$(INSTALL) $(libdir)/$(LIBNAME) $(install_libdir)

include .depend
