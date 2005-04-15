# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,Glacier2,$(VERSION))
SONAME          = $(call mksoname,Glacier2,$(SOVERSION))  
LIBNAME		= $(call mklibname,Glacier2)

ROUTER		= $(top_srcdir)/bin/glacier2router

LIBTARGETS      = $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))
TARGETS         = $(LIBTARGETS) $(ROUTER)

OBJS		= PermissionsVerifierF.o \
		  PermissionsVerifier.o \
		  RouterF.o \
		  Router.o \
		  SessionF.o \
		  Session.o

ROBJS		= Blobject.o \
		  ClientBlobject.o \
		  CryptPermissionsVerifierI.o \
		  Glacier2Router.o \
		  RequestQueue.o \
		  RouterI.o \
		  ServerBlobject.o \
		  SessionRouterI.o


SRCS		= $(OBJS:.o=.cpp) \
		  $(ROBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/PermissionsVerifierF.ice \
		  $(SDIR)/PermissionsVerifier.ice \
		  $(SDIR)/Router.ice \
		  $(SDIR)/RouterF.ice \
		  $(SDIR)/Session.ice \
		  $(SDIR)/SessionF.ice

HDIR		= $(includedir)/Glacier2
SDIR		= $(slicedir)/Glacier2

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DGLACIER2_API_EXPORTS $(OPENSSL_FLAGS)
SLICE2CPPFLAGS	:= --include-dir Glacier2 --dll-export GLACIER2_API $(SLICE2CPPFLAGS)
LINKWITH	:= -lIce -lIceUtil

$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

$(ROUTER): $(ROBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(ROBJS) -lGlacier2 $(LIBS) $(OPENSSL_LIBS) $(EXPAT_LIBS)

install:: all
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
	$(INSTALL_PROGRAM) $(ROUTER) $(install_bindir)

include .depend
