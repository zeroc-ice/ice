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
		  EvictorI.o \
		  IdentityObjectDict.o \
		  Application.o

SRCS		= $(OBJS:.o=.cpp)

HDIR		= $(includedir)/Freeze
SDIR		= $(slicedir)/Freeze
SLICECMD	= $(SLICE2CPP) --include-dir Freeze --dll-export FREEZE_API -I$(slicedir)
SLICE2FREEZECMD = $(SLICE2FREEZE) --include-dir Freeze -I$(slicedir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -I$(DB_HOME)/include
LDFLAGS		:= $(LDFLAGS) -L$(DB_HOME)/lib

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) -Xlinker -soname -Xlinker $(VERSIONED_BASE) $(LDFLAGS) -shared -o $@ $(OBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

$(HDIR)/DB.h DB.cpp: $(SDIR)/DB.ice $(SLICE2CPP)
	rm -f $(HDIR)/DB.h DB.cpp
	$(SLICECMD) $(SDIR)/DB.ice
	mv DB.h $(HDIR)

clean::
	rm -f $(HDIR)/DB.h DB.cpp

$(HDIR)/DBF.h: $(SDIR)/DBF.ice $(SLICE2CPP)
	rm -f $(HDIR)/DBF.h DBF.cpp
	$(SLICECMD) $(SDIR)/DBF.ice
	mv DBF.h $(HDIR)
	rm -f DBF.cpp

clean::
	rm -f $(HDIR)/DBF.h

$(HDIR)/DBException.h DBException.cpp: $(SDIR)/DBException.ice $(SLICE2CPP)
	rm -f $(HDIR)/DBException.h DBException.cpp
	$(SLICECMD) $(SDIR)/DBException.ice
	mv DBException.h $(HDIR)

clean::
	rm -f $(HDIR)/DBException.h DBException.cpp

$(HDIR)/Evictor.h Evictor.cpp: $(SDIR)/Evictor.ice $(SLICE2CPP)
	rm -f $(HDIR)/Evictor.h Evictor.cpp
	$(SLICECMD) $(SDIR)/Evictor.ice
	mv Evictor.h $(HDIR)

clean::
	rm -f $(HDIR)/Evictor.h Evictor.cpp

$(HDIR)/EvictorF.h: $(SDIR)/EvictorF.ice $(SLICE2CPP)
	rm -f $(HDIR)/EvictorF.h EvictorF.cpp
	$(SLICECMD) $(SDIR)/EvictorF.ice
	mv EvictorF.h $(HDIR)
	rm -f EvictorF.cpp

clean::
	rm -f $(HDIR)/EvictorF.h

IdentityObjectDict.h IdentityObjectDict.cpp: $(SLICE2FREEZE)
	rm -f IdentityObjectDict.h IdentityObjectDict.cpp
	$(SLICE2FREEZECMD) --dict Freeze::IdentityObjectDict,Ice::Identity,Object IdentityObjectDict \
	$(slicedir)/Ice/Identity.ice

clean::
	rm -f IdentityObjectDict.h IdentityObjectDict.cpp

include .depend
