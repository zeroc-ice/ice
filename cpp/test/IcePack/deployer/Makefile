# **********************************************************************
#
# Copyright (c) 2001
# ZeroC, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server
SERVICE		= libTestService.so
VERSIONED_SVC	= $(SERVICE).$(VERSION)

TARGETS		= $(CLIENT) $(SERVER) $(SERVICE) $(VERSIONED_SVC)

OBJS		= Test.o

COBJS		= Client.o \
		  AllTests.o

SOBJS		= TestI.o \
		  Server.o

SERVICE_OBJS	= TestI.o \
		  Service.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(SERVICE_OBJS:.o=.cpp)

SLICE_SRCS	= Test.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) -lIcePack -lIceBox -lYellow $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

$(VERSIONED_SVC): $(OBJS) $(SERVICE_OBJS)
	rm -f $@
	$(CXX) -Xlinker -soname -Xlinker $(VERSIONED_SERVICE) $(LDFLAGS) -shared -o $@ $(OBJS) $(SERVICE_OBJS)

$(SERVICE): $(VERSIONED_SVC)
	rm -f $@
	ln -s $(VERSIONED_SVC) $@


Test.h Test.cpp: Test.ice $(SLICE2CPP)
	rm -f Test.h Test.cpp
	$(SLICE2CPP) Test.ice

clean::
	rm -f Test.h Test.cpp

include .depend
