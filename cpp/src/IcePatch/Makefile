#**********************************************************************
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

LIBFILENAME     = $(call mklibfilename,IcePatch,$(VERSION))
SONAME          = $(call mksoname,IcePatch,$(SOVERSION))  
LIBNAME		= $(call mklibname,IcePatch)

CLIENT		= $(top_srcdir)/bin/icepatchclient
SERVER		= $(top_srcdir)/bin/icepatchserver

TARGETS		= $(libdir)/$(LIBFILENAME) $(libdir)/$(SONAME) $(libdir)/$(LIBNAME) \
                  $(CLIENT) $(SERVER)

OBJS		= IcePatch.o \
		  FileDescFactory.o \
		  Util.o \
		  ClientUtil.o

COBJS		= Client.o

SOBJS		= Server.o \
		  IcePatchI.o \
		  FileLocator.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/IcePatch.ice

HDIR		= $(includedir)/IcePatch
SDIR		= $(slicedir)/IcePatch

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(OPENSSL_FLAGS) $(BZIP2_FLAGS) $(CPPFLAGS)
SLICE2CPPFLAGS	:= --ice --include-dir IcePatch --dll-export ICE_PATCH_API $(SLICE2CPPFLAGS)

LINKWITH        := $(BZIP2_LIBS)

$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

$(CLIENT): $(COBJS) $(libdir)/$(LIBNAME)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COBJS) -lIcePatch -lGlacier -lIceSSL $(LIBS) $(OPENSSL_LIBS)

$(SERVER): $(SOBJS) $(libdir)/$(LIBNAME)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) -lIcePatch $(LIBS) $(OPENSSL_LIBS)

install:: all
	$(INSTALL_LIBRARY) $(libdir)/$(LIBFILENAME) $(install_libdir)
	rm -f $(install_libdir)/$(SONAME)
	ln -s $(LIBFILENAME) $(install_libdir)/$(SONAME)
	rm -f $(install_libdir)/$(LIBNAME)
	ln -s $(SONAME) $(install_libdir)/$(LIBNAME)
	$(INSTALL_PROGRAM) $(CLIENT) $(install_bindir)
	$(INSTALL_PROGRAM) $(SERVER) $(install_bindir)

include .depend
