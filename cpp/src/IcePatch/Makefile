#**********************************************************************
#
# Copyright (c) 2002
# MutableRealms, Inc.
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

TARGETS		= $(NAME) $(CLIENT) $(SERVER)

OBJS		= IcePatch.o \
		  FileDescFactory.o \
		  Util.o

COBJS		= Client.o

SOBJS		= Server.o \
		  IcePatchI.o \
		  FileLocator.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

HDIR		= $(includedir)/IcePatch
SDIR		= $(slicedir)/IcePatch
SLICECMD	= $(SLICE2CPP) --include-dir IcePatch --dll-export ICE_PATCH_API -I$(slicedir)

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
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOBJS) -lIcePatch $(OPENSSL_LIBS) $(LIBS)

$(HDIR)/IcePatch.h IcePatch.cpp: $(SDIR)/IcePatch.ice $(SLICE2CPP)
	rm -f $(HDIR)/IcePatch.h IcePatch.cpp
	$(SLICECMD) $(SDIR)/IcePatch.ice
	mv IcePatch.h $(HDIR)

clean::
	rm -f $(HDIR)/IcePatch.h IcePatch.cpp

include .depend
