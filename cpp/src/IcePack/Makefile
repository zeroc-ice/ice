# **********************************************************************
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

NAME		= $(top_srcdir)/lib/libIcePack.$(LIBEXT)
VERSIONED_NAME	= $(NAME).$(VERSION)

ADMIN		= $(top_srcdir)/bin/icepackadmin
NODE_SERVER	= $(top_srcdir)/bin/icepacknode
REGISTRY_SERVER	= $(top_srcdir)/bin/icepackregistry

TARGETS		= $(NAME) $(VERSIONED_NAME) $(NODE_SERVER) $(REGISTRY_SERVER) $(ADMIN)

LIB_OBJS	= Admin.o \
		  ExceptionFactory.o

ADMIN_OBJS	= Grammar.o \
		  Scanner.o \
		  Parser.o \
		  Client.o

COMMON_OBJS	= Internal.o \
		  TraceLevels.o \
		  StringObjectProxyDict.o \
		  ComponentBuilder.o

NODE_OBJS	= NodeInfo.o \
		  NodeI.o \
		  ServiceBuilder.o \
		  ServerBuilder.o \
		  ServerI.o \
		  ServerAdapterI.o \
		  ServerFactory.o \
		  ServerDeployerI.o \
	          Activator.o \
		  ActivatorI.o

REGISTRY_OBJS   = Registry.o \
		  AdapterRegistryI.o \
		  ServerRegistryI.o \
		  NodeRegistryI.o \
		  LocatorI.o \
		  LocatorRegistryI.o \
		  ApplicationBuilder.o \
		  AdapterFactory.o \
		  AdapterI.o \
		  AdminI.o

NODE_SVR_OBJS 	= $(COMMON_OBJS) \
		  $(NODE_OBJS) \
		  $(REGISTRY_OBJS) \
		  IcePackNode.o

REGISTRY_SVR_OBJS = \
		  $(COMMON_OBJS) \
		  $(REGISTRY_OBJS) \
		  IcePackRegistry.o

SRCS		= $(LIB_OBJS:.o=.cpp) \
		  $(ADMIN_OBJS:.o=.cpp) \
		  $(COMMON_OBJS:.o=.cpp) \
		  $(NODE_OBJS:.o=.cpp) \
		  $(REGISTRY_OBJS:.o=.cpp) \
		  IcePackNode.cpp \
		  IcePackRegistry.cpp

SLICE_SRCS	= $(SDIR)/Admin.ice \
		  Internal.ice

HDIR		= $(includedir)/IcePack
LOCAL_HDIR	= ../IcePack
SDIR		= $(slicedir)/IcePack

SLICECMD    	= $(SLICE2CPP) --ice --include-dir IcePack --dll-export ICE_PACK_API $(ICECPPFLAGS)
SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir IcePack $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) $(XERCESC_FLAGS)
ICECPPFLAGS	:= -I.. $(ICECPPFLAGS)

$(VERSIONED_NAME): $(LIB_OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(LIB_OBJS) -lIceBox

$(ADMIN): $(ADMIN_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(ADMIN_OBJS) -lYellow -lIcePack -lIceBox $(LIBS) -lreadline -lcurses

$(REGISTRY_SERVER): $(REGISTRY_SVR_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(REGISTRY_SVR_OBJS) \
	-lYellow -lIcePack -lIceBox -lFreeze -lIceXML $(LIBS) $(XERCESC_LIBS) $(DB_LIBS)

$(NODE_SERVER): $(NODE_SVR_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(NODE_SVR_OBJS) \
	-lYellow -lIcePack -lIceBox -lFreeze -lIceXML $(XERCESC_LIBS) $(LIBS) $(DB_LIBS)

Grammar.cpp Grammar.h: Grammar.y
	bison -dvt Grammar.y
	rm -f Grammar.cpp ; mv Grammar.tab.c Grammar.cpp
	rm -f Grammar.h ; mv Grammar.tab.h Grammar.h

Scanner.cpp: Scanner.l Grammar.h
	flex Scanner.l
	rm -f Scanner.cpp
	echo '#include <IceUtil/Config.h>' > Scanner.cpp
	cat lex.yy.c >> Scanner.cpp

clean::
	-rm -f Grammar.cpp Grammar.h Grammar.output
	-rm -f Scanner.cpp lex.yy.c

$(HDIR)/Admin.h Admin.cpp: $(SDIR)/Admin.ice $(SLICE2CPP)
	rm -f $(HDIR)/Admin.h Admin.cpp
	$(SLICECMD) $(SDIR)/Admin.ice
	mv Admin.h $(HDIR)

clean::
	rm -f $(HDIR)/Admin.h Admin.cpp

$(LOCAL_HDIR)/Internal.h Internal.cpp: Internal.ice $(SLICE2CPP)
	rm -f Internal.h Internal.cpp
	$(SLICECMD) Internal.ice

clean::
	rm -f Internal.h Internal.cpp

$(LOCAL_HDIR)/Activator.h Activator.cpp: Activator.ice $(SLICE2CPP)
	rm -f Activator.h Activator.cpp
	$(SLICECMD) -I.. Activator.ice

clean::
	rm -f Activator.h Activator.cpp

$(LOCAL_HDIR)/StringObjectProxyDict.h StringObjectProxyDict.cpp: $(SLICE2FREEZE)
	rm -f StringObjectProxyDict.h StringObjectProxyDict.cpp
	$(SLICE2FREEZECMD) --dict IcePack::StringObjectProxyDict,string,Object* \
	StringObjectProxyDict ../../slice/Ice/BuiltinSequences.ice

clean::
	rm -f StringObjectProxyDict.h StringObjectProxySeq.cpp


include .depend
