# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

include $(top_srcdir)/config/Make.rules

ICE_SRCS	= Ice_LocalException_ice.py \
		  Ice_Communicator_ice.py \
		  Ice_CommunicatorF_ice.py \
		  Ice_Logger_ice.py \
		  Ice_LoggerF_ice.py \
		  Ice_BuiltinSequences_ice.py \
		  Ice_ObjectAdapter_ice.py \
		  Ice_ObjectAdapterF_ice.py \
		  Ice_ServantLocator_ice.py \
		  Ice_ServantLocatorF_ice.py \
		  Ice_Properties_ice.py \
		  Ice_PropertiesF_ice.py \
		  Ice_ObjectFactory_ice.py \
		  Ice_ObjectFactoryF_ice.py \
		  Ice_Identity_ice.py \
		  Ice_Current_ice.py \
		  Ice_Router_ice.py \
		  Ice_RouterF_ice.py \
		  Ice_Plugin_ice.py \
		  Ice_PluginF_ice.py \
		  Ice_Locator_ice.py \
		  Ice_LocatorF_ice.py \
		  Ice_StatsF_ice.py \
		  Ice_Stats_ice.py \
		  Ice_Process_ice.py \
		  Ice_ProcessF_ice.py \
		  Ice_FacetMap_ice.py \
		  Ice_Connection_ice.py \
		  Ice_ConnectionF_ice.py \
		  Ice_SliceChecksumDict_ice.py

GLACIER_SRCS	= Glacier_Router_ice.py \
		  Glacier_SessionF_ice.py \
		  Glacier_Session_ice.py \
		  Glacier_SessionManagerF_ice.py \
		  Glacier_SessionManager_ice.py \
		  Glacier_Starter_ice.py

GLACIER2_SRCS	= Glacier2_PermissionsVerifierF_ice.py \
		  Glacier2_PermissionsVerifier_ice.py \
		  Glacier2_RouterF_ice.py \
		  Glacier2_Router_ice.py

ICEBOX_SRCS	= IceBox_IceBox_ice.py

ICEPACK_SRCS	= IcePack_Admin_ice.py \
		  IcePack_Exception_ice.py \
		  IcePack_Query_ice.py

ICEPATCH_SRCS	= IcePatch_IcePatch_ice.py

ICESTORM_SRCS	= IceStorm_IceStorm_ice.py

all:: $(ICE_SRCS) $(GLACIER_SRCS) $(GLACIER2_SRCS) $(ICEBOX_SRCS) $(ICEPACK_SRCS) $(ICEPATCH_SRCS) $(ICESTORM_SRCS)

Ice_%_ice.py: $(slicedir)/Ice/%.ice
	$(SLICE2PY) --prefix Ice_ --ice --no-package $(SLICE2PYFLAGS) $<

Glacier_%_ice.py: $(slicedir)/Glacier/%.ice
	$(SLICE2PY) --prefix Glacier_ --checksum $(SLICE2PYFLAGS) $<

Glacier2_%_ice.py: $(slicedir)/Glacier2/%.ice
	$(SLICE2PY) --prefix Glacier2_ --checksum $(SLICE2PYFLAGS) $<

IceBox_%_ice.py: $(slicedir)/IceBox/%.ice
	$(SLICE2PY) --prefix IceBox_ --ice --checksum $(SLICE2PYFLAGS) $<

IcePack_%_ice.py: $(slicedir)/IcePack/%.ice
	$(SLICE2PY) --prefix IcePack_ --ice --checksum $(SLICE2PYFLAGS) $<

IcePatch_%_ice.py: $(slicedir)/IcePatch/%.ice
	$(SLICE2PY) --prefix IcePatch_ --ice --checksum $(SLICE2PYFLAGS) $<

IceStorm_%_ice.py: $(slicedir)/IceStorm/%.ice
	$(SLICE2PY) --prefix IceStorm_ --ice --checksum $(SLICE2PYFLAGS) $<

clean::
	rm -rf Glacier Glacier2 IceBox IcePack IcePatch IceStorm
	rm -f *_ice.py
