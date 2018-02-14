# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

!include $(top_srcdir)\config\Make.rules.mak

ICE_SDIR	= Ice

ICE_SLICES	= Ice\BuiltinSequences.ice \
		  Ice\Communicator.ice \
		  Ice\CommunicatorF.ice \
		  Ice\Connection.ice \
		  Ice\ConnectionF.ice \
		  Ice\Current.ice \
		  Ice\Endpoint.ice \
		  Ice\EndpointF.ice \
		  Ice\EndpointTypes.ice \
		  Ice\FacetMap.ice \
		  Ice\Identity.ice \
		  Ice\ImplicitContext.ice \
		  Ice\ImplicitContextF.ice \
		  Ice\Instrumentation.ice \
		  Ice\InstrumentationF.ice \
		  Ice\LocalException.ice \
		  Ice\Locator.ice \
		  Ice\LocatorF.ice \
		  Ice\Logger.ice \
		  Ice\LoggerF.ice \
		  Ice\Metrics.ice \
		  Ice\ObjectAdapter.ice \
		  Ice\ObjectAdapterF.ice \
		  Ice\ObjectFactory.ice \
		  Ice\ObjectFactoryF.ice \
		  Ice\Plugin.ice \
		  Ice\PluginF.ice \
		  Ice\Process.ice \
		  Ice\ProcessF.ice \
		  Ice\Properties.ice \
		  Ice\PropertiesAdmin.ice \
		  Ice\PropertiesF.ice \
		  Ice\RemoteLogger.ice \
		  Ice\Router.ice \
		  Ice\RouterF.ice \
		  Ice\ServantLocator.ice \
		  Ice\ServantLocatorF.ice \
		  Ice\SliceChecksumDict.ice \
		  Ice\Version.ice \

ICE_SRCS = $(ICE_SLICES:.ice=_ice.py)
ICE_SRCS = $(ICE_SRCS:Ice\=Ice_)

GLACIER2_SDIR	= Glacier2

GLACIER2_SLICES = Glacier2\Metrics.ice \
		  Glacier2\PermissionsVerifier.ice \
		  Glacier2\PermissionsVerifierF.ice \
		  Glacier2\Router.ice \
		  Glacier2\RouterF.ice \
		  Glacier2\Session.ice \
		  Glacier2\SSLInfo.ice \

GLACIER2_SRCS	= $(GLACIER2_SLICES:.ice=_ice.py)
GLACIER2_SRCS	= $(GLACIER2_SRCS:Glacier2\=Glacier2_)

ICEBOX_SDIR	= IceBox

ICEBOX_SLICES	= IceBox\IceBox.ice
ICEBOX_SRCS	= $(ICEBOX_SLICES:.ice=_ice.py)
ICEBOX_SRCS	= $(ICEBOX_SRCS:IceBox\=IceBox_)

ICEGRID_SDIR	= IceGrid

ICEGRID_SLICES	= IceGrid\Admin.ice \
		  IceGrid\Descriptor.ice \
		  IceGrid\Exception.ice \
		  IceGrid\FileParser.ice \
		  IceGrid\Locator.ice \
		  IceGrid\Observer.ice \
		  IceGrid\Query.ice \
		  IceGrid\Registry.ice \
		  IceGrid\Session.ice \
		  IceGrid\UserAccountMapper.ice \

ICEGRID_SRCS	= $(ICEGRID_SLICES:.ice=_ice.py)
ICEGRID_SRCS	= $(ICEGRID_SRCS:IceGrid\=IceGrid_)

ICEPATCH2_SDIR	= IcePatch2

ICEPATCH2_SLICES = IcePatch2\FileInfo.ice \
		  IcePatch2\FileServer.ice

ICEPATCH2_SRCS	= $(ICEPATCH2_SLICES:.ice=_ice.py)
ICEPATCH2_SRCS	= $(ICEPATCH2_SRCS:IcePatch2\=IcePatch2_)

