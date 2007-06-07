# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\glacier2$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\glacier2$(SOVERSION)$(LIBSUFFIX).dll

ROUTER		= $(top_srcdir)\bin\glacier2router.exe

!ifdef BUILD_UTILS

TARGETS         = $(ROUTER)

!else

TARGETS         = $(LIBNAME) $(DLLNAME)

!endif

OBJS		= PermissionsVerifier.obj \
		  Router.obj \
		  SSLInfo.obj \
		  Session.obj

ROBJS		= Blobject.obj \
		  ClientBlobject.obj \
		  CryptPermissionsVerifierI.obj \
		  Glacier2Router.obj \
		  ProxyVerifier.obj \
		  RequestQueue.obj \
		  RouterI.obj \
		  RoutingTable.obj \
		  FilterI.obj \
		  FilterManager.obj \
		  ServerBlobject.obj \
		  SessionRouterI.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(ROBJS:.obj=.cpp)

HDIR		= $(includedir)\Glacier2
SDIR		= $(slicedir)\Glacier2

!include $(top_srcdir)\config\Make.rules.mak

!ifdef BUILD_UTILS

CPPFLAGS	= -I.. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
LINKWITH 	= $(LIBS) $(OPENSSL_LIBS) glacier2$(LIBSUFFIX).lib icessl$(LIBSUFFIX).lib
!if "$(CPP_COMPILER)" != "BCC2006"
LINKWITH	= $(LINKWITH) ws2_32.lib
!endif

!else

CPPFLAGS	= -I.. $(CPPFLAGS) -DGLACIER2_API_EXPORTS

!endif

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
RPDBFLAGS       = /pdb:$(ROUTER:.exe=.pdb)
!endif

SLICE2CPPFLAGS	= --include-dir Glacier2 --dll-export GLACIER2_API $(SLICE2CPPFLAGS)

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(ROUTER): $(ROBJS)
	$(LINK) $(LD_EXEFLAGS) $(RPDBFLAGS) $(ROBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(LINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

!ifdef BUILD_UTILS

clean::
	del /q PermissionsVerifierF.cpp $(HDIR)\PermissionsVerifierF.h
	del /q PermissionsVerifier.cpp $(HDIR)\PermissionsVerifier.h
	del /q RouterF.cpp $(HDIR)\RouterF.h
	del /q Router.cpp $(HDIR)\Router.h
	del /q SessionF.cpp $(HDIR)\SessionF.h
	del /q Session.cpp $(HDIR)\Session.h
	del /q SSLInfo.cpp $(HDIR)\SSLInfo.h
	del /q $(DLLNAME:.dll=.*)
	del /q $(ROUTER:.exe=.*)

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)
	copy $(ROUTER) $(install_bindir)

!if "$(OPTIMIZE)" != "yes"

!if "$(CPP_COMPILER)" == "BCC2006"

install:: all
	copy $(DLLNAME:.dll=.tds) $(install_bindir)
	copy $(ROUTER:.exe=.tds) $(install_bindir)

!else

install:: all
	copy $(DLLNAME:.dll=.pdb) $(install_bindir)
	copy $(ROUTER:.exe=.pdb) $(install_bindir)

!endif

!endif

!else

install:: all

$(EVERYTHING)::
	@$(MAKE) -nologo /f Makefile.mak BUILD_UTILS=1 $@

!endif

!include .depend
