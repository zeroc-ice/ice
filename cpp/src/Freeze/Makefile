# **********************************************************************
#
# Copyright (c) 2003
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

NAME		= $(top_srcdir)/lib/libFreeze.$(LIBEXT)
VERSIONED_NAME	= $(NAME).$(VERSION)

TARGETS		= $(NAME) $(VERSIONED_NAME)

OBJS		= DB.o \
		  DBException.o \
		  Evictor.o \
	  	  DBI.o \
		  EvictorI.o \
		  IdentityObjectDict.o \
		  Application.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/DB.ice \
		  $(SDIR)/DBF.ice \
		  $(SDIR)/DBException.ice \
		  $(SDIR)/Evictor.ice \
		  $(SDIR)/EvictorF.ice

HDIR		= $(includedir)/Freeze
SDIR		= $(slicedir)/Freeze

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir Freeze $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(DB_FLAGS) $(CPPFLAGS)
SLICE2CPPFLAGS	:= --ice --include-dir Freeze --dll-export FREEZE_API $(SLICE2CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(OBJS)

../Freeze/IdentityObjectDict.h IdentityObjectDict.cpp: $(SLICE2FREEZE)
	rm -f IdentityObjectDict.h IdentityObjectDict.cpp
	$(SLICE2FREEZECMD) --dict Freeze::IdentityObjectDict,Ice::Identity,Object IdentityObjectDict \
	$(slicedir)/Ice/Identity.ice

clean::
	rm -f IdentityObjectDict.h IdentityObjectDict.cpp

install:: all
	$(INSTALL_LIBRARY) $(VERSIONED_NAME) $(install_libdir)
	rm -f $(install_libdir)/$(notdir $(NAME))
	ln -s $(notdir $(VERSIONED_NAME)) $(install_libdir)/$(notdir $(NAME))

include .depend
