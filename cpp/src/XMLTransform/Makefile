# **********************************************************************
#
# Copyright (c) 2002
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

top_srcdir	= ../..

NAME		= $(top_srcdir)/lib/libXMLTransform.$(LIBEXT)
VERSIONED_NAME	= $(NAME).$(VERSION)

TRANSFORM_DB	= $(top_srcdir)/bin/transformdb

TARGETS		= $(NAME) $(VERSIONED_NAME) $(TRANSFORM_DB)

OBJS		= ErrorReporter.o \
		  XMLTransform.o

TRANSFORM_OBJS	= TransformDB.o

SRCS		= $(OBJS:.o=.cpp) \
                  $(TRANSFORM_OBJS:.o=.cpp)

HDIR		= $(includedir)/XMLTransform
SDIR		= $(slicedir)/XMLTransform

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) $(XERCESC_FLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(OBJS)

$(TRANSFORM_DB): $(TRANSFORM_OBJS) $(NAME)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(TRANSFORM_OBJS) -lFreeze -lIceXML -lXMLTransform $(LIBS) $(XERCESC_LIBS) $(DB_LIBS)

clean::
	rm -f $(TRANSFORM_DB)

install:: all
	$(INSTALL_LIBRARY) $(VERSIONED_NAME) $(install_libdir)
	rm -f $(install_libdir)/$(notdir $(NAME))
	ln -s $(notdir $(VERSIONED_NAME)) $(install_libdir)/$(notdir $(NAME))
	$(INSTALL_PROGRAM) $(TRANSFORM_DB) $(install_bindir)

include .depend
