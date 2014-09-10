# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

!include $(top_srcdir)\config\Make.rules.mak

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
		  Ice_Router_ice.py \
		  Ice_RouterF_ice.py \
		  Ice_ServantLocator_ice.py \
		  Ice_ServantLocatorF_ice.py \
		  Ice_SliceChecksumDict_ice.py \
		  Ice_Version_ice.py \

GLACIER2_SRCS	= Glacier2_Metrics_ice.py \
		  Glacier2_PermissionsVerifier_ice.py \
		  Glacier2_PermissionsVerifierF_ice.py \
		  Glacier2_Router_ice.py \
		  Glacier2_RouterF_ice.py \
		  Glacier2_Session_ice.py \
		  Glacier2_SSLInfo_ice.py \

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
		  IceGrid_UserAccountMapper_ice.py \

ICEPATCH2_SRCS	= IcePatch2_FileInfo_ice.py \
		  IcePatch2_FileServer_ice.py

ICESTORM_SRCS	= IceStorm_Metrics_ice.py \
		  IceStorm_IceStorm_ice.py

ALL_SRCS	= $(ICE_SRCS) \
		  $(GLACIER2_SRCS) \
		  $(ICEBOX_SRCS) \
		  $(ICEGRID_SRCS) \
		  $(ICEPATCH2_SRCS) \
		  $(ICESTORM_SRCS)


SLICE_DEPENDS = $(ALL_SRCS:.py=.ice.d.mak)

all:: .depend\ice.depend.mak

.depend\ice.depend.mak: Makefile.mak
	@echo Creating Slice dependencies list
	@if not exist ".depend" mkdir .depend
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-list.vbs $(SLICE_DEPENDS) > .depend\ice.depend.mak

!if exist(.depend\ice.depend.mak)
!include .depend\ice.depend.mak
!endif

clean::
	del /q .depend\*.d.mak

PACKAGES	= IceBox IceGrid IcePatch2 IceStorm IceMX

SLICE2PYFLAGS   = $(SLICE2PYFLAGS) --ice

all:: $(ALL_SRCS)

Ice_LocalException_ice.py: "$(slicedir)/Ice/LocalException.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/LocalException.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/LocalException.ice" > .depend/$(*F).ice.d.mak

Ice_Communicator_ice.py: "$(slicedir)/Ice/Communicator.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Communicator.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/Communicator.ice" > .depend/$(*F).ice.d.mak

Ice_CommunicatorF_ice.py: "$(slicedir)/Ice/CommunicatorF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/CommunicatorF.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/CommunicatorF.ice" > .depend/$(*F).ice.d.mak

Ice_Logger_ice.py: "$(slicedir)/Ice/Logger.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Logger.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/Logger.ice" > .depend/$(*F).ice.d.mak

Ice_LoggerF_ice.py: "$(slicedir)/Ice/LoggerF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/LoggerF.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/LoggerF.ice" > .depend/$(*F).ice.d.mak

Ice_BuiltinSequences_ice.py: "$(slicedir)/Ice/BuiltinSequences.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/BuiltinSequences.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/BuiltinSequences.ice" > .depend/$(*F).ice.d.mak

Ice_ObjectAdapter_ice.py: "$(slicedir)/Ice/ObjectAdapter.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ObjectAdapter.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/ObjectAdapter.ice" > .depend/$(*F).ice.d.mak

Ice_ObjectAdapterF_ice.py: "$(slicedir)/Ice/ObjectAdapterF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ObjectAdapterF.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/ObjectAdapterF.ice" > .depend/$(*F).ice.d.mak

Ice_ServantLocator_ice.py: "$(slicedir)/Ice/ServantLocator.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ServantLocator.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/ServantLocator.ice" > .depend/$(*F).ice.d.mak

Ice_ServantLocatorF_ice.py: "$(slicedir)/Ice/ServantLocatorF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ServantLocatorF.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/ServantLocatorF.ice" > .depend/$(*F).ice.d.mak

Ice_PropertiesAdmin_ice.py: "$(slicedir)/Ice/PropertiesAdmin.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/PropertiesAdmin.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/PropertiesAdmin.ice" > .depend/$(*F).ice.d.mak

Ice_Properties_ice.py: "$(slicedir)/Ice/Properties.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Properties.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/Properties.ice" > .depend/$(*F).ice.d.mak

Ice_PropertiesF_ice.py: "$(slicedir)/Ice/PropertiesF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/PropertiesF.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/PropertiesF.ice" > .depend/$(*F).ice.d.mak

Ice_ObjectFactory_ice.py: "$(slicedir)/Ice/ObjectFactory.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ObjectFactory.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/ObjectFactory.ice" > .depend/$(*F).ice.d.mak

Ice_ObjectFactoryF_ice.py: "$(slicedir)/Ice/ObjectFactoryF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ObjectFactoryF.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/ObjectFactoryF.ice" > .depend/$(*F).ice.d.mak

Ice_Identity_ice.py: "$(slicedir)/Ice/Identity.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Identity.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/Identity.ice" > .depend/$(*F).ice.d.mak

