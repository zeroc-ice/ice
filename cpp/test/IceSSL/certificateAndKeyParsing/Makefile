# **********************************************************************
#
# Copyright (c) 2002
# Mutable Realms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= certificateAndKeyParsing

TARGETS		= $(CLIENT)

OBJS		= CertificateAndKeyParsing.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(SSLLIBS) $(LIBS)

include .depend
