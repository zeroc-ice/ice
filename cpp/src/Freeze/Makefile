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


OBJS		= DB.o \
                  Exception.o \
		  Evictor.o \
		  EvictorF.o \
		  EvictorStorage.o \
                  Connection.o \
                  ConnectionF.o \
                  ConnectionI.o \
                  Transaction.o \
                  TransactionI.o \
                  SharedDb.o \
	  	  MapI.o \
                  Util.o \
                  ObjectStore.o \
		  EvictorI.o \
                  EvictorIteratorI.o \
                  Index.o \
                  IndexI.o \
                  SharedDbEnv.o \
                  TransactionHolder.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/DB.ice \
                  $(SDIR)/Exception.ice \
                  $(SDIR)/Connection.ice \
                  $(SDIR)/ConnectionF.ice \
                  $(SDIR)/Transaction.ice \
		  $(SDIR)/Evictor.ice \
		  $(SDIR)/EvictorF.ice \
		  $(SDIR)/EvictorStorage.ice

HDIR		= $(includedir)/Freeze
SDIR		= $(slicedir)/Freeze

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DFREEZE_API_EXPORTS $(DB_FLAGS) 
SLICE2CPPFLAGS	:= --ice --include-dir Freeze --dll-export FREEZE_API $(SLICE2CPPFLAGS)
SLICE2FREEZECMD := $(SLICE2FREEZE) --ice --include-dir Freeze $(ICECPPFLAGS)
LINKWITH	:= -lIce -lIceUtil $(DB_LIBS)

$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

install:: all
	$(INSTALL_LIBRARY) $(libdir)/$(LIBFILENAME) $(install_libdir)
	rm -f $(install_libdir)/$(SONAME)
	ln -s $(LIBFILENAME) $(install_libdir)/$(SONAME)
	rm -f $(install_libdir)/$(LIBNAME)
	ln -s $(SONAME) $(install_libdir)/$(LIBNAME)

include .depend
