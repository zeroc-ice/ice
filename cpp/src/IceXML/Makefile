# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceXML,$(VERSION))
SONAME          = $(call mksoname,IceXML,$(SOVERSION))
LIBNAME		= $(call mklibname,IceXML)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)$(cpp11libdirsuffix)/$(LIBNAME))

OBJS		= Parser.o

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_XML_API_EXPORTS $(EXPAT_FLAGS)

LINKWITH        := -lIceUtil $(EXPAT_LIBS)

$(libdir)/$(LIBFILENAME): $(OBJS)
	@mkdir -p $(dir $@)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)$(cpp11libdirsuffix)/$(LIBNAME): $(libdir)/$(SONAME)
	@mkdir -p $(libdir)$(cpp11libdirsuffix)
	rm -f $@
	ln -s $(cpp11sonamedir)$(SONAME) $@

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
