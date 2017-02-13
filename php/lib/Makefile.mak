# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

!include $(top_srcdir)\config\Make.rules.mak.php

#
# IMPORTANT: If you add or remove Slice files, you also need to check Ice.php!
#
ICE_SRCS	= Ice\LocalException.php \
		  Ice\Communicator.php \
		  Ice\CommunicatorF.php \
		  Ice\Logger.php \
		  Ice\LoggerF.php \
		  Ice\BuiltinSequences.php \
		  Ice\ObjectAdapterF.php \
		  Ice\PropertiesAdmin.php \
		  Ice\Properties.php \
		  Ice\PropertiesF.php \
		  Ice\ObjectFactory.php \
		  Ice\ObjectFactoryF.php \
		  Ice\Identity.php \
		  Ice\Current.php \
		  Ice\ImplicitContextF.php \
		  Ice\ImplicitContext.php \
		  Ice\RemoteLogger.php \
		  Ice\Router.php \
		  Ice\RouterF.php \
		  Ice\Plugin.php \
		  Ice\PluginF.php \
		  Ice\Locator.php \
		  Ice\LocatorF.php \
		  Ice\Process.php \
		  Ice\ProcessF.php \
		  Ice\FacetMap.php \
		  Ice\Connection.php \
		  Ice\ConnectionF.php \
		  Ice\SliceChecksumDict.php \
		  Ice\Endpoint.php \
		  Ice\EndpointF.php \
		  Ice\EndpointTypes.php \
		  Ice\Version.php \
		  Ice\InstrumentationF.php \
		  Ice\Instrumentation.php \
		  Ice\Metrics.php

#
# IMPORTANT: If you add or remove Slice files, you also need to check Glacier2.php!
#
GLACIER2_SRCS	= Glacier2\RouterF.php \
		  Glacier2\Router.php \
		  Glacier2\Session.php \
		  Glacier2\PermissionsVerifierF.php \
		  Glacier2\PermissionsVerifier.php \
		  Glacier2\SSLInfo.php \
		  Glacier2\Metrics.php

#
# IMPORTANT: If you add or remove Slice files, you also need to check IceBox.php!
#
ICEBOX_SRCS	= IceBox\IceBox.php

#
# IMPORTANT: If you add or remove Slice files, you also need to check IceGrid.php!
#
ICEGRID_SRCS	= IceGrid\Admin.php \
		  IceGrid\Descriptor.php \
		  IceGrid\Exception.php \
		  IceGrid\FileParser.php \
		  IceGrid\Locator.php \
		  IceGrid\Observer.php \
		  IceGrid\Query.php \
		  IceGrid\Registry.php \
		  IceGrid\Session.php \
		  IceGrid\UserAccountMapper.php

#
# IMPORTANT: If you add or remove Slice files, you also need to check IcePatch2.php!
#
ICEPATCH2_SRCS	= IcePatch2\FileInfo.php \
		  IcePatch2\FileServer.php

#
# IMPORTANT: If you add or remove Slice files, you also need to check IceStorm.php!
#
ICESTORM_SRCS	= IceStorm\IceStorm.php \
		  IceStorm\Metrics.php

ALL_SRCS	= $(ICE_SRCS) \
		  $(GLACIER2_SRCS) \
		  $(ICEBOX_SRCS) \
		  $(ICEGRID_SRCS) \
		  $(ICEPATCH2_SRCS) \
		  $(ICESTORM_SRCS)

DEPENDS		= $(ALL_SRCS:.php=.d)

MODULES		= Glacier2 Ice IceBox IceGrid IcePatch2 IceStorm
!if "$(USE_NAMESPACES)" == "yes"
MODULE_SRCS	= Glacier2.php Ice_ns.php IceBox.php IceGrid.php IcePatch2.php IceStorm.php
!else
MODULE_SRCS	= Glacier2.php Ice.php IceBox.php IceGrid.php IcePatch2.php IceStorm.php
!endif

SLICE2PHPFLAGS	= $(SLICE2PHPFLAGS) --ice

all:: $(ALL_SRCS)

$(MODULES):
	-$(MKDIR) $@

$(ALL_SRCS): $(MODULES) {$(slicedir)}$*.ice "$(SLICE2PHP)" "$(SLICEPARSERLIB)"
	-"$(SLICE2PHP)" $(SLICE2PHPFLAGS) --output-dir $(*D) "$(slicedir)\$*.ice"

{$(slicedir)\Ice\}.ice{Ice\}.d:
	@echo Generating dependencies for $<
	@"$(SLICE2PHP)" $(SLICE2PHPFLAGS) --output-dir Ice --depend "$<" | \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-slice.vbs $(*F).ice

{$(slicedir)\Glacier2\}.ice{Glacier2\}.d:
	@echo Generating dependencies for $<
	@"$(SLICE2PHP)" $(SLICE2PHPFLAGS) --output-dir Glacier2 --depend "$<" | \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-slice.vbs $(*F).ice

{$(slicedir)\IceBox\}.ice{IceBox\}.d:
	@echo Generating dependencies for $<
	@"$(SLICE2PHP)" $(SLICE2PHPFLAGS) --output-dir IceBox --depend "$<" | \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-slice.vbs $(*F).ice

{$(slicedir)\IceGrid\}.ice{IceGrid\}.d:
	@echo Generating dependencies for $<
	@"$(SLICE2PHP)" $(SLICE2PHPFLAGS) --output-dir IceGrid --depend "$<" | \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-slice.vbs $(*F).ice

{$(slicedir)\IcePatch2\}.ice{IcePatch2\}.d:
	@echo Generating dependencies for $<
	@"$(SLICE2PHP)" $(SLICE2PHPFLAGS) --output-dir IcePatch2 --depend "$<" | \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-slice.vbs $(*F).ice

{$(slicedir)\IceStorm\}.ice{IceStorm\}.d:
	@echo Generating dependencies for $<
	@"$(SLICE2PHP)" $(SLICE2PHPFLAGS) --output-dir IceStorm --depend "$<" | \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-slice.vbs $(*F).ice

depend:: $(DEPENDS)

install:: $(ALL_SRCS)
	@echo "Installing generated code"
	@for %i in ( $(MODULES) ) do \
	    @if not exist "$(install_phpdir)\%i" \
	        $(MKDIR) "$(install_phpdir)\%i"
	@for %i in ( $(MODULES) ) do \
	    copy %i\* "$(install_phpdir)\%i"
	@for %i in ( $(MODULE_SRCS) ) do \
	    copy %i "$(install_phpdir)"

clean::
	-rmdir /S /Q $(MODULES)
