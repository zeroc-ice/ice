# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,Glacier,$(VERSION))
SONAME          = $(call mksoname,Glacier,$(SOVERSION))  
LIBNAME		= $(call mklibname,Glacier)

ROUTER		= $(top_srcdir)/bin/glacierrouter
STARTER		= $(top_srcdir)/bin/glacierstarter

LIBTARGETS      = $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))
TARGETS         = $(LIBTARGETS) $(ROUTER) $(STARTER)

OBJS		= Router.o \
		  SessionF.o \
		  Session.o \
		  SessionManagerF.o \
		  SessionManager.o \
		  Starter.o

ROBJS		= GlacierRouter.o \
		  RouterI.o \
		  Blobject.o \
		  ClientBlobject.o \
		  ServerBlobject.o \
		  Request.o

SOBJS		= GlacierStarter.o \
		  StarterI.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(ROBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/Starter.ice \
		  $(SDIR)/Router.ice \
		  $(SDIR)/Session.ice \
		  $(SDIR)/SessionManager.ice \
		  $(SDIR)/SessionF.ice \
		  $(SDIR)/SessionManagerF.ice

HDIR		= $(includedir)/Glacier
SDIR		= $(slicedir)/Glacier

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DGLACIER_API_EXPORTS $(OPENSSL_FLAGS) 
SLICE2CPPFLAGS	:= --checksum --include-dir Glacier --dll-export GLACIER_API $(SLICE2CPPFLAGS)
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
	$(CXX) $(LDFLAGS) -o $@ $(ROBJS) -lGlacier -lIceSSL $(LIBS) $(OPENSSL_LIBS) $(EXPAT_LIBS)

$(STARTER): $(SOBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) -lGlacier -lIceSSL $(LIBS) $(OPENSSL_LIBS) $(EXPAT_LIBS)

install:: all
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
	$(INSTALL_PROGRAM) $(ROUTER) $(install_bindir)
	$(INSTALL_PROGRAM) $(STARTER) $(install_bindir)

include .depend
