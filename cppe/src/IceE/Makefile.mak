# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\icee$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icee$(SOVERSION)$(LIBSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

TRANSPORT_DIR   = $(top_srcdir)\src\TcpTransport

TRANSPORT_OBJS  = Acceptor.obj \
                  Connector.obj \
                  EndpointFactory.obj \
                  TcpEndpoint.obj \
                  Transceiver.obj

LOCAL_OBJS      = BasicStream.obj \
                  Buffer.obj \
                  BuiltinSequences.obj \
                  Communicator.obj \
                  Cond.obj \
                  Connection.obj \
                  ConvertUTF.obj \
                  Current.obj \
                  DefaultsAndOverrides.obj \
                  Endpoint.obj \
                  ExceptionBase.obj \
                  FactoryTable.obj \
                  FactoryTableDef.obj \
                  Identity.obj \
                  Incoming.obj \
                  IncomingConnectionFactory.obj \
                  Initialize.obj \
                  Instance.obj \
                  LocalException.obj \
                  Locator.obj \
                  LocatorInfo.obj \
                  Logger.obj \
                  LoggerI.obj \
                  LoggerUtil.obj \
                  Network.obj \
                  Object.obj \
                  ObjectAdapter.obj \
                  ObjectAdapterFactory.obj \
                  OperationMode.obj \
                  Outgoing.obj \
                  OutgoingConnectionFactory.obj \
                  Properties.obj \
                  Protocol.obj \
                  Proxy.obj \
                  ProxyFactory.obj \
                  RecMutex.obj \
                  Reference.obj \
                  ReferenceFactory.obj \
                  Router.obj \
                  RouterInfo.obj \
                  SafeStdio.obj \
                  ServantManager.obj \
                  Shared.obj \
                  StaticMutex.obj \
                  StringConverter.obj \
                  StringUtil.obj \
                  Thread.obj \
                  ThreadException.obj \
                  Time.obj \
                  TraceLevels.obj \
                  TraceUtil.obj \
                  UnknownEndpoint.obj \
                  Unicode.obj \
                  UUID.obj

SRCS		= $(LOCAL_OBJS:.obj=.cpp) \
		  $(TRANSPORT_DIR)\Acceptor.cpp \
                  $(TRANSPORT_DIR)\Connector.cpp \
                  $(TRANSPORT_DIR)\EndpointFactory.cpp \
                  $(TRANSPORT_DIR)\TcpEndpoint.cpp \
                  $(TRANSPORT_DIR)\Transceiver.cpp
		

HDIR		= $(includedir)\IceE
SDIR		= $(slicedir)\IceE

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I.. $(CPPFLAGS) -DICE_API_EXPORTS -DFD_SETSIZE=1024 -WX -DWIN32_LEAN_AND_MEAN
SLICE2CPPEFLAGS	= --ice --include-dir IceE --dll-export ICE_API $(SLICE2CPPEFLAGS)

!if "$(STATICLIBS)" != "yes" && "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes" 
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

{$(TRANSPORT_DIR)\}.cpp.obj::
    $(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

!if "$(STATICLIBS)" == "yes"

$(DLLNAME): 

$(LIBNAME): $(LOCAL_OBJS) $(TRANSPORT_OBJS)
	$(AR) $(ARFLAGS) $(PDBFLAGS) $(LOCAL_OBJS) $(TRANSPORT_OBJS) /out:$(LIBNAME)
	
!else

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(LOCAL_OBJS) $(TRANSPORT_OBJS)
	$(LINK) $(LDFLAGS) /dll $(PDBFLAGS) $(LOCAL_OBJS) $(TRANSPORT_OBJS) /out:$(DLLNAME) $(BASELIBS)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

!endif

clean::
	del /q BuiltinSequences.cpp $(HDIR)\BuiltinSequences.h
	del /q Identity.cpp $(HDIR)\Identity.h
	del /q LocatorF.cpp $(HDIR)\LocatorF.h
	del /q Locator.cpp $(HDIR)\Locator.h
	del /q RouterF.cpp $(HDIR)\RouterF.h
	del /q Router.cpp $(HDIR)\Router.h
	del /q $(LIBNAME:.lib=.*)

install:: all
	copy $(LIBNAME) $(install_libdir)

!if "$(STATICLIBS)" != "yes"

clean::
	del /q $(DLLNAME:.dll=.*)

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)

!endif

!include .depend
