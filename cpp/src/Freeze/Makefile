# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,Freeze,$(VERSION))
SONAME          = $(call mksoname,Freeze,$(SOVERSION))  
LIBNAME		= $(call mklibname,Freeze)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))


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
                  TransactionHolder.o \
                  Catalog.o \
                  CatalogData.o \

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/DB.ice \
                  $(SDIR)/Exception.ice \
                  $(SDIR)/Connection.ice \
                  $(SDIR)/ConnectionF.ice \
                  $(SDIR)/Transaction.ice \
		  $(SDIR)/Evictor.ice \
		  $(SDIR)/EvictorF.ice \
		  $(SDIR)/EvictorStorage.ice \
                  $(SDIR)/CatalogData.ice

HDIR		= $(includedir)/Freeze
SDIR		= $(slicedir)/Freeze

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DFREEZE_API_EXPORTS $(DB_FLAGS) 
SLICE2CPPFLAGS	:= --ice --include-dir Freeze --dll-export FREEZE_API $(SLICE2CPPFLAGS)
LINKWITH	:= -lIce -lIceUtil $(DB_LIBS)


$(HDIR)/Catalog.h Catalog.cpp: $(SLICE2FREEZE) $(SDIR)/CatalogData.ice
	rm -f  $(HDIR)/Catalog.h Catalog.cpp
	$(SLICE2FREEZE) $(SLICE2CPPFLAGS) --dict Freeze::Catalog,string,Freeze::CatalogData \
	Catalog ../../slice/Freeze/CatalogData.ice
	mv Catalog.h $(HDIR)/Catalog.h

clean::
	rm -f  $(HDIR)/Catalog.h Catalog.cpp


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
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))

include .depend
