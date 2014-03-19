
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

LIBNAME		= $(top_srcdir)\lib\icegridsqldb$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icegridsqldb$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

TARGETS         = $(LIBNAME) $(DLLNAME)

OBJS  		= SqlStringApplicationInfoDict.obj \
		  SqlIdentityObjectInfoDict.obj \
		  SqlStringAdapterInfoDict.obj \
		  SqlDB.obj

DB_OBJS		= SqlTypes.obj

{$(top_srcdir)\src\IceDB\}.cpp.obj::
    $(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

SRCS		= $(OBJS:.obj=.cpp) \
		  $(top_srcdir)\src\IceDB\SqlTypes.cpp

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I..\.. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

LINKWITH 	= $(QT_LIBS) icegrid$(LIBSUFFIX).lib glacier2$(LIBSUFFIX).lib icedb$(LIBSUFFIX).lib $(LIBS)

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

RES_FILE        = IceGridSqlDB.res

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) $(DB_OBJS) IceGridSqlDB.res
	$(LINK) /base:0x2E000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(DB_OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) \
		$(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

clean::
	-del /q IceGridSqlDB.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"

!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif

!include .depend.mak
