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

BASE		= libIceBox.so
VERSIONED_BASE	= $(BASE).$(VERSION)

NAME		= $(top_srcdir)/lib/$(BASE)
VERSIONED_NAME	= $(top_srcdir)/lib/$(VERSIONED_BASE)

SERVER		= $(top_srcdir)/bin/icebox
ADMIN		= $(top_srcdir)/bin/iceboxadmin

TARGETS		= $(NAME) $(VERSIONED_NAME) $(SERVER) $(ADMIN)

OBJS		= IceBox.o \
		  Exception.o

SOBJS		= ServiceManagerI.o \
		  Server.o

AOBJS		= Admin.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(AOBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/IceBox.ice

HDIR		= $(includedir)/IceBox
SDIR		= $(slicedir)/IceBox

SLICE2CPPFLAGS	= -I$(slicedir)
SLICECMD	= $(SLICE2CPP) --ice --include-dir IceBox $(SLICE2CPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. -I$(DB_HOME)/include $(CPPFLAGS)
LDFLAGS         := $(LDFLAGS) -L$(DB_HOME)/lib

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) -Xlinker -soname -Xlinker $(VERSIONED_BASE) $(LDFLAGS) -shared -o $@ $(OBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOBJS) -lIceBox -lFreeze -lIceXML $(LIBS) -ldb

$(ADMIN): $(AOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(AOBJS) -lIceBox $(LIBS)

$(HDIR)/IceBox.h IceBox.cpp: $(SDIR)/IceBox.ice $(SLICE2CPP)
	rm -f $(HDIR)/IceBox.h IceBox.cpp
	$(SLICECMD) --dll-export ICE_BOX_API $(SDIR)/IceBox.ice
	mv IceBox.h $(HDIR)

clean::
	rm -f $(HDIR)/IceBox.h IceBox.cpp

include .depend
