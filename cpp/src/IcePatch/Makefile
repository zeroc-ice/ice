# **********************************************************************
#
# Copyright (c) 2003 - 2004
# ZeroC, Inc.
# North Palm Beach, FL, USA
#
# All Rights Reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************


top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IcePatch,$(VERSION))
SONAME          = $(call mksoname,IcePatch,$(SOVERSION))  
LIBNAME		= $(call mklibname,IcePatch)

CLIENT		= $(top_srcdir)/bin/icepatchclient
CALC		= $(top_srcdir)/bin/icepatchcalc
SERVER		= $(top_srcdir)/bin/icepatchserver

TARGETS		= $(libdir)/$(LIBFILENAME) $(libdir)/$(SONAME) $(libdir)/$(LIBNAME) \
                  $(CLIENT) $(CALC) $(SERVER)

OBJS		= IcePatch.o \
		  FileDescFactory.o \
		  Util.o \
		  ClientUtil.o

CLIENT_OBJS	= Client.o

CALC_OBJS	= Calc.o

SERVER_OBJS	= Server.o \
		  IcePatchI.o \
		  FileLocator.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(CLIENT_OBJS:.o=.cpp) \
		  $(CALC_OBJS:.o=.cpp) \
		  $(SERVER_OBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/IcePatch.ice

HDIR		= $(includedir)/IcePatch
SDIR		= $(slicedir)/IcePatch

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_PATCH_API_EXPORTS $(OPENSSL_FLAGS) $(BZIP2_FLAGS)
SLICE2CPPFLAGS	:= --ice --include-dir IcePatch --dll-export ICE_PATCH_API $(SLICE2CPPFLAGS)

LINKWITH        := -lIce -lIceUtil $(OPENSSL_LIBS) $(BZIP2_LIBS)

$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

$(CLIENT): $(CLIENT_OBJS) $(libdir)/$(LIBNAME)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(CLIENT_OBJS) -lIcePatch -lGlacier -lIceSSL $(LIBS) $(OPENSSL_LIBS)

$(CALC): $(CALC_OBJS) $(libdir)/$(LIBNAME)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(CALC_OBJS) -lIcePatch $(LIBS)

$(SERVER): $(SERVER_OBJS) $(libdir)/$(LIBNAME)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SERVER_OBJS) -lIcePatch $(LIBS)

install:: all
	$(INSTALL_LIBRARY) $(libdir)/$(LIBFILENAME) $(install_libdir)
	rm -f $(install_libdir)/$(SONAME)
	ln -s $(LIBFILENAME) $(install_libdir)/$(SONAME)
	rm -f $(install_libdir)/$(LIBNAME)
	ln -s $(SONAME) $(install_libdir)/$(LIBNAME)
	$(INSTALL_PROGRAM) $(CLIENT) $(install_bindir)
	$(INSTALL_PROGRAM) $(CALC) $(install_bindir)
	$(INSTALL_PROGRAM) $(SERVER) $(install_bindir)

include .depend
