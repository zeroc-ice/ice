# **********************************************************************
#
# Copyright (c) 2001
# Mutable Realms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

OBJS		= Pinger.o \

COBJS		= Client.o 

SOBJS		= Server.o \

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= Pinger.ice

SLICE2CPPFLAGS	= -I$(slicedir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(SSLLIBS) $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(SSLLIBS) $(LIBS)

Pinger.h Pinger.cpp: Pinger.ice $(SLICE2CPP)
	rm -f Pinger.h Pinger.cpp
	$(SLICE2CPP) $(SLICE2CPPFLAGS) Pinger.ice

clean::
	rm -f Pinger.h Pinger.cpp

include .depend
