# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBNAME = Glacier2

MODULES = $(LIBNAME)

TARGETS	= $(call mklibtargets,$(LIBNAME))

SLICES = $(SDIR)/Metrics.ice \
	$(SDIR)/PermissionsVerifier.ice \
	$(SDIR)/PermissionsVerifierF.ice \
	$(SDIR)/Router.ice \
	$(SDIR)/RouterF.ice \
	$(SDIR)/Session.ice \
	$(SDIR)/SSLInfo.ice

SDIR	= $(slicedir)/Glacier2

GEN_SRCS = $(patsubst 	$(SDIR)/%.ice, %.js,  $(SLICES))

SRCS		:= $(GEN_SRCS)
INSTALL_SRCS	:= Glacier2.js $(GEN_SRCS)

include $(top_srcdir)/config/Make.rules.js

SLICE2JSFLAGS	:= $(SLICE2JSFLAGS) --ice -I$(slicedir) --icejs

lint:: $(INSTALL_SRCS)
	jshint $(LINTFLAGS) $(INSTALL_SRCS)

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBNAME))
	$(call installmodule,$(DESTDIR)$(install_moduledir),$(INSTALL_SRCS),$(LIBNAME))