Ice_Current_ice.py: "$(slicedir)/Ice/Current.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Current.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/Current.ice" > .depend/$(*F).ice.d.mak

Ice_ImplicitContext_ice.py: "$(slicedir)/Ice/ImplicitContext.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ImplicitContext.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/ImplicitContext.ice" > .depend/$(*F).ice.d.mak

Ice_ImplicitContextF_ice.py: "$(slicedir)/Ice/ImplicitContextF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ImplicitContextF.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/ImplicitContextF.ice" > .depend/$(*F).ice.d.mak

Ice_Router_ice.py: "$(slicedir)/Ice/Router.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Router.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/Router.ice" > .depend/$(*F).ice.d.mak

Ice_RouterF_ice.py: "$(slicedir)/Ice/RouterF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/RouterF.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/RouterF.ice" > .depend/$(*F).ice.d.mak

Ice_Plugin_ice.py: "$(slicedir)/Ice/Plugin.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Plugin.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/Plugin.ice" > .depend/$(*F).ice.d.mak

Ice_PluginF_ice.py: "$(slicedir)/Ice/PluginF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/PluginF.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/PluginF.ice" > .depend/$(*F).ice.d.mak

Ice_Locator_ice.py: "$(slicedir)/Ice/Locator.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Locator.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/Locator.ice" > .depend/$(*F).ice.d.mak

Ice_LocatorF_ice.py: "$(slicedir)/Ice/LocatorF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/LocatorF.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/LocatorF.ice" > .depend/$(*F).ice.d.mak

Ice_StatsF_ice.py: "$(slicedir)/Ice/StatsF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/StatsF.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/StatsF.ice" > .depend/$(*F).ice.d.mak

Ice_Stats_ice.py: "$(slicedir)/Ice/Stats.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Stats.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/Stats.ice" > .depend/$(*F).ice.d.mak

Ice_Process_ice.py: "$(slicedir)/Ice/Process.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Process.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/Process.ice" > .depend/$(*F).ice.d.mak

Ice_ProcessF_ice.py: "$(slicedir)/Ice/ProcessF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ProcessF.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/ProcessF.ice" > .depend/$(*F).ice.d.mak

Ice_FacetMap_ice.py: "$(slicedir)/Ice/FacetMap.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/FacetMap.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/FacetMap.ice" > .depend/$(*F).ice.d.mak

Ice_Connection_ice.py: "$(slicedir)/Ice/Connection.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Connection.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/Connection.ice" > .depend/$(*F).ice.d.mak

Ice_ConnectionF_ice.py: "$(slicedir)/Ice/ConnectionF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ConnectionF.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/ConnectionF.ice" > .depend/$(*F).ice.d.mak

Ice_SliceChecksumDict_ice.py: "$(slicedir)/Ice/SliceChecksumDict.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/SliceChecksumDict.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/SliceChecksumDict.ice" > .depend/$(*F).ice.d.mak

Ice_Endpoint_ice.py: "$(slicedir)/Ice/Endpoint.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Endpoint.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/Endpoint.ice" > .depend/$(*F).ice.d.mak

Ice_EndpointF_ice.py: "$(slicedir)/Ice/EndpointF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/EndpointF.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/EndpointF.ice" > .depend/$(*F).ice.d.mak

Ice_EndpointTypes_ice.py: "$(slicedir)/Ice/EndpointTypes.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/EndpointTypes.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/EndpointTypes.ice" > .depend/$(*F).ice.d.mak

Ice_Version_ice.py: "$(slicedir)/Ice/Version.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Version.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/Version.ice" > .depend/$(*F).ice.d.mak

Ice_Instrumentation_ice.py: "$(slicedir)/Ice/Instrumentation.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Instrumentation.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/Instrumentation.ice" > .depend/$(*F).ice.d.mak

Ice_InstrumentationF_ice.py: "$(slicedir)/Ice/InstrumentationF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/InstrumentationF.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/InstrumentationF.ice" > .depend/$(*F).ice.d.mak

Ice_Metrics_ice.py: "$(slicedir)/Ice/Metrics.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package --checksum "$(slicedir)/Ice/Metrics.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Ice/Metrics.ice" > .depend/$(*F).ice.d.mak

Glacier2_RouterF_ice.py: "$(slicedir)/Glacier2/RouterF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/RouterF.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Glacier2_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Glacier2/RouterF.ice" > .depend/$(*F).ice.d.mak

Glacier2_Router_ice.py: "$(slicedir)/Glacier2/Router.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/Router.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Glacier2_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Glacier2/Router.ice" > .depend/$(*F).ice.d.mak

Glacier2_Session_ice.py: "$(slicedir)/Glacier2/Session.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/Session.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Glacier2_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Glacier2/Session.ice" > .depend/$(*F).ice.d.mak

Glacier2_PermissionsVerifierF_ice.py: "$(slicedir)/Glacier2/PermissionsVerifierF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/PermissionsVerifierF.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Glacier2_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Glacier2/PermissionsVerifierF.ice" > .depend/$(*F).ice.d.mak

