# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

OBJS		= Pinger.o

COBJS		= Client.o 

SOBJS		= Server.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= Pinger.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(OPENSSL_FLAGS) $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(OPENSSL_RPATH_LINK) -lIceSSL $(EXPAT_RPATH_LINK) -lIceXML $(LIBS) $(OPENSSL_LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(OPENSSL_RPATH_LINK) -lIceSSL $(EXPAT_RPATH_LINK) -lIceXML $(LIBS) $(OPENSSL_LIBS)

include .depend