ICESTORM_SDIR	= IceStorm

ICESTORM_SLICES	= IceStorm\Metrics.ice \
		  IceStorm\IceStorm.ice

ICESTORM_SRCS	= $(ICESTORM_SLICES:.ice=_ice.py)
ICESTORM_SRCS	= $(ICESTORM_SRCS:IceStorm\=IceStorm_)

DEPENDS		= $(ICE_SLICES:.ice=.d) \
		  $(GLACIER2_SLICES:.ice=.d) \
		  $(ICEBOX_SLICES:.ice=.d) \
		  $(ICEGRID_SLICES:.ice=.d) \
		  $(ICEPATCH2_SLICES:.ice=.d) \
		  $(ICESTORM_SLICES:.ice=.d)

ALL_SRCS	= $(ICE_SRCS) \
		  $(GLACIER2_SRCS) \
		  $(ICEBOX_SRCS) \
		  $(ICEGRID_SRCS) \
		  $(ICEPATCH2_SRCS) \
		  $(ICESTORM_SRCS)

PACKAGES	= IceBox IceGrid IcePatch2 IceStorm IceMX

SLICE2PYFLAGS   = $(SLICE2PYFLAGS) --ice

all:: $(ALL_SRCS)

depend::
	del /q .depend.mak

{$(slicedir)\$(ICE_SDIR)\}.ice{$(ICE_SDIR)\}.d:
	@echo Generating dependencies for $<
	@$(SLICE2PY) --prefix Ice_ --no-package $(SLICE2PYFLAGS) --depend "$<" | \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-slice.vbs $(*F).ice

{$(slicedir)\$(GLACIER2_SDIR)\}.ice{$(GLACIER2_SDIR)\}.d:
	@echo Generating dependencies for $<
	@$(SLICE2PY) --prefix Glacier2_ --no-package $(SLICE2PYFLAGS) --depend "$<" | \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-slice.vbs $(*F).ice

{$(slicedir)\$(ICEBOX_SDIR)\}.ice{$(ICEBOX_SDIR)\}.d:
	@echo Generating dependencies for $<
	@$(SLICE2PY) --prefix IceBox_ --no-package $(SLICE2PYFLAGS) --depend "$<" | \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-slice.vbs $(*F).ice

{$(slicedir)\$(ICEGRID_SDIR)\}.ice{$(ICEGRID_SDIR)\}.d:
	@echo Generating dependencies for $<
	@$(SLICE2PY) --prefix IceGrid_ --no-package $(SLICE2PYFLAGS) --depend "$<" | \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-slice.vbs $(*F).ice

{$(slicedir)\$(ICEPATCH2_SDIR)\}.ice{$(ICEPATCH2_SDIR)\}.d:
	@echo Generating dependencies for $<
	@$(SLICE2PY) --prefix IcePatch2_ --no-package $(SLICE2PYFLAGS) --depend "$<" | \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-slice.vbs $(*F).ice

{$(slicedir)\$(ICESTORM_SDIR)\}.ice{$(ICESTORM_SDIR)\}.d:
	@echo Generating dependencies for $<
	@$(SLICE2PY) --prefix IceStorm_ --no-package $(SLICE2PYFLAGS) --depend "$<" | \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-slice.vbs $(*F).ice

depend:: $(DEPENDS)

Ice_LocalException_ice.py: "$(slicedir)/Ice/LocalException.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/LocalException.ice"

Ice_Communicator_ice.py: "$(slicedir)/Ice/Communicator.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Communicator.ice"

Ice_CommunicatorF_ice.py: "$(slicedir)/Ice/CommunicatorF.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/CommunicatorF.ice"

Ice_Logger_ice.py: "$(slicedir)/Ice/Logger.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Logger.ice"

Ice_LoggerF_ice.py: "$(slicedir)/Ice/LoggerF.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/LoggerF.ice"

