# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

PLUGINLIBFILENAME	= $(call mklibfilename,TestPlugin,$(VERSION))
PLUGINSONAME		= $(call mksoname,TestPlugin,$(SOVERSION))  
PLUGINLIBNAME		= $(call mklibname,TestPlugin)
CLIENT			= client
PLUGINDIR		= plugins

TARGETS		= $(CLIENT) $(call mklibtargets,$(PLUGINDIR)/$(PLUGINLIBFILENAME),$(PLUGINDIR)/$(PLUGINSONAME),$(PLUGINDIR)/$(PLUGINLIBNAME))

COBJS		= Client.o \

POBJS		= Plugin.o

SRCS		= $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(POBJS:.o=.cpp)

LINKWITH        := -lIceUtil -lIce

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COBJS) $(LIBS)

$(PLUGINDIR)/$(PLUGINLIBFILENAME): $(POBJS)
	rm -f $@
	$(call mkshlib,$@,$(PLUGINSONAME),$(POBJS),$(LINKWITH))

$(PLUGINDIR)/$(PLUGINSONAME): $(PLUGINDIR)/$(PLUGINLIBFILENAME)
	rm -f $@
	ln -s $(PLUGINLIBFILENAME) $@

$(PLUGINDIR)/$(PLUGINLIBNAME): $(PLUGINDIR)/$(PLUGINSONAME)
	rm -f $@
	ln -s $(PLUGINSONAME) $@


include .depend
