#**********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../..

BASE		= libIceStorm.so
VERSIONED_BASE	= $(BASE).$(VERSION)

NAME		= $(top_srcdir)/lib/$(BASE)
VERSIONED_NAME	= $(top_srcdir)/lib/$(VERSIONED_BASE)

SERVER		= $(top_srcdir)/bin/icestorm
ADMIN		= $(top_srcdir)/bin/icestormadmin
TARGETS		= $(NAME) $(VERSIONED_NAME) $(SERVER) $(ADMIN)

OBJS		= IceStorm.o

SOBJS		= TraceLevels.o Flusher.o Subscriber.o TopicI.o TopicManagerI.o Server.o

AOBJS		= Admin.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(AOBJS:.o=.cpp)

HDIR		= $(includedir)/IceStorm
SDIR		= $(slicedir)/IceStorm
SLICECMD	= $(SLICE2CPP) --include-dir IceStorm --dll-export ICE_STORM_API -I$(slicedir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared \
	-o $@ $(OBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOBJS) -lIceStorm $(LIBS)

$(ADMIN): $(AOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(AOBJS) -lIceStorm $(LIBS)

$(HDIR)/IceStorm.h IceStorm.cpp: $(SDIR)/IceStorm.ice $(SLICE2CPP)
	rm -f $(HDIR)/IceStorm.h IceStorm.cpp
	$(SLICECMD) $(SDIR)/IceStorm.ice
	mv IceStorm.h $(HDIR)


clean::
	rm -f $(HDIR)/Clock.h Clock.cpp

include .depend
