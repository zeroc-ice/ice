# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
CLIENTF		= clientFail

TARGETS		= $(CLIENT) $(CLIENTF)

COBJS		= Client.o

CFOBJS		= ClientFail.o

OBJS		= $(COBJS) $(CFOBJS)

GENPIC          = no

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(COBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

$(CLIENTF): $(CFOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(CFOBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)


clean::
	-rm -rf db/*
