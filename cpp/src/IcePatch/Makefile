#**********************************************************************
#
# Copyright (c) 2002
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

top_srcdir	= ../..

NAME		= $(top_srcdir)/lib/libIcePatch.$(LIBEXT)
VERSIONED_NAME	= $(NAME).$(VERSION)

CLIENT		= $(top_srcdir)/bin/icepatchclient
SERVER		= $(top_srcdir)/bin/icepatchserver

TARGETS		= $(NAME) $(VERSIONED_NAME) $(CLIENT) $(SERVER)

OBJS		= IcePatch.o \
		  FileDescFactory.o \
		  Util.o \
		  ClientUtil.o

COBJS		= Client.o

SOBJS		= Server.o \
		  IcePatchI.o \
		  FileLocator.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/IcePatch.ice

HDIR		= $(includedir)/IcePatch
SDIR		= $(slicedir)/IcePatch

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS)
SLICE2CPPFLAGS	:= --ice --include-dir IcePatch --dll-export ICE_PATCH_API $(SLICE2CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(OBJS)

$(CLIENT): $(COBJS) $(NAME)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COBJS) -lIcePatch -lGlacier -lIceSSL $(LIBS) $(OPENSSL_LIBS)

$(SERVER): $(SOBJS) $(NAME)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) -lIcePatch $(LIBS) $(OPENSSL_LIBS)

include .depend
