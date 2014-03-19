# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME     	= $(top_srcdir)\lib\freeze$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\freeze$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

OBJS		= BackgroundSaveEvictor.obj \
                  BackgroundSaveEvictorI.obj \
                  CatalogData.obj \
                  Catalog.obj \
                  CatalogIndexList.obj \
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
                  MapDb.obj \
	  	  MapI.obj \
                  ObjectStore.obj \
		  PingObject.obj \
                  SharedDbEnv.obj \
                  TransactionalEvictor.obj \
                  TransactionalEvictorI.obj \
                  TransactionalEvictorContext.obj \
                  TransactionHolder.obj \
                  TransactionI.obj \
                  Transaction.obj \
                  Util.obj

SRCS		= $(OBJS:.obj=.cpp)

HDIR		= $(headerdir)\Freeze
SDIR		= $(slicedir)\Freeze

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I.. $(CPPFLAGS) -DFREEZE_API_EXPORTS -DWIN32_LEAN_AND_MEAN
SLICE2CPPFLAGS	= --ice --include-dir Freeze --dll-export FREEZE_API $(SLICE2CPPFLAGS)
LINKWITH	= $(LIBS) $(DB_LIBS)

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

RES_FILE        = Freeze.res

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) Freeze.res
	$(LINK) $(BASE):0x25000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(HDIR)/Catalog.h Catalog.cpp: $(SDIR)/CatalogData.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q $(HDIR)\Catalog.h Catalog.cpp
	$(SLICE2FREEZE) $(SLICE2CPPFLAGS) --dict Freeze::Catalog,string,Freeze::CatalogData \
	Catalog $(slicedir)/Freeze/CatalogData.ice
	move Catalog.h $(HDIR)

$(HDIR)/CatalogIndexList.h CatalogIndexList.cpp: $(slicedir)/Ice/BuiltinSequences.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q $(HDIR)\CatalogIndexList.h CatalogIndexList.cpp
	$(SLICE2FREEZE) $(SLICE2CPPFLAGS) --dict Freeze::CatalogIndexList,string,Ice::StringSeq \
	CatalogIndexList $(slicedir)/Ice/BuiltinSequences.ice
	move CatalogIndexList.h $(HDIR)

clean::
	-del /q Catalog.cpp $(HDIR)\Catalog.h
	-del /q CatalogIndexList.cpp $(HDIR)\CatalogIndexList.h
	-del /q DB.cpp $(HDIR)\DB.h
	-del /q BackgroundSaveEvictor.cpp $(HDIR)\BackgroundSaveEvictor.h
	-del /q CatalogData.cpp $(HDIR)\CatalogData.h
	-del /q Connection.cpp $(HDIR)\Connection.h
	-del /q ConnectionF.cpp $(HDIR)\ConnectionF.h 
	-del /q Exception.cpp $(HDIR)\Exception.h
	-del /q EvictorF.cpp $(HDIR)\EvictorF.h
	-del /q Evictor.cpp $(HDIR)\Evictor.h
	-del /q EvictorStorage.cpp $(HDIR)\EvictorStorage.h
	-del /q Transaction.cpp $(HDIR)\Transaction.h
	-del /q TransactionalEvictor.cpp $(HDIR)\TransactionalEvictor.h
	-del /q PingObject.cpp PingObject.h
	-del /q Freeze.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"

!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif

!include .depend.mak
