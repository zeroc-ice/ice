# **********************************************************************
#
# Copyright (c) 2001
# Mutable Realms, Inc.
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

HDIR		= $(includedir)/IcePack
LOCAL_HDIR	= ../IcePack
SDIR		= $(slicedir)/IcePack
SLICECMD	= $(SLICE2CPP) --ice --include-dir IcePack --dll-export ICE_PACK_API -I$(slicedir)
SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir IcePack -I$(slicedir) -I..
include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) $(XERCESC_FLAGS)
LDFLAGS		:= $(LDFLAGS) -L$(DB_HOME)/lib

$(VERSIONED_NAME): $(LIB_OBJS)
	rm -f $@
	$(CXX) -Xlinker -soname -Xlinker $(VERSIONED_BASE) $(LDFLAGS) -shared -o $@ $(LIB_OBJS) -lIceBox

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

$(ADMIN): $(ADMIN_OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(ADMIN_OBJS) -lYellow -lIcePack -lIceBox $(LIBS) -lreadline -lcurses

$(REGISTRY_SERVER): $(REGISTRY_SVR_OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(REGISTRY_SVR_OBJS) \
	-lYellow -lIcePack -lIceBox -lFreeze $(XERCESC_LIBS) $(LIBS) -ldb

$(NODE_SERVER): $(NODE_SVR_OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(NODE_SVR_OBJS) \
	-lYellow -lIcePack -lIceBox -lFreeze $(XERCESC_LIBS) $(LIBS) -ldb

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
	$(SLICECMD) -I.. Internal.ice

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
