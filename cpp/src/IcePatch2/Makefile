# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IcePatch2,$(VERSION))
SONAME          = $(call mksoname,IcePatch2,$(SOVERSION))  
LIBNAME		= $(call mklibname,IcePatch2)

SERVER		= $(top_srcdir)/bin/icepatch2server
CLIENT		= $(top_srcdir)/bin/icepatch2client
CALC		= $(top_srcdir)/bin/icepatch2calc

LIBTARGETS      = $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))
TARGETS         = $(LIBTARGETS) $(SERVER) $(CLIENT) $(CALC)

OBJS		= Util.o \
		  ClientUtil.o \
		  FileInfo.o \
		  FileServer.o \
		  OS.o

SOBJS		= Server.o \
		  FileServerI.o

COBJS		= Client.o

CALCOBJS	= Calc.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(CALCOBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/FileInfo.ice \
		  $(SDIR)/FileServer.ice

HDIR		= $(includedir)/IcePatch2
SDIR		= $(slicedir)/IcePatch2

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I.. $(CPPFLAGS) -DICE_PATCH2_API_EXPORTS $(OPENSSL_FLAGS) $(BZIP2_FLAGS)
SLICE2CPPFLAGS	:= --ice --include-dir IcePatch2 --dll-export ICE_PATCH2_API $(SLICE2CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIce -lIceUtil $(OPENSSL_LIBS) $(BZIP2_LIBS)

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
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) $(OPENSSL_RPATH_LINK) -lIcePatch2 $(LIBS)

$(CLIENT): $(COBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COBJS) $(OPENSSL_RPATH_LINK) -lIcePatch2 $(LIBS)

$(CALC): $(CALCOBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(CALCOBJS) $(OPENSSL_RPATH_LINK) -lIcePatch2 $(LIBS)

install:: all
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
	$(call installprogram,$(SERVER),$(install_bindir))
	$(call installprogram,$(CLIENT),$(install_bindir))
	$(call installprogram,$(CALC),$(install_bindir))

include .depend
