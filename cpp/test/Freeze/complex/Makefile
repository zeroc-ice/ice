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

CLIENT		= client
POPULATE	= populate

TARGETS		= $(CLIENT) \
		  $(POPULATE)

OBJS		= ComplexDict.o \
		  Complex.o

COBJS		= Client.o

POBJS		= Grammar.o \
		  Scanner.o \
		  Parser.o \
		  Populate.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(POJBS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS) -I$(DB)/include
LDFLAGS		:= $(LDFLAGS) -L$(DB)/lib

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS) -lFreeze -ldb -lreadline -lcurses

$(POPULATE): $(OBJS) $(POBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(POBJS) $(LIBS) -lFreeze -ldb -lreadline -lcurses

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

Complex.h Complex.cpp: Complex.ice $(SLICE2CPP)
	rm -f Complex.h Complex.cpp
	$(SLICE2CPP) -I$(slicedir) Complex.ice

clean::
	rm -f Complex.h Complex.cpp

ComplexDict.h ComplexDict.cpp: Complex.ice $(SLICE2FREEZE)
	rm -f ComplexDict.h ComplexDict.cpp
	$(SLICE2FREEZE) -I$(slicedir) --dict Complex::ComplexDict,Complex::Key,Complex::Node ComplexDict Complex.ice

clean::
	rm -f ComplexDict.h ComplexDict.cpp


include .depend
