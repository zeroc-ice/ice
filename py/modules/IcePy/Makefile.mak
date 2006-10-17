# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME     	= IcePy$(PYLIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\python\IcePy$(PYLIBSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

OBJS		= Communicator.obj \
		  Connection.obj \
		  Current.obj \
		  Init.obj \
		  Logger.obj \
		  ObjectAdapter.obj \
		  ObjectFactory.obj \
		  Operation.obj \
		  Properties.obj \
		  Proxy.obj \
		  Slice.obj \
		  ThreadNotification.obj \
		  Types.obj \
		  Util.obj

SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) $(ICE_CPPFLAGS) $(PYTHON_CPPFLAGS)

LINKWITH        = $(ICE_LIBS) $(PYTHON_LIBS) $(CXXLIBS)

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	$(LINK) $(PYTHON_LDFLAGS) $(ICE_LDFLAGS) $(LD_DLLFLAGS) $(PDBFLAGS) /export:initIcePy $(OBJS) \
		$(PREOUT)$(DLLNAME) $(PRELIBS)$(LINKWITH)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)

install:: all
	copy $(DLLNAME) $(install_libdir)

!include .depend
