#**********************************************************************
#
# Copyright (c) 2002
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../..

SERVER		= $(top_srcdir)/bin/icepatch

TARGETS		= $(SERVER)

OBJS		= Node.o

SOBJS		= Server.o \
		  NodeI.o \
		  NodeLocator.o

COBJS		=

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

HDIR		= $(includedir)/IcePatch
SDIR		= $(slicedir)/IcePatch
SLICECMD	= $(SLICE2CPP) --include-dir IcePatch -I$(slicedir)
SLICE2FREEZECMD = $(SLICE2FREEZE) --include-dir IcePatch -I$(slicedir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

$(HDIR)/Node.h Node.cpp: $(SDIR)/Node.ice $(SLICE2CPP)
	rm -f $(HDIR)/Node.h Node.cpp
	$(SLICECMD) $(SDIR)/Node.ice
	mv Node.h $(HDIR)

clean::
	rm -f $(HDIR)/Node.h Node.cpp

include .depend