Ice_BuiltinSequences_ice.py: "$(slicedir)/Ice/BuiltinSequences.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/BuiltinSequences.ice"

Ice_ObjectAdapter_ice.py: "$(slicedir)/Ice/ObjectAdapter.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ObjectAdapter.ice"

Ice_ObjectAdapterF_ice.py: "$(slicedir)/Ice/ObjectAdapterF.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ObjectAdapterF.ice"

Ice_ServantLocator_ice.py: "$(slicedir)/Ice/ServantLocator.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ServantLocator.ice"

Ice_ServantLocatorF_ice.py: "$(slicedir)/Ice/ServantLocatorF.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ServantLocatorF.ice"

Ice_PropertiesAdmin_ice.py: "$(slicedir)/Ice/PropertiesAdmin.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/PropertiesAdmin.ice"

Ice_Properties_ice.py: "$(slicedir)/Ice/Properties.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Properties.ice"

Ice_PropertiesF_ice.py: "$(slicedir)/Ice/PropertiesF.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/PropertiesF.ice"

Ice_ObjectFactory_ice.py: "$(slicedir)/Ice/ObjectFactory.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ObjectFactory.ice"

Ice_ObjectFactoryF_ice.py: "$(slicedir)/Ice/ObjectFactoryF.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ObjectFactoryF.ice"

Ice_Identity_ice.py: "$(slicedir)/Ice/Identity.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Identity.ice"

Ice_Current_ice.py: "$(slicedir)/Ice/Current.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Current.ice"

Ice_ImplicitContext_ice.py: "$(slicedir)/Ice/ImplicitContext.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ImplicitContext.ice"

Ice_ImplicitContextF_ice.py: "$(slicedir)/Ice/ImplicitContextF.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ImplicitContextF.ice"

Ice_RemoteLogger_ice.py: "$(slicedir)/Ice/RemoteLogger.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/RemoteLogger.ice"

Ice_Router_ice.py: "$(slicedir)/Ice/Router.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Router.ice"

Ice_RouterF_ice.py: "$(slicedir)/Ice/RouterF.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/RouterF.ice"

Ice_Plugin_ice.py: "$(slicedir)/Ice/Plugin.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Plugin.ice"

Ice_PluginF_ice.py: "$(slicedir)/Ice/PluginF.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/PluginF.ice"

Ice_Locator_ice.py: "$(slicedir)/Ice/Locator.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Locator.ice"

Ice_LocatorF_ice.py: "$(slicedir)/Ice/LocatorF.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/LocatorF.ice"

Ice_Process_ice.py: "$(slicedir)/Ice/Process.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Process.ice"

Ice_ProcessF_ice.py: "$(slicedir)/Ice/ProcessF.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ProcessF.ice"

Ice_FacetMap_ice.py: "$(slicedir)/Ice/FacetMap.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/FacetMap.ice"

Ice_Connection_ice.py: "$(slicedir)/Ice/Connection.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Connection.ice"

Ice_ConnectionF_ice.py: "$(slicedir)/Ice/ConnectionF.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/ConnectionF.ice"

Ice_SliceChecksumDict_ice.py: "$(slicedir)/Ice/SliceChecksumDict.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/SliceChecksumDict.ice"

Ice_Endpoint_ice.py: "$(slicedir)/Ice/Endpoint.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Endpoint.ice"

Ice_EndpointF_ice.py: "$(slicedir)/Ice/EndpointF.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/EndpointF.ice"

Ice_EndpointTypes_ice.py: "$(slicedir)/Ice/EndpointTypes.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/EndpointTypes.ice"

Ice_Version_ice.py: "$(slicedir)/Ice/Version.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Version.ice"

Ice_Instrumentation_ice.py: "$(slicedir)/Ice/Instrumentation.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/Instrumentation.ice"

Ice_InstrumentationF_ice.py: "$(slicedir)/Ice/InstrumentationF.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package "$(slicedir)/Ice/InstrumentationF.ice"

