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

PACKAGES	= IceBox IceGrid IcePatch2 IceStorm IceMX

SLICE2PYFLAGS   = $(SLICE2PYFLAGS) --ice

all:: $(ALL_SRCS)

Ice_LocalException_ice.py: "$(slicedir)/Ice/LocalException.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/LocalException.ice"

Ice_Communicator_ice.py: "$(slicedir)/Ice/Communicator.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Communicator.ice"

Ice_CommunicatorF_ice.py: "$(slicedir)/Ice/CommunicatorF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/CommunicatorF.ice"

Ice_Logger_ice.py: "$(slicedir)/Ice/Logger.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Logger.ice"

Ice_LoggerF_ice.py: "$(slicedir)/Ice/LoggerF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/LoggerF.ice"

Ice_BuiltinSequences_ice.py: "$(slicedir)/Ice/BuiltinSequences.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/BuiltinSequences.ice"

Ice_ObjectAdapter_ice.py: "$(slicedir)/Ice/ObjectAdapter.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ObjectAdapter.ice"

Ice_ObjectAdapterF_ice.py: "$(slicedir)/Ice/ObjectAdapterF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ObjectAdapterF.ice"

Ice_ServantLocator_ice.py: "$(slicedir)/Ice/ServantLocator.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ServantLocator.ice"

Ice_ServantLocatorF_ice.py: "$(slicedir)/Ice/ServantLocatorF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ServantLocatorF.ice"

Ice_PropertiesAdmin_ice.py: "$(slicedir)/Ice/PropertiesAdmin.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/PropertiesAdmin.ice"

Ice_Properties_ice.py: "$(slicedir)/Ice/Properties.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Properties.ice"

Ice_PropertiesF_ice.py: "$(slicedir)/Ice/PropertiesF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/PropertiesF.ice"

Ice_ObjectFactory_ice.py: "$(slicedir)/Ice/ObjectFactory.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ObjectFactory.ice"

Ice_ObjectFactoryF_ice.py: "$(slicedir)/Ice/ObjectFactoryF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ObjectFactoryF.ice"

Ice_Identity_ice.py: "$(slicedir)/Ice/Identity.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Identity.ice"

Ice_Current_ice.py: "$(slicedir)/Ice/Current.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Current.ice"

Ice_ImplicitContext_ice.py: "$(slicedir)/Ice/ImplicitContext.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ImplicitContext.ice"

Ice_ImplicitContextF_ice.py: "$(slicedir)/Ice/ImplicitContextF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ImplicitContextF.ice"

Ice_Router_ice.py: "$(slicedir)/Ice/Router.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Router.ice"

Ice_RouterF_ice.py: "$(slicedir)/Ice/RouterF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/RouterF.ice"

Ice_Plugin_ice.py: "$(slicedir)/Ice/Plugin.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Plugin.ice"

Ice_PluginF_ice.py: "$(slicedir)/Ice/PluginF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/PluginF.ice"

Ice_Locator_ice.py: "$(slicedir)/Ice/Locator.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Locator.ice"

Ice_LocatorF_ice.py: "$(slicedir)/Ice/LocatorF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/LocatorF.ice"

Ice_StatsF_ice.py: "$(slicedir)/Ice/StatsF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/StatsF.ice"

Ice_Stats_ice.py: "$(slicedir)/Ice/Stats.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Stats.ice"

Ice_Process_ice.py: "$(slicedir)/Ice/Process.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Process.ice"

Ice_ProcessF_ice.py: "$(slicedir)/Ice/ProcessF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ProcessF.ice"

Ice_FacetMap_ice.py: "$(slicedir)/Ice/FacetMap.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/FacetMap.ice"

Ice_Connection_ice.py: "$(slicedir)/Ice/Connection.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Connection.ice"

Ice_ConnectionF_ice.py: "$(slicedir)/Ice/ConnectionF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ConnectionF.ice"

