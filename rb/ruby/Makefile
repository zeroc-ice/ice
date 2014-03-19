# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
		  Ice/PropertiesAdmin.rb \
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
		  Ice/Endpoint.rb \
		  Ice/EndpointF.rb \
		  Ice/EndpointTypes.rb \
		  Ice/Version.rb \
		  Ice/InstrumentationF.rb \
		  Ice/Instrumentation.rb \
		  Ice/Metrics.rb

#
# IMPORTANT: If you add or remove Slice files, you also need to check Glacier2.rb!
#
GLACIER2_SRCS	= Glacier2/RouterF.rb \
		  Glacier2/Router.rb \
		  Glacier2/Session.rb \
		  Glacier2/PermissionsVerifierF.rb \
		  Glacier2/PermissionsVerifier.rb \
		  Glacier2/SSLInfo.rb \
		  Glacier2/Metrics.rb

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
ICESTORM_SRCS	= IceStorm/IceStorm.rb \
		  IceStorm/Metrics.rb

ALL_SRCS	= $(ICE_SRCS) \
		  $(GLACIER2_SRCS) \
		  $(ICEBOX_SRCS) \
		  $(ICEGRID_SRCS) \
		  $(ICEPATCH2_SRCS) \
		  $(ICESTORM_SRCS)

ICE_SLICE	= $(patsubst %.rb,$(slicedir)/%.ice,$(ICE_SRCS))
GLACIER2_SLICE	= $(patsubst %.rb,$(slicedir)/%.ice,$(GLACIER2_SRCS))
ICEBOX_SLICE	= $(patsubst %.rb,$(slicedir)/%.ice,$(ICEBOX_SRCS))
ICEGRID_SLICE	= $(patsubst %.rb,$(slicedir)/%.ice,$(ICEGRID_SRCS))
ICEPATCH2_SLICE	= $(patsubst %.rb,$(slicedir)/%.ice,$(ICEPATCH2_SRCS))
ICESTORM_SLICE	= $(patsubst %.rb,$(slicedir)/%.ice,$(ICESTORM_SRCS))

MODULES		= Glacier2 Ice IceBox IceGrid IcePatch2 IceStorm

all:: $(ALL_SRCS)

SLICE2RBFLAGS	+= --ice

Ice/%.rb: $(slicedir)/Ice/%.ice $(SLICE2RB) $(SLICEPARSERLIB)
	@mkdir -p $(notdir $(<D))
	$(SLICE2RB) --output-dir $(notdir $(<D)) $(SLICE2RBFLAGS) $<

Glacier2/%.rb: $(slicedir)/Glacier2/%.ice $(SLICE2RB) $(SLICEPARSERLIB)
	@mkdir -p $(notdir $(<D))
	$(SLICE2RB) --output-dir $(notdir $(<D)) $(SLICE2RBFLAGS) $<

IceBox/%.rb: $(slicedir)/IceBox/%.ice $(SLICE2RB) $(SLICEPARSERLIB)
	@mkdir -p $(notdir $(<D))
	$(SLICE2RB) --output-dir $(notdir $(<D)) $(SLICE2RBFLAGS) $<

IceGrid/%.rb: $(slicedir)/IceGrid/%.ice $(SLICE2RB) $(SLICEPARSERLIB)
	@mkdir -p $(notdir $(<D))
	$(SLICE2RB) --output-dir $(notdir $(<D)) $(SLICE2RBFLAGS) $<

IcePatch2/%.rb: $(slicedir)/IcePatch2/%.ice $(SLICE2RB) $(SLICEPARSERLIB)
	@mkdir -p $(notdir $(<D))
	$(SLICE2RB) --output-dir $(notdir $(<D)) $(SLICE2RBFLAGS) $<

IceStorm/%.rb: $(slicedir)/IceStorm/%.ice $(SLICE2RB) $(SLICEPARSERLIB)
	@mkdir -p $(notdir $(<D))
	$(SLICE2RB) --output-dir $(notdir $(<D)) $(SLICE2RBFLAGS) $<

depend::  $(ICE_SLICE) $(GLACIER2_SLICE) $(ICEBOX_SLICE) $(ICEGRID_SLICE) $(ICEPATCH2_SLICE) $(ICESTORM_SLICE)
	-rm -f .depend .depend.mak
	if test -n "$(ICE_SLICE)" ; then \
	    $(SLICE2RB) --depend $(SLICE2RBFLAGS) $(ICE_SLICE) | $(ice_dir)/config/makedepend.py Ice; \
	fi
	if test -n "$(GLACIER2_SLICE)" ; then \
	    $(SLICE2RB) --depend $(SLICE2RBFLAGS) $(GLACIER2_SLICE) | $(ice_dir)/config/makedepend.py Glacier2; \
	fi
	if test -n "$(ICEBOX_SLICE)" ; then \
	    $(SLICE2RB) --depend $(SLICE2RBFLAGS) $(ICEBOX_SLICE) | $(ice_dir)/config/makedepend.py IceBox; \
	fi
	if test -n "$(ICEGRID_SLICE)" ; then \
	    $(SLICE2RB) --depend $(SLICE2RBFLAGS) $(ICEGRID_SLICE) | $(ice_dir)/config/makedepend.py IceGrid; \
	fi
	if test -n "$(ICEPATCH2_SLICE)" ; then \
	    $(SLICE2RB) --depend $(SLICE2RBFLAGS) $(ICEPATCH2_SLICE) | $(ice_dir)/config/makedepend.py IcePatch2; \
	fi
	if test -n "$(ICESTORM_SLICE)" ; then \
	    $(SLICE2RB) --depend $(SLICE2RBFLAGS) $(ICESTORM_SLICE) | $(ice_dir)/config/makedepend.py IceStorm; \
	fi

install:: $(ALL_SRCS)
	@echo "Installing generated code"
	@for i in $(MODULES) ; \
	do \
	    $(INSTALL_DATA) $${i}.rb $(DESTDIR)$(install_rubydir) ; \
	    $(INSTALL_DATA) -r $$i $(DESTDIR)$(install_rubydir) ; \
	done

clean::
	rm -rf $(MODULES)

include .depend
