# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

include $(top_srcdir)/config/Make.rules

#
# IMPORTANT: If you add or remove Slice files, you also need to check Ice.rb!
#
ICE_SRCS	= Ice/LocalException.rb \
		  Ice/Communicator.rb \
		  Ice/CommunicatorF.rb \
		  Ice/Logger.rb \
		  Ice/LoggerF.rb \
		  Ice/BuiltinSequences.rb \
		  Ice/ObjectAdapterF.rb \
		  Ice/Properties.rb \
		  Ice/PropertiesF.rb \
		  Ice/ObjectFactory.rb \
		  Ice/ObjectFactoryF.rb \
		  Ice/Identity.rb \
		  Ice/Current.rb \
		  Ice/ImplicitContextF.rb \
		  Ice/ImplicitContext.rb \
		  Ice/Router.rb \
		  Ice/RouterF.rb \
		  Ice/Plugin.rb \
		  Ice/PluginF.rb \
		  Ice/Locator.rb \
		  Ice/LocatorF.rb \
		  Ice/StatsF.rb \
		  Ice/Stats.rb \
		  Ice/Process.rb \
		  Ice/ProcessF.rb \
		  Ice/FacetMap.rb \
		  Ice/Connection.rb \
		  Ice/ConnectionF.rb \
		  Ice/SliceChecksumDict.rb \
		  Ice/Endpoint.rb

#
# IMPORTANT: If you add or remove Slice files, you also need to check Glacier2.rb!
#
GLACIER2_SRCS	= Glacier2/RouterF.rb \
		  Glacier2/Router.rb \
		  Glacier2/SessionF.rb \
		  Glacier2/Session.rb \
		  Glacier2/PermissionsVerifierF.rb \
		  Glacier2/PermissionsVerifier.rb \
		  Glacier2/SSLInfo.rb

#
# IMPORTANT: If you add or remove Slice files, you also need to check IceBox.rb!
#
ICEBOX_SRCS	= IceBox/IceBox.rb

#
# IMPORTANT: If you add or remove Slice files, you also need to check IceGrid.rb!
#
ICEGRID_SRCS	= IceGrid/Admin.rb \
		  IceGrid/Descriptor.rb \
		  IceGrid/Exception.rb \
		  IceGrid/FileParser.rb \
		  IceGrid/Locator.rb \
		  IceGrid/Observer.rb \
		  IceGrid/Query.rb \
		  IceGrid/Registry.rb \
		  IceGrid/Session.rb \
		  IceGrid/UserAccountMapper.rb

#
# IMPORTANT: If you add or remove Slice files, you also need to check IcePatch2.rb!
#
ICEPATCH2_SRCS	= IcePatch2/FileInfo.rb \
		  IcePatch2/FileServer.rb

#
# IMPORTANT: If you add or remove Slice files, you also need to check IceStorm.rb!
#
ICESTORM_SRCS	= IceStorm/IceStorm.rb

ALL_SRCS	= $(ICE_SRCS) \
		  $(GLACIER2_SRCS) \
		  $(ICEBOX_SRCS) \
		  $(ICEGRID_SRCS) \
		  $(ICEPATCH2_SRCS) \
		  $(ICESTORM_SRCS)

MODULES		= Glacier2 Ice IceBox IceGrid IcePatch2 IceStorm

all:: $(ALL_SRCS)

Ice/%.rb: $(slicedir)/Ice/%.ice
	@mkdir -p $(notdir $(<D))
	$(SLICE2RB) --output-dir $(notdir $(<D)) --ice $(SLICE2RBFLAGS) $<

Glacier2/%.rb: $(slicedir)/Glacier2/%.ice
	@mkdir -p $(notdir $(<D))
	$(SLICE2RB) --output-dir $(notdir $(<D)) --ice $(SLICE2RBFLAGS) $<

IceBox/%.rb: $(slicedir)/IceBox/%.ice
	@mkdir -p $(notdir $(<D))
	$(SLICE2RB) --output-dir $(notdir $(<D)) --ice $(SLICE2RBFLAGS) $<

IceGrid/%.rb: $(slicedir)/IceGrid/%.ice
	@mkdir -p $(notdir $(<D))
	$(SLICE2RB) --output-dir $(notdir $(<D)) --ice $(SLICE2RBFLAGS) $<

IcePatch2/%.rb: $(slicedir)/IcePatch2/%.ice
	@mkdir -p $(notdir $(<D))
	$(SLICE2RB) --output-dir $(notdir $(<D)) --ice $(SLICE2RBFLAGS) $<

IceStorm/%.rb: $(slicedir)/IceStorm/%.ice
	@mkdir -p $(notdir $(<D))
	$(SLICE2RB) --output-dir $(notdir $(<D)) --ice $(SLICE2RBFLAGS) $<

install:: $(ALL_SRCS)
	@echo "Installing generated code"
	@for i in $(MODULES) ; \
	do \
	    $(INSTALL_DATA) $${i}.rb $(install_rubydir) ; \
	    $(INSTALL_DATA) -r $$i $(install_rubydir) ; \
	done

clean::
	rm -rf $(MODULES)