Ice_Metrics_ice.py: "$(slicedir)/Ice/Metrics.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Ice_ --no-package --checksum "$(slicedir)/Ice/Metrics.ice"

Glacier2_RouterF_ice.py: "$(slicedir)/Glacier2/RouterF.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/RouterF.ice"

Glacier2_Router_ice.py: "$(slicedir)/Glacier2/Router.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/Router.ice"

Glacier2_Session_ice.py: "$(slicedir)/Glacier2/Session.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/Session.ice"

Glacier2_PermissionsVerifierF_ice.py: "$(slicedir)/Glacier2/PermissionsVerifierF.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/PermissionsVerifierF.ice"

Glacier2_PermissionsVerifier_ice.py: "$(slicedir)/Glacier2/PermissionsVerifier.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/PermissionsVerifier.ice"

Glacier2_SSLInfo_ice.py: "$(slicedir)/Glacier2/SSLInfo.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/SSLInfo.ice"

Glacier2_Metrics_ice.py: "$(slicedir)/Glacier2/Metrics.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix Glacier2_ --no-package --checksum "$(slicedir)/Glacier2/Metrics.ice"

IceBox_IceBox_ice.py: "$(slicedir)/IceBox/IceBox.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix IceBox_ --checksum "$(slicedir)/IceBox/IceBox.ice"

IceGrid_Admin_ice.py: "$(slicedir)/IceGrid/Admin.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Admin.ice"

IceGrid_Descriptor_ice.py: "$(slicedir)/IceGrid/Descriptor.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Descriptor.ice"

IceGrid_Exception_ice.py: "$(slicedir)/IceGrid/Exception.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Exception.ice"

IceGrid_FileParser_ice.py: "$(slicedir)/IceGrid/FileParser.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/FileParser.ice"

IceGrid_Locator_ice.py: "$(slicedir)/IceGrid/Locator.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Locator.ice"

IceGrid_Observer_ice.py: "$(slicedir)/IceGrid/Observer.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Observer.ice"

IceGrid_Query_ice.py: "$(slicedir)/IceGrid/Query.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Query.ice"

IceGrid_Registry_ice.py: "$(slicedir)/IceGrid/Registry.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Registry.ice"

IceGrid_Session_ice.py: "$(slicedir)/IceGrid/Session.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/Session.ice"

IceGrid_UserAccountMapper_ice.py: "$(slicedir)/IceGrid/UserAccountMapper.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix IceGrid_ --checksum "$(slicedir)/IceGrid/UserAccountMapper.ice"

IcePatch2_FileInfo_ice.py: "$(slicedir)/IcePatch2/FileInfo.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix IcePatch2_ --checksum "$(slicedir)/IcePatch2/FileInfo.ice"

IcePatch2_FileServer_ice.py: "$(slicedir)/IcePatch2/FileServer.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix IcePatch2_ --checksum "$(slicedir)/IcePatch2/FileServer.ice"

IceStorm_IceStorm_ice.py: "$(slicedir)/IceStorm/IceStorm.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix IceStorm_ --checksum "$(slicedir)/IceStorm/IceStorm.ice"

IceStorm_Metrics_ice.py: "$(slicedir)/IceStorm/Metrics.ice" "$(SLICEPARSERLIB)"
	$(SLICE2PY) $(SLICE2PYFLAGS) --prefix IceStorm_ --checksum "$(slicedir)/IceStorm/Metrics.ice"

install:: $(ALL_SRCS)
	@echo "Installing generated code"
	copy *.py "$(install_pythondir)"
	@for %i in ( $(PACKAGES) ) do \
	    @if not exist "$(install_pythondir)\%i" \
	        $(MKDIR) "$(install_pythondir)\%i"
	@for %i in ( $(PACKAGES) ) do \
	    copy %i\* "$(install_pythondir)\%i"

clean::
	-rmdir /s /q $(PACKAGES)
	del /q *_ice.py

#include .depend.mak
