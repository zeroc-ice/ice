# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

OBJS		= PermissionsVerifier.o \
		  Router.o \
		  SSLInfo.o \
		  Session.o

ROBJS		= Blobject.o \
		  ClientBlobject.o \
		  CryptPermissionsVerifierI.o \
		  Glacier2Router.o \
		  ProxyVerifier.o \
		  RequestQueue.o \
		  RouterI.o \
		  RoutingTable.o \
		  FilterI.o \
		  FilterManager.o \
		  ServerBlobject.o \
		  SessionRouterI.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(ROBJS:.o=.cpp)

HDIR		= $(includedir)\Glacier2
SDIR		= $(slicedir)\Glacier2

!include $(top_srcdir)\config\Make.rules.mak

!ifdef BUILD_UTILS

CPPFLAGS	= -I.. $(CPPFLAGS)
LINKWITH 	= $(LIBS) $(OPENSSL_LIBS) glacier2$(LIBSUFFIX).lib icessl$(LIBSUFFIX).lib

!else

CPPFLAGS	= -I.. $(CPPFLAGS) -DGLACIER2_API_EXPORTS

!endif

SLICE2CPPFLAGS	= --include-dir Glacier2 --dll-export GLACIER2_API $(SLICE2CPPFLAGS)

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	del /q $@
	$(LINK) $(LD_DLLFLAGS) $(OBJS), $(DLLNAME),, $(LIBS)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)

$(ROUTER): $(ROBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(ROBJS), $@,, $(LINKWITH)

$(HDIR)\PermissionsVerifierF.h: $(SDIR)\PermissionsVerifierF.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\PermissionsVerifierF.ice
	del /q PermissionsVerifierF.cpp
	move PermissionsVerifierF.h $(HDIR)

PermissionsVerifier.cpp $(HDIR)\PermissionsVerifier.h: $(SDIR)\PermissionsVerifier.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\PermissionsVerifier.ice
	move PermissionsVerifier.h $(HDIR)

$(HDIR)\RouterF.h: $(SDIR)\RouterF.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\RouterF.ice
	del /q RouterF.cpp
	move RouterF.h $(HDIR)

Router.cpp $(HDIR)\Router.h: $(SDIR)\Router.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Router.ice
	move Router.h $(HDIR)

$(HDIR)\SessionF.h: $(SDIR)\SessionF.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\SessionF.ice
	del /q SessionF.cpp
	move SessionF.h $(HDIR)

Session.cpp $(HDIR)\Session.h: $(SDIR)\Session.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Session.ice
	move Session.h $(HDIR)

SSLInfo.cpp $(HDIR)\SSLInfo.h: $(SDIR)\SSLInfo.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\SSLInfo.ice
	move SSLInfo.h $(HDIR)

!ifdef BUILD_UTILS

clean::
	del /q $(HDIR)\PermissionsVerifierF.h
	del /q PermissionsVerifier.cpp $(HDIR)\PermissionsVerifier.h
	del /q $(HDIR)\RouterF.h
	del /q Router.cpp $(HDIR)\Router.h
	del /q $(HDIR)\SessionF.h
	del /q Session.cpp $(HDIR)\Session.h
	del /q SSLInfo.cpp $(HDIR)\SSLInfo.h

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)
	copy $(ROUTER) $(install_bindir)

!else

install:: all

$(EVERYTHING)::
	$(MAKE) /f Makefile.mak BUILD_UTILS=1 $@

!endif

!include .depend
