# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

!include $(top_srcdir)\config\Make.rules.mak

#
# IMPORTANT: If you add or remove Slice files, you also need to check Ice.rb!
#
ICE_SRCS	= Ice\LocalException.rb \
		  Ice\Communicator.rb \
		  Ice\CommunicatorF.rb \
		  Ice\Logger.rb \
		  Ice\LoggerF.rb \
		  Ice\BuiltinSequences.rb \
		  Ice\ObjectAdapterF.rb \
		  Ice\Properties.rb \
		  Ice\PropertiesF.rb \
		  Ice\ObjectFactory.rb \
		  Ice\ObjectFactoryF.rb \
		  Ice\Identity.rb \
		  Ice\Current.rb \
		  Ice\ImplicitContextF.rb \
		  Ice\ImplicitContext.rb \
		  Ice\Router.rb \
		  Ice\RouterF.rb \
		  Ice\Plugin.rb \
		  Ice\PluginF.rb \
		  Ice\Locator.rb \
		  Ice\LocatorF.rb \
		  Ice\StatsF.rb \
		  Ice\Stats.rb \
		  Ice\Process.rb \
		  Ice\ProcessF.rb \
		  Ice\FacetMap.rb \
		  Ice\Connection.rb \
		  Ice\ConnectionF.rb \
		  Ice\SliceChecksumDict.rb \
		  Ice\Endpoint.rb

#
# IMPORTANT: If you add or remove Slice files, you also need to check Glacier2.rb!
#
GLACIER2_SRCS	= Glacier2\RouterF.rb \
		  Glacier2\Router.rb \
		  Glacier2\SessionF.rb \
		  Glacier2\Session.rb \
		  Glacier2\PermissionsVerifierF.rb \
		  Glacier2\PermissionsVerifier.rb \
		  Glacier2\SSLInfo.rb

#
# IMPORTANT: If you add or remove Slice files, you also need to check IceBox.rb!
#
ICEBOX_SRCS	= IceBox\IceBox.rb

#
# IMPORTANT: If you add or remove Slice files, you also need to check IceGrid.rb!
#
ICEGRID_SRCS	= IceGrid\Admin.rb \
		  IceGrid\Descriptor.rb \
		  IceGrid\Exception.rb \
		  IceGrid\FileParser.rb \
		  IceGrid\Locator.rb \
		  IceGrid\Observer.rb \
		  IceGrid\Query.rb \
		  IceGrid\Registry.rb \
		  IceGrid\Session.rb \
		  IceGrid\UserAccountMapper.rb

#
# IMPORTANT: If you add or remove Slice files, you also need to check IcePatch2.rb!
#
ICEPATCH2_SRCS	= IcePatch2\FileInfo.rb \
		  IcePatch2\FileServer.rb

#
# IMPORTANT: If you add or remove Slice files, you also need to check IceStorm.rb!
#
ICESTORM_SRCS	= IceStorm\IceStorm.rb

ALL_SRCS	= $(ICE_SRCS) \
		  $(GLACIER2_SRCS) \
		  $(ICEBOX_SRCS) \
		  $(ICEGRID_SRCS) \
		  $(ICEPATCH2_SRCS) \
		  $(ICESTORM_SRCS)

MODULES		= Glacier2 Ice IceBox IceGrid IcePatch2 IceStorm

SLICE2RBFLAGS	= -I$(slicedir) --ice

all:: $(ALL_SRCS)

$(MODULES):
	-mkdir $@

$(ALL_SRCS): $(MODULES) {$(slicedir)}$*.ice
	-$(SLICE2RB) $(SLICE2RBFLAGS) --output-dir $(*D) $(slicedir)\$*.ice


install::
	@echo "Installing generated code"
	copy *.rb $(install_rubydir)
	@for %i in ( $(MODULES) ) do \
	    @if not exist $(install_rubydir)\%i \
	        mkdir $(install_rubydir)\%i
	@for %i in ( $(MODULES) ) do \
	    copy %i\* $(install_rubydir)\%i

clean::
	-rmdir /S /Q $(MODULES)
