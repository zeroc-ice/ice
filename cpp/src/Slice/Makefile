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

top_srcdir	= ../..

NAME		= $(top_srcdir)/lib/libSlice.$(LIBEXT)
VERSIONED_NAME	= $(NAME).$(VERSION)

TARGETS		= $(NAME) $(VERSIONED_NAME)

OBJS		= Scanner.o \
		  Grammar.o \
		  Parser.o \
		  CPlusPlusUtil.o \
		  JavaUtil.o \
		  Preprocessor.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(OBJS)

# Needed for make -jn to work.
../Slice/Grammar.y: Grammar.h

clean::
	rm -f Grammar.cpp Grammar.h
	rm -f Scanner.cpp

install:: all
	$(INSTALL_LIBRARY) $(VERSIONED_NAME) $(install_libdir)
	rm -f $(install_libdir)/$(notdir $(NAME))
	ln -s $(notdir $(VERSIONED_NAME)) $(install_libdir)/$(notdir $(NAME))

include .depend