Ice_SliceChecksumDict_ice.py: "$(slicedir)/Ice/SliceChecksumDict.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/SliceChecksumDict.ice"

Ice_Endpoint_ice.py: "$(slicedir)/Ice/Endpoint.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Endpoint.ice"

Ice_EndpointF_ice.py: "$(slicedir)/Ice/EndpointF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/EndpointF.ice"

Ice_EndpointTypes_ice.py: "$(slicedir)/Ice/EndpointTypes.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/EndpointTypes.ice"

Ice_Version_ice.py: "$(slicedir)/Ice/Version.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Version.ice"

Ice_Instrumentation_ice.py: "$(slicedir)/Ice/Instrumentation.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Instrumentation.ice"

Ice_InstrumentationF_ice.py: "$(slicedir)/Ice/InstrumentationF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/InstrumentationF.ice"

Ice_Metrics_ice.py: "$(slicedir)/Ice/Metrics.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Ice_ --no-package --checksum "$(slicedir)/Ice/Metrics.ice"


Glacier2_RouterF_ice.py: "$(slicedir)/Glacier2/RouterF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/RouterF.ice"

Glacier2_Router_ice.py: "$(slicedir)/Glacier2/Router.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/Router.ice"

Glacier2_Session_ice.py: "$(slicedir)/Glacier2/Session.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/Session.ice"

Glacier2_PermissionsVerifierF_ice.py: "$(slicedir)/Glacier2/PermissionsVerifierF.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/PermissionsVerifierF.ice"

Glacier2_PermissionsVerifier_ice.py: "$(slicedir)/Glacier2/PermissionsVerifier.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/PermissionsVerifier.ice"

Glacier2_SSLInfo_ice.py: "$(slicedir)/Glacier2/SSLInfo.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/SSLInfo.ice"

Glacier2_Metrics_ice.py: "$(slicedir)/Glacier2/Metrics.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/Metrics.ice"


IceBox_IceBox_ice.py: "$(slicedir)/IceBox/IceBox.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceBox_ --checksum "$(slicedir)/IceBox/IceBox.ice"


IceGrid_Admin_ice.py: "$(slicedir)/IceGrid/Admin.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Admin.ice"

IceGrid_Descriptor_ice.py: "$(slicedir)/IceGrid/Descriptor.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Descriptor.ice"

IceGrid_Exception_ice.py: "$(slicedir)/IceGrid/Exception.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Exception.ice"

IceGrid_FileParser_ice.py: "$(slicedir)/IceGrid/FileParser.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/FileParser.ice"

IceGrid_Locator_ice.py: "$(slicedir)/IceGrid/Locator.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Locator.ice"

IceGrid_Observer_ice.py: "$(slicedir)/IceGrid/Observer.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Observer.ice"

IceGrid_Query_ice.py: "$(slicedir)/IceGrid/Query.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Query.ice"

IceGrid_Registry_ice.py: "$(slicedir)/IceGrid/Registry.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Registry.ice"

IceGrid_Session_ice.py: "$(slicedir)/IceGrid/Session.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Session.ice"

IceGrid_UserAccountMapper_ice.py: "$(slicedir)/IceGrid/UserAccountMapper.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/UserAccountMapper.ice"


IcePatch2_FileInfo_ice.py: "$(slicedir)/IcePatch2/FileInfo.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IcePatch2_ --checksum "$(slicedir)/IcePatch2/FileInfo.ice"

IcePatch2_FileServer_ice.py: "$(slicedir)/IcePatch2/FileServer.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IcePatch2_ --checksum "$(slicedir)/IcePatch2/FileServer.ice"


IceStorm_IceStorm_ice.py: "$(slicedir)/IceStorm/IceStorm.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceStorm_ --checksum "$(slicedir)/IceStorm/IceStorm.ice"

IceStorm_Metrics_ice.py: "$(slicedir)/IceStorm/Metrics.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) --prefix IceStorm_ --checksum "$(slicedir)/IceStorm/Metrics.ice"


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

include .depend.mak
