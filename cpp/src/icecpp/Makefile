# **********************************************************************
#
# Copyright (c) 2001
# ZeroC, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../..

NAME		= $(top_srcdir)/bin/icecpp

TARGETS		= $(NAME)

OBJS		= cccp.o \
		  cexp.o \
		  prefix.o

include $(top_srcdir)/config/Make.rules

cccp.o: cccp.c
	$(CC) -I. -c $<

cexp.o: cexp.c
	$(CC) -I. -c $<

prefix.o: prefix.c
	$(CC) -I. -DPREFIX=\"\" -c $<

cexp.c: cexp.y
	bison -o cexp.c cexp.y

$(NAME): $(OBJS)
	rm -f $@
	$(CC) -o $@ $(OBJS)
