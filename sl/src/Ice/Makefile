# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

PKG		= IceSL
LIBNAME		= $(PKG).dll
TARGETS		= $(bindir)/$(LIBNAME)

SRCS		= Arrays.cs \
		  AssemblyInfo.cs \
		  AssemblyUtil.cs \
		  Base64.cs \
		  BasicStream.cs \
		  Object.cs \
		  ByteBuffer.cs \
		  CollectionBase.cs \
		  Collections.cs \
		  CommunicatorI.cs \
		  Compare.cs \
		  DefaultsAndOverrides.cs \
		  DictionaryBase.cs \
		  EndpointFactory.cs \
		  EndpointFactoryManager.cs \
		  EndpointI.cs \
		  Exception.cs \
		  ImplicitContextI.cs \
		  Instance.cs \
		  LoggerI.cs \
		  ObjectFactoryManager.cs \
		  OutgoingBase.cs \
		  Outgoing.cs \
		  OutgoingAsync.cs \
		  OutputBase.cs \
		  Patcher.cs \
		  PropertiesI.cs \
		  Property.cs \
		  PropertyNames.cs \
		  Protocol.cs \
		  Proxy.cs \
		  ProxyFactory.cs \
		  ProxyIdentityKey.cs \
		  ReadObjectCallback.cs \
		  Reference.cs \
		  ReferenceFactory.cs \
		  ReplyStatus.cs \
		  Set.cs \
		  StringUtil.cs \
		  TraceLevels.cs \
		  UnknownEndpointI.cs \
		  TcpEndpointI.cs \
		  UdpEndpointI.cs \
		  UserExceptionFactory.cs \
		  Util.cs \
		  ValueWriter.cs \
		  TraceUtil.cs

SLICE_SRCS	= $(SDIR)/BuiltinSequences.ice \
		  $(SDIR)/Communicator.ice \
		  $(SDIR)/Endpoint.ice \
		  $(SDIR)/FacetMap.ice \
		  $(SDIR)/Identity.ice \
		  $(SDIR)/ImplicitContext.ice \
		  $(SDIR)/LocalException.ice \
		  $(SDIR)/Logger.ice \
		  $(SDIR)/ObjectFactory.ice \
		  $(SDIR)/Properties.ice \
		  $(SDIR)/Stats.ice \
		  $(SDIR)/Context.ice

SDIR		= $(slicedir)/Ice
GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:library -out:$(TARGETS) -warnaserror- -r:System.Web.dll
MCSFLAGS	:= $(MCSFLAGS) -keyfile:$(KEYFILE)

SLICE2SLFLAGS	:= $(SLICE2CSFLAGS) --ice -I$(slicedir)

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) $(subst /,$(DSEP),$^)

ifeq ($(NOGAC),)

install:: all
	$(GACUTIL) -i $(bindir)/$(LIBNAME)

else

install:: all
	$(call installlibrary,$(bindir)/IceSL.dll,$(install_bindir))

endif

include .depend
