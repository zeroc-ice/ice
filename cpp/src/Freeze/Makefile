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

BASE		= libFreeze.so
VERSIONED_BASE	= $(BASE).$(VERSION)

NAME		= $(top_srcdir)/lib/$(BASE)
VERSIONED_NAME	= $(top_srcdir)/lib/$(VERSIONED_BASE)

TARGETS		= $(NAME) $(VERSIONED_NAME)

OBJS		= DB.o \
	  	  DBI.o

SRCS		= $(OBJS:.o=.cpp)

HDIR		= $(includedir)/Freeze
IDIR		= $(slicedir)/Freeze
SLICECMD	= $(SLICE) --include-dir Freeze --dll-export FREEZE_API -I$(slicedir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $(OBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

$(HDIR)/DB.h DB.cpp: $(IDIR)/DB.ice $(SLICE)
	rm -f $(HDIR)/DB.h DB.cpp
	$(SLICECMD) $(IDIR)/DB.ice
	mv DB.h $(HDIR)

clean::
	rm -f $(HDIR)/DB.h DB.cpp

$(HDIR)/DBF.h: $(IDIR)/DBF.ice $(SLICE)
	rm -f $(HDIR)/DBF.h DBF.cpp
	$(SLICECMD) $(IDIR)/DBF.ice
	mv DBF.h $(HDIR)
	rm -f DBF.cpp

clean::
	rm -f $(HDIR)/DBF.h

include .depend
