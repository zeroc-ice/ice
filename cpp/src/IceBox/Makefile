#**********************************************************************
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

NAME		= $(top_srcdir)/lib/libIceBox.$(LIBEXT)
VERSIONED_NAME	= $(NAME).$(VERSION)

SERVER		= $(top_srcdir)/bin/icebox
ADMIN		= $(top_srcdir)/bin/iceboxadmin

TARGETS		= $(NAME) $(VERSIONED_NAME) $(SERVER) $(ADMIN)

OBJS		= IceBox.o \
		  Exception.o

SOBJS		= ServiceManagerI.o \
		  Server.o

AOBJS		= Admin.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(AOBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/IceBox.ice

HDIR		= $(includedir)/IceBox
SDIR		= $(slicedir)/IceBox


include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(DB_FLAGS) $(CPPFLAGS)
SLICE2CPPFLAGS	:= --ice --include-dir IceBox $(SLICE2CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(OBJS)

$(SERVER): $(SOBJS) $(NAME)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) -lIceBox -lFreeze -lIceXML $(LIBS) $(DB_LIBS)

$(ADMIN): $(AOBJS) $(NAME)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(AOBJS) -lIceBox $(LIBS)

install:: all
	$(INSTALL_LIBRARY) $(VERSIONED_NAME) $(install_libdir)
	rm -f $(install_libdir)/$(notdir $(NAME))
	ln -s $(notdir $(VERSIONED_NAME)) $(install_libdir)/$(notdir $(NAME))
	$(INSTALL_PROGRAM) $(SERVER) $(install_bindir)
	$(INSTALL_PROGRAM) $(ADMIN) $(install_bindir)

include .depend
