# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= certificateVerifier

TARGETS		= $(CLIENT)

OBJS		= CertificateVerifier.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(OPENSSL_FLAGS) $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(OPENSSL_RPATH_LINK) -lIceSSL $(EXPAT_RPATH_LINK) -lIceXML $(LIBS) $(OPENSSL_LIBS)

include .depend
