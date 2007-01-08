# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceBox,$(VERSION))
SONAME          = $(call mksoname,IceBox,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceBox)

SERVER		= $(top_srcdir)/bin/icebox
ADMIN		= $(top_srcdir)/bin/iceboxadmin

LIBTARGETS      = $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))
TARGETS         = $(LIBTARGETS) $(SERVER) $(ADMIN)

OBJS		= IceBox.o \
		  Exception.o

SOBJS		= ServiceManagerI.o \
		  Service.o

AOBJS		= Admin.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(AOBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/IceBox.ice

HDIR		= $(includedir)/IceBox
SDIR		= $(slicedir)/IceBox


include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_BOX_API_EXPORTS 
SLICE2CPPFLAGS	:= --checksum --ice --dll-export ICE_BOX_API --include-dir IceBox $(SLICE2CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIce -lIceUtil

$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

$(SERVER): $(SOBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) -lIceBox $(LIBS)

$(ADMIN): $(AOBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(AOBJS) -lIceBox $(LIBS)

install:: all
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
	$(call installprogram,$(SERVER),$(install_bindir))
	$(call installprogram,$(ADMIN),$(install_bindir))

include .depend
