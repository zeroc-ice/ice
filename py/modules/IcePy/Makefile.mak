# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME     	= IcePy$(PYLIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\python\IcePy$(PYLIBSUFFIX).pyd

TARGETS		= $(LIBNAME) $(DLLNAME)

OBJS		= Communicator.obj \
		  Connection.obj \
		  ConnectionInfo.obj \
		  Current.obj \
		  Endpoint.obj \
		  EndpointInfo.obj \
		  ImplicitContext.obj \
		  Init.obj \
		  Logger.obj \
		  ObjectAdapter.obj \
		  ObjectFactory.obj \
		  Operation.obj \
		  Properties.obj \
		  PropertiesAdmin.obj \
		  Proxy.obj \
		  Slice.obj \
		  Thread.obj \
		  Types.obj \
		  Util.obj

SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) $(ICE_CPPFLAGS) $(PYTHON_CPPFLAGS)

LINKWITH        = $(ICE_LIBS) $(PYTHON_LIBS) $(CXXLIBS)

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) IcePy.res
	$(LINK) $(PYTHON_LDFLAGS) $(ICE_LDFLAGS) $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) \
		$(PREOUT)$@ $(PRELIBS)$(LINKWITH) IcePy.res
	move $(@:.pyd=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	   $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(@:.pyd=.exp) del /q $(@:.pyd=.exp)


clean::
	-del /q IcePy.res

install:: all
	copy $(DLLNAME) "$(install_libdir)"

!include .depend.mak
