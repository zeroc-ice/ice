# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

SRCS		= Application.py \
		  BuiltinSequences_ice.py \
		  Communicator_ice.py \
		  CommunicatorF_ice.py \
		  Current_ice.py \
		  FacetMap_ice.py \
		  Identity_ice.py \
		  LocalException_ice.py \
		  Locator_ice.py \
		  LocatorF_ice.py \
		  Logger_ice.py \
		  LoggerF_ice.py \
		  ObjectAdapter_ice.py \
		  ObjectAdapterF_ice.py \
		  ObjectFactory_ice.py \
		  ObjectFactoryF_ice.py \
		  Plugin_ice.py \
		  PluginF_ice.py \
		  Process_ice.py \
		  ProcessF_ice.py \
		  Properties_ice.py \
		  PropertiesF_ice.py \
		  Router_ice.py \
		  RouterF_ice.py \
		  ServantLocator_ice.py \
		  ServantLocatorF_ice.py \
		  Stats_ice.py \
		  StatsF_ice.py \
		  TransportInfo_ice.py \
		  TransportInfoF_ice.py

SLICE_SRCS	= $(SDIR)/LocalException.ice \
		  $(SDIR)/Communicator.ice \
		  $(SDIR)/CommunicatorF.ice \
		  $(SDIR)/Logger.ice \
		  $(SDIR)/LoggerF.ice \
		  $(SDIR)/BuiltinSequences.ice \
		  $(SDIR)/ObjectAdapter.ice \
		  $(SDIR)/ObjectAdapterF.ice \
		  $(SDIR)/ServantLocator.ice \
		  $(SDIR)/ServantLocatorF.ice \
		  $(SDIR)/Properties.ice \
		  $(SDIR)/PropertiesF.ice \
		  $(SDIR)/ObjectFactory.ice \
		  $(SDIR)/ObjectFactoryF.ice \
		  $(SDIR)/Identity.ice \
		  $(SDIR)/Current.ice \
		  $(SDIR)/Router.ice \
		  $(SDIR)/RouterF.ice \
		  $(SDIR)/Plugin.ice \
		  $(SDIR)/PluginF.ice \
		  $(SDIR)/Locator.ice \
		  $(SDIR)/LocatorF.ice \
                  $(SDIR)/StatsF.ice \
                  $(SDIR)/Stats.ice \
                  $(SDIR)/Process.ice \
                  $(SDIR)/ProcessF.ice \
		  $(SDIR)/TransportInfo.ice \
		  $(SDIR)/TransportInfoF.ice

SDIR		= $(slicedir)/Ice

include $(top_srcdir)/config/Make.rules

SLICE2PYFLAGS	:= --ice --no-package $(SLICE2PYFLAGS)
