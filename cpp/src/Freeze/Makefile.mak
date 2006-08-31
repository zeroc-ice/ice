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


OBJS		= CatalogData.obj \
                  Catalog.obj \
                  ConnectionI.obj \
                  Connection.obj \
		  DB.obj \
		  EvictorI.obj \
                  EvictorIteratorI.obj \
		  Evictor.obj \
		  EvictorStorage.obj \
                  Exception.obj \
                  IndexI.obj \
                  Index.obj \
	  	  MapI.obj \
                  ObjectStore.obj \
		  PingObject.obj \
                  SharedDbEnv.obj \
                  SharedDb.obj \
                  TransactionHolder.obj \
                  TransactionI.obj \
                  Transaction.obj \
                  Util.obj

SRCS		= $(OBJS:.obj=.cpp)

HDIR		= $(includedir)\Freeze
SDIR		= $(slicedir)\Freeze

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I.. $(CPPFLAGS) -DFREEZE_API_EXPORTS
SLICE2CPPFLAGS	= --ice --include-dir Freeze --dll-export FREEZE_API $(SLICE2CPPFLAGS)
LINKWITH	= $(LIBS) $(DB_LIBS)

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	del /q $@
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$(DLLNAME) $(PRELIBS)$(LINKWITH)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)

$(HDIR)/Catalog.h Catalog.cpp: $(SLICE2FREEZE) $(SDIR)/CatalogData.ice
	del /q $(HDIR)\Catalog.h Catalog.cpp
	$(SLICE2FREEZE) $(SLICE2CPPFLAGS) --dict Freeze::Catalog,string,Freeze::CatalogData \
	Catalog ../../slice/Freeze/CatalogData.ice
	move Catalog.h $(HDIR)

clean::
	del /q $(HDIR)\Catalog.h Catalog.cpp

clean::
	del /q $(DLLNAME:.dll=.*)
	del /q DB.cpp $(HDIR)\DB.h
	del /q CatalogData.cpp $(HDIR)\CatalogData.h
	del /q Connection.cpp $(HDIR)\Connection.h
	del /q ConnectionF.cpp $(HDIR)\ConnectionF.h 
	del /q Exception.cpp $(HDIR)\Exception.h
	del /q EvictorF.cpp $(HDIR)\EvictorF.h
	del /q Evictor.cpp $(HDIR)\Evictor.h
	del /q EvictorStorage.cpp $(HDIR)\EvictorStorage.h
	del /q Transaction.cpp $(HDIR)\Transaction.h
	del /q PingObject.cpp PingObject.h

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)

!include .depend
