#**********************************************************************
#
# Copyright (c) 2003
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

NAME		= $(top_srcdir)/lib/libIceStorm.$(LIBEXT)
VERSIONED_NAME	= $(NAME).$(VERSION)

SNAME		= $(top_srcdir)/lib/libIceStormService.$(LIBEXT)
VERSIONED_SNAME	= $(SNAME).$(VERSION)

ADMIN		= $(top_srcdir)/bin/icestormadmin

TARGETS		= $(NAME) $(VERSIONED_NAME) $(SNAME) $(VERSIONED_SNAME) $(ADMIN)

OBJS		= IceStorm.o

SERVICE_OBJS	= IceStorm.o \
		  TraceLevels.o \
		  Flusher.o \
		  Subscriber.o \
		  OnewaySubscriber.o \
		  OnewayBatchSubscriber.o \
		  LinkSubscriber.o \
		  SubscriberFactory.o \
		  TopicI.o \
		  TopicManagerI.o \
		  StringBoolDict.o \
		  IdentityLinkDict.o \
		  LinkDB.o \
		  IceStormInternal.o \
		  Service.o

AOBJS		= Admin.o \
		  Grammar.o \
		  Scanner.o \
		  WeightedGraph.o \
		  Parser.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(SERVICE_OBJS:.o=.cpp) \
		  $(SERVER_OBJS:.o=.cpp) \
		  $(AOBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/IceStorm.ice \
		  IceStormInternal.ice \
		  LinkDB.ice

HDIR		= $(includedir)/IceStorm
SDIR		= $(slicedir)/IceStorm

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir IceStorm $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(XERCESC_FLAGS) $(CPPFLAGS)
ICECPPFLAGS	:= -I.. $(ICECPPFLAGS)
SLICE2CPPFLAGS	:= --ice --include-dir IceStorm $(SLICE2CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(OBJS)

$(VERSIONED_SNAME): $(SERVICE_OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(SERVICE_OBJS)

$(ADMIN): $(AOBJS) $(NAME)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(AOBJS) -lIceStorm $(LIBS) $(XERCESC_LIBS) -lreadline -lcurses

StringBoolDict.h StringBoolDict.cpp: $(SLICE2FREEZE)
	rm -f StringBoolDict.h StringBoolDict.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::StringBoolDict,string,bool StringBoolDict

clean::
	rm -f StringBoolDict.h StringBoolDict.cpp

IdentityLinkDict.h IdentityLinkDict.cpp: ../IceStorm/LinkDB.ice $(slicedir)/Ice/Identity.ice $(SLICE2FREEZE)
	rm -f IdentityLinkDict.h IdentityLinkDict.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::IdentityLinkDict,Ice::Identity,IceStorm::LinkDB IdentityLinkDict \
	../IceStorm/LinkDB.ice $(slicedir)/Ice/Identity.ice

clean::
	rm -f IdentityLinkDict.h IdentityLinkDict.cpp

# Needed for make -jn to work.
../IceStorm/Grammar.y: Grammar.h

clean::
	-rm -f Grammar.cpp Grammar.h
	-rm -f Scanner.cpp

install:: all
	$(INSTALL_LIBRARY) $(VERSIONED_NAME) $(install_libdir)
	rm -f $(install_libdir)/$(notdir $(NAME))
	ln -s $(notdir $(VERSIONED_NAME)) $(install_libdir)/$(notdir $(NAME))
	$(INSTALL_LIBRARY) $(VERSIONED_SNAME) $(install_libdir)
	rm -f $(install_libdir)/$(notdir $(SNAME))
	ln -s $(notdir $(VERSIONED_SNAME)) $(install_libdir)/$(notdir $(SNAME))
	$(INSTALL_PROGRAM) $(ADMIN) $(install_bindir)

include .depend
