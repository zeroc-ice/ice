# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client

TARGETS		= $(CLIENT)

OBJS		= ComplexDict.o \
		  Complex.o \
		  Grammar.o \
		  Scanner.o \
		  Parser.o \
		  Client.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= Complex.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

ComplexDict.h ComplexDict.cpp: Complex.ice $(SLICE2FREEZE)
	rm -f ComplexDict.h ComplexDict.cpp
	$(SLICE2FREEZE) -I$(slicedir) --dict Complex::ComplexDict,Complex::Key,Complex::Node ComplexDict Complex.ice

clean::
	rm -f ComplexDict.h ComplexDict.cpp

clean::
	rm -f Grammar.cpp Grammar.h
	rm -f Scanner.cpp

clean::
	rm -f db/test db/log.*

include .depend
