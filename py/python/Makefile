# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
		  Ice_PropertiesAdmin_ice.py \
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
		  Ice_Endpoint_ice.py \
		  Ice_EndpointF_ice.py \
		  Ice_EndpointTypes_ice.py \
		  Ice_Version_ice.py \
		  Ice_Instrumentation_ice.py \
		  Ice_InstrumentationF_ice.py \
		  Ice_Metrics_ice.py

GLACIER2_SRCS	= Glacier2_RouterF_ice.py \
		  Glacier2_Router_ice.py \
		  Glacier2_Session_ice.py \
		  Glacier2_PermissionsVerifierF_ice.py \
		  Glacier2_PermissionsVerifier_ice.py \
		  Glacier2_SSLInfo_ice.py \
		  Glacier2_Metrics_ice.py

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

ICESTORM_SRCS	= IceStorm_IceStorm_ice.py \
		  IceStorm_Metrics_ice.py

ALL_SRCS	= $(ICE_SRCS) \
		  $(GLACIER2_SRCS) \
		  $(ICEBOX_SRCS) \
		  $(ICEGRID_SRCS) \
		  $(ICEPATCH2_SRCS) \
		  $(ICESTORM_SRCS)

ICE_SLICE	= $(patsubst Ice_%_ice.py,$(slicedir)/Ice/%.ice,$(ICE_SRCS))
GLACIER2_SLICE	= $(patsubst Glacier2_%_ice.py,$(slicedir)/Glacier2/%.ice,$(GLACIER2_SRCS))
ICEBOX_SLICE	= $(patsubst IceBox_%_ice.py,$(slicedir)/IceBox/%.ice,$(ICEBOX_SRCS))
ICEGRID_SLICE	= $(patsubst IceGrid_%_ice.py,$(slicedir)/IceGrid/%.ice,$(ICEGRID_SRCS))
ICEPATCH2_SLICE	= $(patsubst IcePatch2_%_ice.py,$(slicedir)/IcePatch2/%.ice,$(ICEPATCH2_SRCS))
ICESTORM_SLICE	= $(patsubst IceStorm_%_ice.py,$(slicedir)/IceStorm/%.ice,$(ICESTORM_SRCS))

PACKAGES	= IceBox IceGrid IcePatch2 IceStorm IceMX

all:: $(ALL_SRCS)

SLICE2PYFLAGS	+= --ice

Ice_%_ice.py: $(slicedir)/Ice/%.ice $(SLICE2PY) $(SLICEPARSERLIB)
	$(SLICE2PY) --prefix Ice_ --no-package $(SLICE2PYFLAGS) $<

Ice_Metrics_ice.py: $(slicedir)/Ice/Metrics.ice $(SLICE2PY) $(SLICEPARSERLIB)
	$(SLICE2PY) --prefix Ice_ --checksum $(SLICE2PYFLAGS) $<

Glacier2_%_ice.py: $(slicedir)/Glacier2/%.ice $(SLICE2PY) $(SLICEPARSERLIB)
	$(SLICE2PY) --prefix Glacier2_ --no-package --checksum $(SLICE2PYFLAGS) $<

Glacier2_Metrics_ice.py: $(slicedir)/Glacier2/Metrics.ice $(SLICE2PY) $(SLICEPARSERLIB)
	$(SLICE2PY) --prefix Glacier2_ --checksum $(SLICE2PYFLAGS) $<

IceBox_%_ice.py: $(slicedir)/IceBox/%.ice $(SLICE2PY) $(SLICEPARSERLIB)
	$(SLICE2PY) --prefix IceBox_ --checksum $(SLICE2PYFLAGS) $<

IceGrid_%_ice.py: $(slicedir)/IceGrid/%.ice $(SLICE2PY) $(SLICEPARSERLIB)
	$(SLICE2PY) --prefix IceGrid_ --checksum $(SLICE2PYFLAGS) $<

IcePatch2_%_ice.py: $(slicedir)/IcePatch2/%.ice $(SLICE2PY) $(SLICEPARSERLIB)
	$(SLICE2PY) --prefix IcePatch2_ --checksum $(SLICE2PYFLAGS) $<

IceStorm_%_ice.py: $(slicedir)/IceStorm/%.ice $(SLICE2PY) $(SLICEPARSERLIB)
	$(SLICE2PY) --prefix IceStorm_ --checksum $(SLICE2PYFLAGS) $<

install:: $(ALL_SRCS)
	@echo "Installing generated code"
	$(INSTALL_DATA) *.py $(DESTDIR)$(install_pythondir)
	@for i in $(PACKAGES) ; \
	do \
	    $(INSTALL_DATA) -r $$i $(DESTDIR)$(install_pythondir) ; \
	done

depend::  $(ICE_SLICE) $(GLACIER2_SLICE) $(ICEBOX_SLICE) $(ICEGRID_SLICE) $(ICEPATCH2_SLICE) $(ICESTORM_SLICE)
	-rm -f .depend .depend.mak
	if test -n "$(ICE_SLICE)" ; then \
	    $(SLICE2PY) --depend --prefix Ice_ $(SLICE2PYFLAGS) $(ICE_SLICE) | $(ice_dir)/config/makedepend.py; \
	fi
	if test -n "$(GLACIER2_SLICE)" ; then \
	    $(SLICE2PY) --depend --prefix Glacier2_ $(SLICE2PYFLAGS) $(GLACIER2_SLICE) | $(ice_dir)/config/makedepend.py; \
	fi
	if test -n "$(ICEBOX_SLICE)" ; then \
	    $(SLICE2PY) --depend --prefix IceBox_ $(SLICE2PYFLAGS) $(ICEBOX_SLICE) | $(ice_dir)/config/makedepend.py; \
	fi
	if test -n "$(ICEGRID_SLICE)" ; then \
	    $(SLICE2PY) --depend --prefix IceGrid_ $(SLICE2PYFLAGS) $(ICEGRID_SLICE) | $(ice_dir)/config/makedepend.py; \
	fi
	if test -n "$(ICEPATCH2_SLICE)" ; then \
	    $(SLICE2PY) --depend --prefix IcePatch2_ $(SLICE2PYFLAGS) $(ICEPATCH2_SLICE) | $(ice_dir)/config/makedepend.py; \
	fi
	if test -n "$(ICESTORM_SLICE)" ; then \
	    $(SLICE2PY) --depend --prefix IceStorm_ $(SLICE2PYFLAGS) $(ICESTORM_SLICE) | $(ice_dir)/config/makedepend.py; \
	fi

clean::
	rm -rf $(PACKAGES) *_ice.py

include .depend
