# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
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

ROUTER		= $(top_srcdir)/bin/glacier
STARTER		= $(top_srcdir)/bin/glacierstarter

TARGETS		= $(NAME) $(VERSIONED_NAME) $(ROUTER) $(STARTER)

OBJS		= Glacier.o

ROBJS		= GlacierRouter.o \
		  RouterI.o \
		  ClientBlobject.o \
		  ServerBlobject.o \
                  CertVerifier.o

SOBJS		= GlacierStarter.o \
		  GlacierI.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(ROBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

HDIR		= $(includedir)/Glacier
SDIR		= $(slicedir)/Glacier
SLICECMD	= $(SLICE2CPP) --include-dir Glacier --dll-export GLACIER_API -I$(slicedir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $(OBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

$(ROUTER): $(ROBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(ROBJS) $(LIBS)

$(STARTER): $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOBJS) -lGlacier $(LIBS)

$(HDIR)/Glacier.h Glacier.cpp: $(SDIR)/Glacier.ice $(SLICE2CPP)
	rm -f $(HDIR)/Glacier.h Glacier.cpp
	$(SLICECMD) $(SDIR)/Glacier.ice
	mv Glacier.h $(HDIR)

clean::
	rm -f $(HDIR)/Glacier.h Glacier.cpp

include .depend
