# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server

LOGGERFILENAME	= $(call mklibfilename,LoggerPlugin)
LOGGERSONAME	= $(call mksoname,LoggerPlugin,$(SOVERSION))

HELLOFILENAME	= $(call mklibfilename,HelloPlugin)
HELLOSONAME	= $(call mksoname,HelloPlugin,$(SOVERSION))

TARGETS		= $(CLIENT) $(SERVER) $(LOGGERFILENAME) $(LOGGERSONAME) $(HELLOFILENAME) $(HELLOSONAME)

OBJS		= Hello.o

COBJS		= Client.o

SOBJS		= Server.o

LOBJS		= LoggerPluginI.o

HOBJS		= HelloPluginI.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(LOBJS:.o=.cpp) \
		  $(HOBJS:.o=.cpp)

SLICE_SRCS	= Hello.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

$(LOGGERFILENAME): $(LOBJS)
	rm -f $@
	$(call mkshlib,$@,$(LOGGERSONAME),$(LOBJS),$(LIBS))

$(LOGGERSONAME): $(LOGGERFILENAME)
	rm -f $@
	ln -s $(LOGGERFILENAME) $@

$(HELLOFILENAME): $(OBJS) $(HOBJS)
	rm -f $@
	$(call mkshlib,$@,$(HELLOSONAME),$(OBJS) $(HOBJS),$(LIBS))

$(HELLOSONAME): $(HELLOFILENAME)
	rm -f $@
	ln -s $(HELLOFILENAME) $@

include .depend
