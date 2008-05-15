# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

include $(top_srcdir)/config/Make.rules

#
# Parallel builds are not supported because multiple executions of slice2py could
# attempt to modify the __init__.py file simultaneously.
#
.NOTPARALLEL:

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
		  Ice_ImplicitContextF_ice.py \
		  Ice_ImplicitContext_ice.py \
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
		  Ice_SliceChecksumDict_ice.py \
		  Ice_Endpoint_ice.py

GLACIER2_SRCS	= Glacier2_RouterF_ice.py \
		  Glacier2_Router_ice.py \
		  Glacier2_SessionF_ice.py \
		  Glacier2_Session_ice.py \
		  Glacier2_PermissionsVerifierF_ice.py \
		  Glacier2_PermissionsVerifier_ice.py \
		  Glacier2_SSLInfo_ice.py

ICEBOX_SRCS	= IceBox_IceBox_ice.py

ICEGRID_SRCS	= IceGrid_Admin_ice.py \
		  IceGrid_Descriptor_ice.py \
		  IceGrid_Exception_ice.py \
		  IceGrid_FileParser_ice.py \
		  IceGrid_Observer_ice.py \
		  IceGrid_Locator_ice.py \
		  IceGrid_Query_ice.py \
		  IceGrid_Registry_ice.py \
		  IceGrid_Session_ice.py \
		  IceGrid_UserAccountMapper_ice.py

ICEPATCH2_SRCS	= IcePatch2_FileInfo_ice.py \
		  IcePatch2_FileServer_ice.py

ICESTORM_SRCS	= IceStorm_IceStorm_ice.py

ALL_SRCS	= $(ICE_SRCS) \
		  $(GLACIER2_SRCS) \
		  $(ICEBOX_SRCS) \
		  $(ICEGRID_SRCS) \
		  $(ICEPATCH2_SRCS) \
		  $(ICESTORM_SRCS)

PACKAGES	= Glacier2 IceBox IceGrid IcePatch2 IceStorm

all:: $(ALL_SRCS)

Ice_%_ice.py: $(slicedir)/Ice/%.ice
	$(SLICE2PY) --prefix Ice_ --ice --no-package $(SLICE2PYFLAGS) $<

Glacier2_%_ice.py: $(slicedir)/Glacier2/%.ice
	$(SLICE2PY) --prefix Glacier2_ --checksum $(SLICE2PYFLAGS) $<

IceBox_%_ice.py: $(slicedir)/IceBox/%.ice
	$(SLICE2PY) --prefix IceBox_ --ice --checksum $(SLICE2PYFLAGS) $<

IceGrid_%_ice.py: $(slicedir)/IceGrid/%.ice
	$(SLICE2PY) --prefix IceGrid_ --ice --checksum $(SLICE2PYFLAGS) $<

IcePatch2_%_ice.py: $(slicedir)/IcePatch2/%.ice
	$(SLICE2PY) --prefix IcePatch2_ --ice --checksum $(SLICE2PYFLAGS) $<

IceStorm_%_ice.py: $(slicedir)/IceStorm/%.ice
	$(SLICE2PY) --prefix IceStorm_ --ice --checksum $(SLICE2PYFLAGS) $<

install:: $(ALL_SRCS)
	@echo "Installing generated code"
	$(INSTALL_DATA) *.py $(install_pythondir)
	@for i in $(PACKAGES) ; \
	do \
	    $(INSTALL_DATA) -r $$i $(install_pythondir) ; \
	done

clean::
	rm -rf $(PACKAGES) *_ice.py
