# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME     	= $(top_srcdir)\lib\freeze$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\freeze$(SOVERSION)$(LIBSUFFIX).dll


TARGETS		= $(LIBNAME) $(DLLNAME)


OBJS		= CatalogData.o \
                  Catalog.o \
                  ConnectionI.o \
                  Connection.o \
		  DB.o \
		  EvictorI.o \
                  EvictorIteratorI.o \
		  Evictor.o \
		  EvictorStorage.o \
                  Exception.o \
                  IndexI.o \
                  Index.o \
	  	  MapI.o \
                  ObjectStore.o \
		  PingObject.o \
                  SharedDbEnv.o \
                  SharedDb.o \
                  TransactionHolder.o \
                  TransactionI.o \
                  Transaction.o \
                  Util.o

SRCS		= $(OBJS:.o=.cpp)


HDIR		= $(includedir)\Freeze
SDIR		= $(slicedir)\Freeze

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I.. $(CPPFLAGS) -DFREEZE_API_EXPORTS
SLICE2CPPFLAGS	= --ice --include-dir Freeze --dll-export FREEZE_API $(SLICE2CPPFLAGS)
LINKWITH	= $(LIBS) $(DB_LIBS)

$(HDIR)/Catalog.h Catalog.cpp: $(SLICE2FREEZE) $(SDIR)/CatalogData.ice
	del /q $(HDIR)\Catalog.h Catalog.cpp
	$(SLICE2FREEZE) $(SLICE2CPPFLAGS) --dict Freeze::Catalog,string,Freeze::CatalogData \
	Catalog ../../slice/Freeze/CatalogData.ice
	move Catalog.h $(HDIR)

clean::
	del /q $(HDIR)\Catalog.h Catalog.cpp


$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	del /q $@
	$(LINK) $(LD_DLLFLAGS) $(OBJS), $(DLLNAME),, $(LINKWITH)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)

DB.cpp $(HDIR)/DB.h: $(SDIR)/DB.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)/DB.ice
	move DB.h $(HDIR)

CatalogData.cpp $(HDIR)/CatalogData.h: $(SDIR)/CatalogData.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)/CatalogData.ice
	move CatalogData.h $(HDIR)

Connection.cpp $(HDIR)/Connection.h: $(SDIR)/Connection.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)/Connection.ice
	move Connection.h $(HDIR)

$(HDIR)/ConnectionF.h: $(SDIR)/ConnectionF.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)/ConnectionF.ice
	del /q ConnectionF.cpp
	move ConnectionF.h $(HDIR)

Exception.cpp $(HDIR)/Exception.h: $(SDIR)/Exception.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)/Exception.ice
	move Exception.h $(HDIR)

$(HDIR)/EvictorF.h: $(SDIR)/EvictorF.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)/EvictorF.ice
	del /q EvictorF.cpp
	move EvictorF.h $(HDIR)

Evictor.cpp $(HDIR)/Evictor.h: $(SDIR)/Evictor.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)/Evictor.ice
	move Evictor.h $(HDIR)

EvictorStorage.cpp $(HDIR)/EvictorStorage.h: $(SDIR)/EvictorStorage.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)/EvictorStorage.ice
	move EvictorStorage.h $(HDIR)

Transaction.cpp $(HDIR)/Transaction.h: $(SDIR)/Transaction.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)/Transaction.ice
	move Transaction.h $(HDIR)

PingObject.cpp ../Freeze/PingObject.h: PingObject.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) PingObject.ice

clean::
	del /q DB.cpp $(HDIR)\DB.h
	del /q CatalogData.cpp $(HDIR)\CatalogData.h
	del /q Connection.cpp $(HDIR)\Connection.h
	del /q $(HDIR)\ConnectionF.h
	del /q Exception.cpp $(HDIR)\Exception.h
	del /q $(HDIR)\EvictorF.h
	del /q Evictor.cpp $(HDIR)\Evictor.h
	del /q EvictorStorage.cpp $(HDIR)\EvictorStorage.h
	del /q Transaction.cpp $(HDIR)\Transaction.h
	del /q PingObject.cpp PingObject.h

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)

!include .depend
