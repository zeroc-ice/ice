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

BASE		= libSlice.so
VERSIONED_BASE	= $(BASE).$(VERSION)

NAME		= $(top_srcdir)/lib/$(BASE)
VERSIONED_NAME	= $(top_srcdir)/lib/$(VERSIONED_BASE)

TARGETS		= $(NAME) $(VERSIONED_NAME)

OBJS		= Scanner.o \
		  Grammar.o \
		  Parser.o \
		  CPlusPlusUtil.o \
		  XsdVisitor.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DYYDEBUG=1

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(BASELDFLAGS) -shared -o $@ $(OBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

Grammar.cpp Grammar.h: Grammar.y
	bison -dvt Grammar.y
	rm -f Grammar.cpp ; mv Grammar.tab.c Grammar.cpp
	rm -f Grammar.h ; mv Grammar.tab.h Grammar.h

Scanner.cpp: Scanner.l Grammar.h
	flex Scanner.l
	rm -f Scanner.cpp ; mv lex.yy.c Scanner.cpp

clean::
	-rm -f Grammar.cpp Grammar.h Grammar.output
	-rm -f Scanner.cpp lex.yy.c

include .depend
