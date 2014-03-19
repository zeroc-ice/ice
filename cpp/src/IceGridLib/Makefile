# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceGrid,$(VERSION))
SONAME          = $(call mksoname,IceGrid,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceGrid)

LIBTARGETS	= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))
TARGETS         = $(LIBTARGETS) $(NODE_SERVER) $(REGISTRY_SERVER) $(ADMIN)

LIB_OBJS	= Admin.o \
		  Locator.o \
		  Query.o \
	 	  Exception.o \
		  Descriptor.o \
                  FileParser.o \
		  Observer.o \
		  Session.o \
		  Registry.o \
		  UserAccountMapper.o

SRCS		= $(LIB_OBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/Admin.ice \
                  $(SDIR)/Exception.ice \
                  $(SDIR)/FileParser.ice \
                  $(SDIR)/Locator.ice \
                  $(SDIR)/Query.ice \
		  $(SDIR)/Registry.ice \
		  $(SDIR)/Session.ice \
		  $(SDIR)/Observer.ice \
		  $(SDIR)/Descriptor.ice \
		  $(SDIR)/UserAccountMapper.ice \

HDIR		= $(headerdir)/IceGrid
LOCAL_HDIR	= ../IceGrid
SDIR		= $(slicedir)/IceGrid
LOCAL_SDIR	= ../IceGrid

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -I.. $(OPENSSL_FLAGS) $(READLINE_FLAGS)
ICECPPFLAGS	:= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS 	:= --checksum --ice --include-dir IceGrid --dll-export ICE_GRID_API $(SLICE2CPPFLAGS)
LINKWITH        := -lGlacier2 $(BZIP2_RPATH_LINK) -lIce -lIceUtil

$(libdir)/$(LIBFILENAME): $(LIB_OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(LIB_OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))

include .depend
