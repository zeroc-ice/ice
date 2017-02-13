# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

LIBFILENAME	= $(call mklibfilename,TestPlugin,$(VERSION))
SONAME		= $(call mksoname,TestPlugin,$(SOVERSION))
LIBNAME		= $(call mklibname,TestPlugin)
CLIENT		= $(call mktestname,client)
PLUGINDIR	= plugins

TARGETS		= $(CLIENT) \
		  $(call mklibtargets,$(PLUGINDIR)/$(LIBFILENAME),$(PLUGINDIR)/$(SONAME),$(PLUGINDIR)/$(LIBNAME))

COBJS		= Client.o \

POBJS		= Plugin.o

OBJS		= $(COBJS) \
		  $(POBJS)

LINKWITH        := -lIceUtil -lIce

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(call mktest,$@,$(COBJS),$(LIBS))

$(PLUGINDIR)/$(LIBFILENAME): $(POBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(POBJS),$(LINKWITH))

$(PLUGINDIR)/$(SONAME): $(PLUGINDIR)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(PLUGINDIR)/$(LIBNAME): $(PLUGINDIR)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@
