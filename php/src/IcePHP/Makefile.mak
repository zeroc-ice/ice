# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= php_ice$(LIBSUFFIX).lib
DLLNAME         = $(bindir)\php_ice$(LIBSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

OBJS		= Communicator.obj \
		  Init.obj \
		  Marshal.obj \
		  Profile.obj \
		  Proxy.obj \
		  Util.obj

SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. -I.. $(CPPFLAGS) $(ICE_CPPFLAGS) $(PHP_CPPFLAGS) 
!if "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(LIBNAME:.lib=.pdb)
!endif

LINKWITH        = $(ICE_LIBS) $(PHP_LIBS) $(CXXLIBS)

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	$(LINK) $(ICE_LDFLAGS) $(PHP_LDFLAGS) $(LD_DLLFLAGS) $(PDBFLAGS) /export:get_module $(OBJS) \
		$(PREOUT)$(DLLNAME) $(PRELIBS)$(LINKWITH)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)

install:: all
	copy $(DLLNAME) $(install_bindir)

!include .depend
