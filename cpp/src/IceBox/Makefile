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

SLICECMD	= $(SLICE2CPP) --ice --include-dir IceBox $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. -I$(DB_HOME)/include $(CPPFLAGS)
LDFLAGS         := $(LDFLAGS) -L$(DB_HOME)/lib

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(OBJS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) -lIceBox -lFreeze -lIceXML $(LIBS) -ldb

$(ADMIN): $(AOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(AOBJS) -lIceBox $(LIBS)

$(HDIR)/IceBox.h IceBox.cpp: $(SDIR)/IceBox.ice $(SLICE2CPP)
	rm -f $(HDIR)/IceBox.h IceBox.cpp
	$(SLICECMD) --dll-export ICE_BOX_API $(SDIR)/IceBox.ice
	mv IceBox.h $(HDIR)

clean::
	rm -f $(HDIR)/IceBox.h IceBox.cpp

include .depend
