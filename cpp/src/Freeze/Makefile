# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,Freeze,$(VERSION))
SONAME          = $(call mksoname,Freeze,$(SOVERSION))
LIBNAME		= $(call mklibname,Freeze)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)$(cpp11libdirsuffix)/$(LIBNAME))

SLICE_OBJS      = BackgroundSaveEvictor.o \
		  CatalogData.o \
		  Connection.o \
		  ConnectionF.o \
		  DB.o \
		  Evictor.o \
		  EvictorF.o \
		  EvictorStorage.o \
		  Exception.o \
		  Transaction.o \
		  TransactionalEvictor.o \
		  PingObject.o

OBJS	        = BackgroundSaveEvictorI.o \
		  Catalog.o \
		  CatalogIndexList.o \
		  ConnectionI.o \
		  EvictorI.o \
		  EvictorIteratorI.o \
		  Index.o \
		  IndexI.o \
		  MapDb.o \
		  MapI.o \
		  ObjectStore.o \
		  SharedDbEnv.o \
		  TransactionalEvictorContext.o \
		  TransactionalEvictorI.o \
		  TransactionHolder.o \
		  TransactionI.o \
		  Util.o \
                  $(SLICE_OBJS)

HDIR		= $(headerdir)/Freeze
SDIR		= $(slicedir)/Freeze

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DFREEZE_API_EXPORTS $(DB_FLAGS)
SLICE2CPPFLAGS	:= --ice --include-dir Freeze --dll-export FREEZE_API $(SLICE2CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIce -lIceUtil $(DB_RPATH_LINK) $(DB_LIBS)

# The slice2freeze rules are structured like this to avoid issues with
# parallel make.
$(HDIR)/Catalog.h: Catalog.cpp
Catalog.cpp: $(SDIR)/CatalogData.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f  $(HDIR)/Catalog.h Catalog.cpp
	$(SLICE2FREEZE) $(SLICE2CPPFLAGS) --dict Freeze::Catalog,string,Freeze::CatalogData \
	Catalog $(slicedir)/Freeze/CatalogData.ice
	mv Catalog.h $(HDIR)/Catalog.h

clean::
	-rm -f $(HDIR)/Catalog.h Catalog.cpp

$(HDIR)/CatalogIndexList.h: CatalogIndexList.cpp
CatalogIndexList.cpp: $(slicedir)/Ice/BuiltinSequences.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f  $(HDIR)/CatalogIndexList.h CatalogIndexList.cpp
	$(SLICE2FREEZE) $(SLICE2CPPFLAGS) --dict Freeze::CatalogIndexList,string,Ice::StringSeq \
	CatalogIndexList ../../../slice/Ice/BuiltinSequences.ice
	mv CatalogIndexList.h $(HDIR)/CatalogIndexList.h

clean::
	-rm -f $(HDIR)/CatalogIndexList.h CatalogIndexList.cpp

$(libdir)/$(LIBFILENAME): $(OBJS)
	@mkdir -p $(dir $@)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)$(cpp11libdirsuffix)/$(LIBNAME): $(libdir)/$(SONAME)
	@mkdir -p $(libdir)$(cpp11libdirsuffix)
	rm -f $@
	ln -s $(cpp11sonamedir)$(SONAME) $@

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
