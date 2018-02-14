# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

ICE_SRCS	= Ice_BuiltinSequences_ice.py \
		  Ice_Communicator_ice.py \
		  Ice_CommunicatorF_ice.py \
		  Ice_Connection_ice.py \
		  Ice_ConnectionF_ice.py \
		  Ice_Current_ice.py \
		  Ice_Endpoint_ice.py \
		  Ice_EndpointF_ice.py \
		  Ice_EndpointTypes_ice.py \
		  Ice_FacetMap_ice.py \
		  Ice_Identity_ice.py \
		  Ice_ImplicitContext_ice.py \
		  Ice_ImplicitContextF_ice.py \
		  Ice_Instrumentation_ice.py \
		  Ice_InstrumentationF_ice.py \
		  Ice_LocalException_ice.py \
		  Ice_Locator_ice.py \
		  Ice_LocatorF_ice.py \
		  Ice_Logger_ice.py \
		  Ice_LoggerF_ice.py \
		  Ice_Metrics_ice.py \
		  Ice_ObjectAdapter_ice.py \
		  Ice_ObjectAdapterF_ice.py \
		  Ice_ObjectFactory_ice.py \
		  Ice_ObjectFactoryF_ice.py \
		  Ice_Plugin_ice.py \
		  Ice_PluginF_ice.py \
		  Ice_Process_ice.py \
		  Ice_ProcessF_ice.py \
		  Ice_Properties_ice.py \
		  Ice_PropertiesAdmin_ice.py \
		  Ice_PropertiesF_ice.py \
		  Ice_RemoteLogger_ice.py \
		  Ice_Router_ice.py \
		  Ice_RouterF_ice.py \
		  Ice_ServantLocator_ice.py \
		  Ice_ServantLocatorF_ice.py \
		  Ice_SliceChecksumDict_ice.py \
		  Ice_Version_ice.py

GLACIER2_SRCS	= Glacier2_Metrics_ice.py \
		  Glacier2_PermissionsVerifier_ice.py \
		  Glacier2_PermissionsVerifierF_ice.py \
		  Glacier2_Router_ice.py \
		  Glacier2_RouterF_ice.py \
		  Glacier2_Session_ice.py \
		  Glacier2_SSLInfo_ice.py

ICEBOX_SRCS	= IceBox_IceBox_ice.py

ICEGRID_SRCS	= IceGrid_Admin_ice.py \
		  IceGrid_Descriptor_ice.py \
		  IceGrid_Exception_ice.py \
		  IceGrid_FileParser_ice.py \
		  IceGrid_Locator_ice.py \
		  IceGrid_Observer_ice.py \
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

Ice_%_ice.py: $(slicedir)/Ice/%.ice $(SLICEPARSERLIB)
	$(SLICE2PY) --prefix Ice_ --no-package $(SLICE2PYFLAGS) $<
	@mkdir -p .depend
	@$(SLICE2PY) --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend $< > .depend/$(*F).ice.d

Ice_Metrics_ice.py: $(slicedir)/Ice/Metrics.ice $(SLICEPARSERLIB)
	$(SLICE2PY) --prefix Ice_ --checksum $(SLICE2PYFLAGS) $<
	@mkdir -p .depend
	@$(SLICE2PY) --prefix Ice_ --checksum $(SLICE2PYFLAGS) --depend $< > .depend/$(*F).ice.d

Glacier2_%_ice.py: $(slicedir)/Glacier2/%.ice $(SLICEPARSERLIB)
	$(SLICE2PY) --prefix Glacier2_ --no-package --checksum $(SLICE2PYFLAGS) $<
	@mkdir -p .depend
	@$(SLICE2PY) --prefix Glacier2_ --no-package --checksum $(SLICE2PYFLAGS) --depend $< > .depend/$(*F).ice.d

Glacier2_Metrics_ice.py: $(slicedir)/Glacier2/Metrics.ice $(SLICEPARSERLIB)
	$(SLICE2PY) --prefix Glacier2_ --checksum $(SLICE2PYFLAGS) $<
	@mkdir -p .depend
	@$(SLICE2PY) --prefix Glacier2_ --checksum $(SLICE2PYFLAGS) --depend $< > .depend/$(*F).ice.d

IceBox_%_ice.py: $(slicedir)/IceBox/%.ice $(SLICEPARSERLIB)
	$(SLICE2PY) --prefix IceBox_ --checksum $(SLICE2PYFLAGS) $<
	@mkdir -p .depend
	@$(SLICE2PY) --prefix IceBox_ --checksum $(SLICE2PYFLAGS) --depend $< > .depend/$(*F).ice.d

IceGrid_%_ice.py: $(slicedir)/IceGrid/%.ice $(SLICEPARSERLIB)
	$(SLICE2PY) --prefix IceGrid_ --checksum $(SLICE2PYFLAGS) $<
	@mkdir -p .depend
	@$(SLICE2PY) --prefix IceGrid_ --checksum $(SLICE2PYFLAGS) --depend $< > .depend/$(*F).ice.d

IcePatch2_%_ice.py: $(slicedir)/IcePatch2/%.ice $(SLICEPARSERLIB)
	$(SLICE2PY) --prefix IcePatch2_ --checksum $(SLICE2PYFLAGS) $<
	@mkdir -p .depend
	@$(SLICE2PY) --prefix IcePatch2_ --checksum $(SLICE2PYFLAGS) --depend $< > .depend/$(*F).ice.d

IceStorm_%_ice.py: $(slicedir)/IceStorm/%.ice $(SLICEPARSERLIB)
	$(SLICE2PY) --prefix IceStorm_ --checksum $(SLICE2PYFLAGS) $<
	@mkdir -p .depend
	@$(SLICE2PY) --prefix IceStorm_ --checksum $(SLICE2PYFLAGS) --depend $< > .depend/$(*F).ice.d

install:: $(ALL_SRCS)
	@echo "Installing generated code"
	$(INSTALL_DATA) *.py $(DESTDIR)$(install_pythondir)
	@for i in $(PACKAGES) ; \
	do \
	    $(INSTALL_DATA) -r $$i $(DESTDIR)$(install_pythondir) ; \
	done

clean::
	rm -rf $(PACKAGES) *_ice.py
