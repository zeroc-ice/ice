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
		  DBException.o \
		  Evictor.o \
	  	  DBI.o \
		  EvictorI.o

SRCS		= $(OBJS:.o=.cpp)

HDIR		= $(includedir)/Freeze
IDIR		= $(slicedir)/Freeze
SLICECMD	= $(SLICE2CPP) --include-dir Freeze --dll-export FREEZE_API -I$(slicedir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -I$(DB)/include
LDFLAGS		:= $(LDFLAGS) -L$(DB)/lib

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $(OBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

$(HDIR)/DB.h DB.cpp: $(IDIR)/DB.ice $(SLICE2CPP)
	rm -f $(HDIR)/DB.h DB.cpp
	$(SLICECMD) $(IDIR)/DB.ice
	mv DB.h $(HDIR)

clean::
	rm -f $(HDIR)/DB.h DB.cpp

$(HDIR)/DBF.h: $(IDIR)/DBF.ice $(SLICE2CPP)
	rm -f $(HDIR)/DBF.h DBF.cpp
	$(SLICECMD) $(IDIR)/DBF.ice
	mv DBF.h $(HDIR)
	rm -f DBF.cpp

clean::
	rm -f $(HDIR)/DBF.h

$(HDIR)/DBException.h DBException.cpp: $(IDIR)/DBException.ice $(SLICE2CPP)
	rm -f $(HDIR)/DBException.h DBException.cpp
	$(SLICECMD) $(IDIR)/DBException.ice
	mv DBException.h $(HDIR)

clean::
	rm -f $(HDIR)/DBException.h DBException.cpp

$(HDIR)/Evictor.h Evictor.cpp: $(IDIR)/Evictor.ice $(SLICE2CPP)
	rm -f $(HDIR)/Evictor.h Evictor.cpp
	$(SLICECMD) $(IDIR)/Evictor.ice
	mv Evictor.h $(HDIR)

clean::
	rm -f $(HDIR)/Evictor.h Evictor.cpp

$(HDIR)/EvictorF.h: $(IDIR)/EvictorF.ice $(SLICE2CPP)
	rm -f $(HDIR)/EvictorF.h EvictorF.cpp
	$(SLICECMD) $(IDIR)/EvictorF.ice
	mv EvictorF.h $(HDIR)
	rm -f EvictorF.cpp

clean::
	rm -f $(HDIR)/EvictorF.h

include .depend
