# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../../..

BASE		= libSlice.so
VERSIONED_BASE	= $(BASE).$(VERSION)

NAME		= $(top_srcdir)/lib/$(BASE)
VERSIONED_NAME	= $(top_srcdir)/lib/$(VERSIONED_BASE)

TARGETS		= $(NAME) $(VERSIONED_NAME)

OBJS		= Shared.o \
		  Scanner.o \
		  Grammer.o \
		  GrammerUtil.o \
		  Parser.o \
		  OutputUtil.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(BASELDFLAGS) -shared -o $@ $(OBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

Grammer.cpp Grammer.h: Grammer.y
	bison -dvt Grammer.y
	rm -f Grammer.cpp ; mv Grammer.tab.c Grammer.cpp
	rm -f Grammer.h ; mv Grammer.tab.h Grammer.h

Scanner.cpp: Scanner.l Grammer.h
	flex Scanner.l
	rm -f Scanner.cpp ; mv lex.yy.c Scanner.cpp

clean::
	-rm -f Grammer.cpp Grammer.h Grammer.output
	-rm -f Scanner.cpp lex.yy.c

Shared.cpp: ../../library/Shared.cpp
	rm -f $@
	ln -s ../../library/Shared.cpp $@

clean::
	-rm -f Shared.cpp

include .depend