Glacier2_PermissionsVerifier_ice.py: "$(slicedir)/Glacier2/PermissionsVerifier.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/PermissionsVerifier.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Glacier2_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Glacier2/PermissionsVerifier.ice" > .depend/$(*F).ice.d.mak

Glacier2_SSLInfo_ice.py: "$(slicedir)/Glacier2/SSLInfo.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/SSLInfo.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Glacier2_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Glacier2/SSLInfo.ice" > .depend/$(*F).ice.d.mak

Glacier2_Metrics_ice.py: "$(slicedir)/Glacier2/Metrics.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/Metrics.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix Glacier2_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/Glacier2/Metrics.ice" > .depend/$(*F).ice.d.mak

IceBox_IceBox_ice.py: "$(slicedir)/IceBox/IceBox.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceBox_ --checksum "$(slicedir)/IceBox/IceBox.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix IceBox_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/IceBox/IceBox.ice" > .depend/$(*F).ice.d.mak

IceGrid_Admin_ice.py: "$(slicedir)/IceGrid/Admin.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Admin.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix IceGrid_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/IceGrid/Admin.ice" > .depend/$(*F).ice.d.mak

IceGrid_Descriptor_ice.py: "$(slicedir)/IceGrid/Descriptor.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Descriptor.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix IceGrid_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/IceGrid/Descriptor.ice" > .depend/$(*F).ice.d.mak

IceGrid_Exception_ice.py: "$(slicedir)/IceGrid/Exception.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Exception.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix IceGrid_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/IceGrid/Exception.ice" > .depend/$(*F).ice.d.mak

IceGrid_FileParser_ice.py: "$(slicedir)/IceGrid/FileParser.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/FileParser.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix IceGrid_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/IceGrid/FileParser.ice" > .depend/$(*F).ice.d.mak

IceGrid_Locator_ice.py: "$(slicedir)/IceGrid/Locator.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Locator.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix IceGrid_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/IceGrid/Locator.ice" > .depend/$(*F).ice.d.mak

IceGrid_Observer_ice.py: "$(slicedir)/IceGrid/Observer.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Observer.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix IceGrid_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/IceGrid/Observer.ice" > .depend/$(*F).ice.d.mak

IceGrid_Query_ice.py: "$(slicedir)/IceGrid/Query.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Query.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix IceGrid_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/IceGrid/Query.ice" > .depend/$(*F).ice.d.mak

IceGrid_Registry_ice.py: "$(slicedir)/IceGrid/Registry.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Registry.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix IceGrid_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/IceGrid/Registry.ice" > .depend/$(*F).ice.d.mak

IceGrid_Session_ice.py: "$(slicedir)/IceGrid/Session.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Session.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix IceGrid_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/IceGrid/Session.ice" > .depend/$(*F).ice.d.mak

IceGrid_UserAccountMapper_ice.py: "$(slicedir)/IceGrid/UserAccountMapper.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/UserAccountMapper.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix IceGrid_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/IceGrid/UserAccountMapper.ice" > .depend/$(*F).ice.d.mak

IcePatch2_FileInfo_ice.py: "$(slicedir)/IcePatch2/FileInfo.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IcePatch2_ --checksum "$(slicedir)/IcePatch2/FileInfo.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix IceGrid_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/IcePatch2/FileInfo.ice" > .depend/$(*F).ice.d.mak

IcePatch2_FileServer_ice.py: "$(slicedir)/IcePatch2/FileServer.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IcePatch2_ --checksum "$(slicedir)/IcePatch2/FileServer.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix IceGrid_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/IcePatch2/FileServer.ice" > .depend/$(*F).ice.d.mak

IceStorm_IceStorm_ice.py: "$(slicedir)/IceStorm/IceStorm.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceStorm_ --checksum "$(slicedir)/IceStorm/IceStorm.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix IceGrid_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/IceStorm/IceStorm.ice" > .depend/$(*F).ice.d.mak

IceStorm_Metrics_ice.py: "$(slicedir)/IceStorm/Metrics.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceStorm_ --checksum "$(slicedir)/IceStorm/Metrics.ice"
	@if not exist ".depend" mkdir .depend
	@"$(SLICE2PY)" --prefix IceGrid_ --no-package $(SLICE2PYFLAGS) --depend "$(slicedir)/IceStorm/Metrics.ice" > .depend/$(*F).ice.d.mak

install:: $(ALL_SRCS)
	@echo "Installing generated code"
	copy *.py "$(install_pythondir)"
	@for %i in ( $(PACKAGES) ) do \
		@if not exist "$(install_pythondir)\%i" \
			mkdir "$(install_pythondir)\%i"
	@for %i in ( $(PACKAGES) ) do \
		copy %i\* "$(install_pythondir)\%i"

clean::
	-rmdir /s /q $(PACKAGES)
	del /q *_ice.py
