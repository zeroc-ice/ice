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

READER		= reader
WRITER		= writer

TARGETS		= $(READER) $(WRITER)

OBJS		= Pickle.o \

ROBJS		= Reader.o \
		  ServantFactory.o

WOBJS		= Writer.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(ROBJS:.o=.cpp) \
		  $(WOBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

$(READER): $(OBJS) $(ROBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(ROBJS) $(LIBS)

$(WRITER): $(OBJS) $(WOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(WOBJS) $(LIBS)

Pickle.h Pickle.cpp: Pickle.ice $(SLICE)
	rm -f Pickle.h Pickle.cpp
	$(SLICE) Pickle.ice

clean::
	rm -f Pickle.h Pickle.cpp

include .depend
