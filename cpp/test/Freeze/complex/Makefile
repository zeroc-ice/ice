# **********************************************************************
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

CPPFLAGS	:= -I. -I../../include $(DB_FLAGS) $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) -lFreeze $(LIBS)

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
