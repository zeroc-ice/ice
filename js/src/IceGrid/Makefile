# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBNAME = IceGrid

TARGETS	= $(call mklibtargets,$(LIBNAME))

SLICES = $(SDIR)/Admin.ice \
	$(SDIR)/Descriptor.ice \
	$(SDIR)/Exception.ice \
	$(SDIR)/FileParser.ice \
	$(SDIR)/Locator.ice \
	$(SDIR)/Observer.ice \
	$(SDIR)/Query.ice \
	$(SDIR)/Registry.ice \
	$(SDIR)/Session.ice \
	$(SDIR)/UserAccountMapper.ice

SDIR	= $(slicedir)/IceGrid

GEN_SRCS = $(patsubst $(SDIR)/%.ice, %.js, $(SLICES))

SRCS		:= $(GEN_SRCS)
INSTALL_SRCS	:= IceGrid.js $(GEN_SRCS)

include $(top_srcdir)/config/Make.rules.js

SLICE2JSFLAGS	:= $(SLICE2JSFLAGS) --ice -I$(slicedir)

lint:: $(INSTALL_SRCS)
	jshint $(LINTFLAGS) $(INSTALL_SRCS)

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBNAME))
	$(call installmodule,$(DESTDIR)$(install_moduledir),$(INSTALL_SRCS),$(LIBNAME))
