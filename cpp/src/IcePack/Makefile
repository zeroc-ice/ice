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

CLIENT		= $(top_srcdir)/bin/icepackadmin
SERVER		= $(top_srcdir)/bin/icepack

TARGETS		= $(NAME) $(VERSIONED_NAME) $(CLIENT) $(SERVER)

OBJS		= Admin.o \
		  ServerManager.o \
		  AdapterManager.o \
		  Activator.o

COBJS		= Grammar.o \
		  Scanner.o \
		  Parser.o \
		  Client.o

SOBJS		= Server.o \
		  AdapterManagerI.o \
		  ServerManagerI.o \
		  ComponentDeployer.o \
		  ServerDeployer.o \
		  ServiceDeployer.o \
		  LocatorI.o \
		  LocatorRegistryI.o \
		  AdminI.o \
		  ActivatorI.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

HDIR		= $(includedir)/IcePack
LOCAL_HDIR	= ../IcePack
SDIR		= $(slicedir)/IcePack
SLICECMD	= $(SLICE2CPP) --include-dir IcePack --dll-export ICE_PACK_API -I$(slicedir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) $(XERCESC_FLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) -Xlinker -soname -Xlinker $(VERSIONED_BASE) $(LDFLAGS) -shared -o $@ $(OBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(COBJS) -lYellow -lIcePack $(LIBS) -lreadline -lcurses

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOBJS) -lYellow -lIcePack -lFreeze $(XERCESC_LIBS) $(LIBS) -ldb

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

$(HDIR)/AdminF.h: $(SDIR)/AdminF.ice $(SLICE2CPP)
	rm -f $(HDIR)/AdminF.h AdminF.cpp
	$(SLICECMD) $(SDIR)/AdminF.ice
	mv AdminF.h $(HDIR)
	rm -f AdminF.cpp

clean::
	rm -f $(HDIR)/AdminF.h

$(LOCAL_HDIR)/AdapterManager.h AdapterManager.cpp: AdapterManager.ice $(SLICE2CPP)
	rm -f AdapterManager.h AdapterManager.cpp
	$(SLICECMD) -I.. AdapterManager.ice

clean::
	rm -f AdapterManager.h AdapterManager.cpp

$(LOCAL_HDIR)/AdapterManagerF.h: AdapterManagerF.ice $(SLICE2CPP)
	rm -f AdapterManagerF.h AdapterManagerF.cpp
	$(SLICECMD) -I.. AdapterManagerF.ice
	rm -f AdapterManagerF.cpp

clean::
	rm -f AdapterManagerF.h

$(LOCAL_HDIR)/ServerManager.h ServerManager.cpp: ServerManager.ice $(SLICE2CPP)
	rm -f ServerManager.h ServerManager.cpp
	$(SLICECMD) -I.. ServerManager.ice

clean::
	rm -f ServerManager.h ServerManager.cpp

$(LOCAL_HDIR)/ServerManagerF.h: ServerManagerF.ice $(SLICE2CPP)
	rm -f ServerManagerF.h ServerManagerF.cpp
	$(SLICECMD) -I.. ServerManagerF.ice
	rm -f ServerManagerF.cpp

clean::
	rm -f ServerManagerF.h

$(LOCAL_HDIR)/Activator.h Activator.cpp: Activator.ice $(SLICE2CPP)
	rm -f Activator.h Activator.cpp
	$(SLICECMD) -I.. Activator.ice

clean::
	rm -f Activator.h Activator.cpp

include .depend
