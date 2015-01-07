# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice Touch is licensed to you under the terms described in the
# ICE_TOUCH_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client

TARGETS		= $(CLIENT)

SLICE_OBJS 	= Library.o \
			  Session.o \
			  Glacier2Session.o

COBJS		= Client.o \
			  Grammar.o \
			  Scanner.o \
			  Parser.o \
		  	  RunParser.o

OBJS		= $(SLICE_OBJS) $(COBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

#
# Flex and bison generated files doesn't support ARC.
#
Grammar.o: Grammar.m
	$(CC) -c $(CPPFLAGS) $(OBJCFLAGS) $(CFLAGS) $<

Scanner.o: Scanner.m
	$(CC) -c $(CPPFLAGS) $(OBJCFLAGS) $(CFLAGS) $<

$(CLIENT): $(SLICE_OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SLICE_OBJS) $(COBJS) -lGlacier2ObjC $(LIBS)

clean::
	-rm -f Grammar.m Grammar.h
	-rm -f Scanner.m
