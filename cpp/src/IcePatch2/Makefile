# **********************************************************************
#
# Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

SERVER		= $(top_srcdir)/bin/icepatch2server
CLIENT		= $(top_srcdir)/bin/icepatch2client
CALC		= $(top_srcdir)/bin/icepatch2calc

TARGETS         = $(SERVER) $(CLIENT) $(CALC)

SOBJS		= Server.o \
		  FileServerI.o

COBJS		= Client.o

CALCOBJS	= Calc.o

SRCS		= $(SOBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(CALCOBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I.. $(CPPFLAGS) $(OPENSSL_FLAGS) $(BZIP2_FLAGS)

$(SERVER): $(SOBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) $(OPENSSL_RPATH_LINK) -lIcePatch2 $(LIBS)

$(CLIENT): $(COBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COBJS) $(OPENSSL_RPATH_LINK) -lIcePatch2 $(LIBS)

$(CALC): $(CALCOBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(CALCOBJS) $(OPENSSL_RPATH_LINK) -lIcePatch2 $(LIBS)

install:: all
	$(call installprogram,$(SERVER),$(install_bindir))
	$(call installprogram,$(CLIENT),$(install_bindir))
	$(call installprogram,$(CALC),$(install_bindir))

include .depend
