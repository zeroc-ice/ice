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

COBJS		= Grammar.o \
		  Scanner.o \
		  Parser.o \
		  Client.o

SOBJS		= Server.o \
		  Forward.o \
		  AdminI.o \
		  Activator.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

HDIR		= $(includedir)/IcePack
SDIR		= $(slicedir)/IcePack
SLICECMD	= $(SLICE2CPP) --include-dir IcePack --dll-export ICE_PACK_API -I$(slicedir)

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
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(COBJS) -lIcePack $(LIBS) -lreadline -lcurses

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOBJS) -lIcePack $(LIBS)

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

include .depend
