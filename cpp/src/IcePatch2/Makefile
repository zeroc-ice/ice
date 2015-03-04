# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

SERVER		= $(bindir)/icepatch2server
CLIENT		= $(bindir)/icepatch2client
CALC		= $(bindir)/icepatch2calc

TARGETS         = $(SERVER) $(CLIENT) $(CALC)

SOBJS		= Server.o \
		  FileServerI.o

COBJS		= Client.o

CALCOBJS	= Calc.o

SRCS		= $(SOBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(CALCOBJS:.o=.cpp)

ifeq ($(CPP11),yes)
RPATH_DIR	= @loader_path/../../lib/c++11
else
RPATH_DIR	= @loader_path/../lib
endif

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
	$(call installprogram,$(SERVER),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/icepatch2server.1,$(DESTDIR)$(install_mandir))
	$(call installprogram,$(CLIENT),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/icepatch2client.1,$(DESTDIR)$(install_mandir))
	$(call installprogram,$(CALC),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/icepatch2calc.1,$(DESTDIR)$(install_mandir))

include .depend
