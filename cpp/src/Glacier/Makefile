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

NAME		= $(top_srcdir)/lib/libGlacier.$(LIBEXT)
VERSIONED_NAME	= $(NAME).$(VERSION)

ROUTER		= $(top_srcdir)/bin/glacierrouter
STARTER		= $(top_srcdir)/bin/glacierstarter

TARGETS		= $(NAME) $(VERSIONED_NAME) $(ROUTER) $(STARTER)

OBJS		= Starter.o \
		  Router.o \
		  Session.o \
		  SessionManager.o

ROBJS		= GlacierRouter.o \
		  RouterI.o \
		  Blobject.o \
		  ClientBlobject.o \
		  ServerBlobject.o \
		  Missive.o

SOBJS		= GlacierStarter.o \
		  StarterI.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(ROBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/Starter.ice \
		  $(SDIR)/Router.ice \
		  $(SDIR)/Session.ice \
		  $(SDIR)/SessionManager.ice \
		  $(SDIR)/SessionF.ice \
		  $(SDIR)/SessionManagerF.ice

HDIR		= $(includedir)/Glacier
SDIR		= $(slicedir)/Glacier

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS)
SLICE2CPPFLAGS	:= --include-dir Glacier --dll-export GLACIER_API $(SLICE2CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(OBJS)

$(ROUTER): $(ROBJS) $(NAME)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(ROBJS) -lGlacier -lIceSSL $(LIBS) $(OPENSSL_LIBS)

$(STARTER): $(SOBJS) $(NAME)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) -lGlacier -lIceSSL $(LIBS) $(OPENSSL_LIBS) -lcrypt

install:: all
	$(INSTALL_LIBRARY) $(VERSIONED_NAME) $(install_libdir)
	rm -f $(install_libdir)/$(notdir $(NAME))
	ln -s $(notdir $(VERSIONED_NAME)) $(install_libdir)/$(notdir $(NAME))
	$(INSTALL_PROGRAM) $(ROUTER) $(install_bindir)
	$(INSTALL_PROGRAM) $(STARTER) $(install_bindir)

include .depend
