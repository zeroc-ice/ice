#**********************************************************************
#
# Copyright (c) 2002
# Mutable Realms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../..

BASE		= libIcePatch.so
VERSIONED_BASE	= $(BASE).$(VERSION)

NAME		= $(top_srcdir)/lib/$(BASE)
VERSIONED_NAME	= $(top_srcdir)/lib/$(VERSIONED_BASE)

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
SLICE2CPPFLAGS	= -I$(slicedir)
SLICECMD	= $(SLICE2CPP) --ice --include-dir IcePatch --dll-export ICE_PATCH_API $(SLICE2CPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) -Xlinker -soname -Xlinker $(VERSIONED_BASE) $(LDFLAGS) -shared -o $@ $(OBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(COBJS) -lIcePatch -lGlacier $(SSLLIBS) $(LIBS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOBJS) -lIcePatch $(LIBS) $(OPENSSL_LIBS)

$(HDIR)/IcePatch.h IcePatch.cpp: $(SDIR)/IcePatch.ice $(SLICE2CPP)
	rm -f $(HDIR)/IcePatch.h IcePatch.cpp
	$(SLICECMD) $(SDIR)/IcePatch.ice
	mv IcePatch.h $(HDIR)

clean::
	rm -f $(HDIR)/IcePatch.h IcePatch.cpp

include .depend
