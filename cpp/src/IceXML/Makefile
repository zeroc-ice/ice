# **********************************************************************
#
# Copyright (c) 2003 - 2004
# ZeroC, Inc.
# North Palm Beach, FL, USA
#
# All Rights Reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************


top_srcdir	= ../..
 
LIBFILENAME     = $(call mklibfilename,IceXML,$(VERSION))
SONAME          = $(call mksoname,IceXML,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceXML)

TARGETS		= $(libdir)/$(LIBFILENAME) $(libdir)/$(SONAME) $(libdir)/$(LIBNAME)

OBJS		= Parser.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -DICE_XML_API_EXPORTS $(EXPAT_FLAGS)

LINKWITH        := -lIceUtil $(EXPAT_LIBS)

$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

install:: all
	$(INSTALL_LIBRARY) $(libdir)/$(LIBFILENAME) $(install_libdir)
	rm -f $(install_libdir)/$(SONAME)
	ln -s $(LIBFILENAME) $(install_libdir)/$(SONAME)
	rm -f $(install_libdir)/$(LIBNAME)
	ln -s $(SONAME) $(install_libdir)/$(LIBNAME)



include .depend
