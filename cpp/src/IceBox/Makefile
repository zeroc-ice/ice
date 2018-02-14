# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME	= $(call mklibfilename,IceBox,$(VERSION))
SONAME    	= $(call mksoname,IceBox,$(SOVERSION))
LIBNAME		= $(call mklibname,IceBox)

SERVER		= $(bindir)/icebox$(binsuffix)
ADMIN		= $(bindir)/iceboxadmin

LIBTARGETS 	= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)$(cpp11libdirsuffix)/$(LIBNAME))
TARGETS   	= $(LIBTARGETS) $(SERVER) $(ADMIN)

SLICE_OBJS      = IceBox.o

LIB_OBJS	= Exception.o \
		  $(SLICE_OBJS)

SOBJS		= Service.o \
		  ServiceManagerI.o

AOBJS		= Admin.o

OBJS		= $(LIB_OBJS) \
		  $(SOBJS) \
		  $(AOBJS)

HDIR		= $(headerdir)/IceBox
SDIR		= $(slicedir)/IceBox

RPATH_DIR	= $(LOADER_PATH)/../$(libsubdir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS)
SLICE2CPPFLAGS	:= --checksum --ice --dll-export ICE_BOX_API --include-dir IceBox $(SLICE2CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIce -lIceUtil

$(libdir)/$(LIBFILENAME): $(LIB_OBJS)
	@mkdir -p $(dir $@)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(LIB_OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)$(cpp11libdirsuffix)/$(LIBNAME): $(libdir)/$(SONAME)
	@mkdir -p $(libdir)$(cpp11libdirsuffix)
	rm -f $@
	ln -s $(cpp11sonamedir)$(SONAME) $@

$(SERVER): $(SOBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(SOBJS) -lIceBox $(LIBS)

$(ADMIN): $(AOBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(AOBJS) -lIceBox $(LIBS)

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
	$(call installprogram,$(SERVER),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/icebox.1,$(DESTDIR)$(install_mandir))
	$(call installprogram,$(ADMIN),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/iceboxadmin.1,$(DESTDIR)$(install_mandir))
