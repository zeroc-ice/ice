# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../..

BASE		= libIcePack.so
VERSIONED_BASE	= $(BASE).$(VERSION)

NAME		= $(top_srcdir)/lib/$(BASE)
VERSIONED_NAME	= $(top_srcdir)/lib/$(VERSIONED_BASE)

CLIENT		= $(top_srcdir)/bin/icepackadmin
SERVER		= $(top_srcdir)/bin/icepack

TARGETS		= $(NAME) $(VERSIONED_NAME) $(CLIENT) $(SERVER)

OBJS		= Admin.o

COBJS		= Client.o

SOBJS		= Server.o \
		  Forward.o \
		  AdminI.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

HDIR		= $(includedir)/IcePack
IDIR		= $(slicedir)/IcePack
SLICECMD	= $(SLICE) --include-dir IcePack --dll-export ICE_PACK_API -I$(slicedir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I.. $(CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $(OBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(COBJS) -lIcePack $(LIBS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOBJS) -lIcePack $(LIBS)

$(HDIR)/Admin.h Admin.cpp: $(IDIR)/Admin.ice $(SLICE)
	rm -f $(HDIR)/Admin.h Admin.cpp
	$(SLICECMD) $(IDIR)/Admin.ice
	mv Admin.h $(HDIR)

clean::
	rm -f $(HDIR)/Admin.h Admin.cpp

$(HDIR)/AdminF.h: $(IDIR)/AdminF.ice $(SLICE)
	rm -f $(HDIR)/AdminF.h AdminF.cpp
	$(SLICECMD) $(IDIR)/AdminF.ice
	mv AdminF.h $(HDIR)
	rm -f AdminF.cpp

clean::
	rm -f $(HDIR)/AdminF.h

include .depend
