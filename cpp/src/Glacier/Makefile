# **********************************************************************
#
# Copyright (c) 2001
# Mutable Realms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../..

BASE		= libGlacier.so
VERSIONED_BASE	= $(BASE).$(VERSION)

NAME		= $(top_srcdir)/lib/$(BASE)
VERSIONED_NAME	= $(top_srcdir)/lib/$(VERSIONED_BASE)

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
SLICE2CPPFLAGS	= -I$(slicedir)
SLICECMD	= $(SLICE2CPP) --include-dir Glacier --dll-export GLACIER_API $(SLICE2CPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) -Xlinker -soname -Xlinker $(VERSIONED_BASE) $(LDFLAGS) -shared -o $@ $(OBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

$(ROUTER): $(ROBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(ROBJS) -lGlacier $(SSLLIBS) $(LIBS)

$(STARTER): $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOBJS) -lGlacier $(SSLLIBS) $(LIBS)

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
