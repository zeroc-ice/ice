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

ROUTER		= $(top_srcdir)/bin/glacier

TARGETS		= $(ROUTER)

ROBJS		= GlacierRouter.o \
		  RouterI.o \
		  ClientBlobject.o \
		  ServerBlobject.o

SRCS		= $(ROBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS)

$(ROUTER): $(ROBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(ROBJS) $(LIBS)

include .depend
