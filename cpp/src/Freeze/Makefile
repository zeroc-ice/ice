# **********************************************************************
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

LIBFILENAME     = $(call mklibfilename,Freeze,$(VERSION))
SONAME          = $(call mksoname,Freeze,$(SOVERSION))  
LIBNAME		= $(call mklibname,Freeze)

TARGETS		= $(libdir)/$(LIBFILENAME) $(libdir)/$(SONAME) $(libdir)/$(LIBNAME)


OBJS		= DBException.o \
		  Evictor.o \
		  ObjectRecord.o \
	  	  MapI.o \
		  EvictorI.o \
                  SharedDbEnv.o \
		  IdentityObjectRecordDict.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/DB.ice \
                  $(SDIR)/DBException.ice \
		  $(SDIR)/Evictor.ice \
		  $(SDIR)/EvictorF.ice \
		  $(SDIR)/ObjectRecord.ice

HDIR		= $(includedir)/Freeze
SDIR		= $(slicedir)/Freeze

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir Freeze $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DFREEZE_API_EXPORTS $(DB_FLAGS) 
SLICE2CPPFLAGS	:= --ice --include-dir Freeze --dll-export FREEZE_API $(SLICE2CPPFLAGS)

$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

../Freeze/IdentityObjectRecordDict.h IdentityObjectRecordDict.cpp: $(SLICE2FREEZE)
	rm -f IdentityObjectRecordDict.h IdentityObjectRecordDict.cpp
	$(SLICE2FREEZECMD) --dict Freeze::IdentityObjectRecordDict,Ice::Identity,Freeze::ObjectRecord \
	    IdentityObjectRecordDict $(slicedir)/Ice/Identity.ice $(slicedir)/Freeze/ObjectRecord.ice

clean::
	rm -f IdentityObjectRecordDict.h IdentityObjectRecordDict.cpp

install:: all
	$(INSTALL_LIBRARY) $(libdir)/$(LIBFILENAME) $(install_libdir)
	rm -f $(install_libdir)/$(SONAME)
	ln -s $(LIBFILENAME) $(install_libdir)/$(SONAME)
	rm -f $(install_libdir)/$(LIBNAME)
	ln -s $(SONAME) $(install_libdir)/$(LIBNAME)

include .depend
