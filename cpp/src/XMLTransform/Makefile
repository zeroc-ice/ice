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

BASE		= libXMLTransform.so
VERSIONED_BASE	= $(BASE).$(VERSION)

NAME		= $(top_srcdir)/lib/$(BASE)
VERSIONED_NAME	= $(top_srcdir)/lib/$(VERSIONED_BASE)

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
LDFLAGS		:= $(LDFLAGS) $(XERCESC_LIBS) -L$(DB_HOME)/lib

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) -Xlinker -soname -Xlinker $(VERSIONED_BASE) $(LDFLAGS) -shared -o $@ $(OBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

$(TRANSFORM_DB): $(TRANSFORM_OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(TRANSFORM_OBJS) -lFreeze -lIceXML -lXMLTransform $(LIBS) -ldb

clean::
	rm -f $(TRANSFORM_DB)

include .depend
