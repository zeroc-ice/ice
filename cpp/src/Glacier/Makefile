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

SLICECMD	= $(SLICE2CPP) --include-dir Glacier --dll-export GLACIER_API $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(OBJS)

$(ROUTER): $(ROBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(ROBJS) -lGlacier $(SSLLIBS) $(LIBS)

$(STARTER): $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) -lGlacier $(SSLLIBS) $(LIBS) -lcrypt

$(HDIR)/Starter.h Starter.cpp: $(SDIR)/Starter.ice $(SLICE2CPP)
	rm -f $(HDIR)/Starter.h Starter.cpp
	$(SLICECMD) $(SDIR)/Starter.ice
	mv Starter.h $(HDIR)

clean::
	rm -f $(HDIR)/Starter.h Starter.cpp

$(HDIR)/Router.h Router.cpp: $(SDIR)/Router.ice $(SLICE2CPP)
	rm -f $(HDIR)/Router.h Router.cpp
	$(SLICECMD) $(SDIR)/Router.ice
	mv Router.h $(HDIR)

clean::
	rm -f $(HDIR)/Router.h Router.cpp

$(HDIR)/Session.h Session.cpp: $(SDIR)/Session.ice $(SLICE2CPP)
	rm -f $(HDIR)/Session.h Session.cpp
	$(SLICECMD) $(SDIR)/Session.ice
	mv Session.h $(HDIR)

clean::
	rm -f $(HDIR)/Session.h Session.cpp

$(HDIR)/SessionF.h: $(SDIR)/SessionF.ice $(SLICE2CPP)
	rm -f $(HDIR)/SessionF.h SessionF.cpp
	$(SLICECMD) $(SDIR)/SessionF.ice
	mv SessionF.h $(HDIR)
	rm -f SessionF.cpp

clean::
	rm -f $(HDIR)/SessionF.h

$(HDIR)/SessionManager.h SessionManager.cpp: $(SDIR)/SessionManager.ice $(SLICE2CPP)
	rm -f $(HDIR)/SessionManager.h SessionManager.cpp
	$(SLICECMD) $(SDIR)/SessionManager.ice
	mv SessionManager.h $(HDIR)

clean::
	rm -f $(HDIR)/SessionManager.h SessionManager.cpp

$(HDIR)/SessionManagerF.h: $(SDIR)/SessionManagerF.ice $(SLICE2CPP)
	rm -f $(HDIR)/SessionManagerF.h SessionManagerF.cpp
	$(SLICECMD) $(SDIR)/SessionManagerF.ice
	mv SessionManagerF.h $(HDIR)
	rm -f SessionManagerF.cpp

clean::
	rm -f $(HDIR)/SessionManagerF.h

include .depend
