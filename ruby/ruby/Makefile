# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

include $(top_srcdir)/config/Make.rules

#
# Disable parallel builds
#
.NOTPARALLEL:

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
		  Ice/RemoteLogger.rb \
		  Ice/Router.rb \
		  Ice/RouterF.rb \
		  Ice/Plugin.rb \
		  Ice/PluginF.rb \
		  Ice/Locator.rb \
		  Ice/LocatorF.rb \
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
	@mkdir -p .depend
	@$(SLICE2RB) --output-dir $(notdir $(<D)) $(SLICE2RBFLAGS) --depend $< | \
		sed 's/\(.*: \\\)/Ice\/\1/' > .depend/Ice_$(*F).ice.d

Glacier2/%.rb: $(slicedir)/Glacier2/%.ice $(SLICE2RB) $(SLICEPARSERLIB)
	@mkdir -p $(notdir $(<D))
	$(SLICE2RB) --output-dir $(notdir $(<D)) $(SLICE2RBFLAGS) $<
	@mkdir -p .depend
	@$(SLICE2RB) --output-dir $(notdir $(<D)) $(SLICE2RBFLAGS) --depend $< | \
		sed 's/\(.*: \\\)/Glacier2\/\1/' > .depend/Glacier2_$(*F).ice.d

IceBox/%.rb: $(slicedir)/IceBox/%.ice $(SLICE2RB) $(SLICEPARSERLIB)
	@mkdir -p $(notdir $(<D))
	$(SLICE2RB) --output-dir $(notdir $(<D)) $(SLICE2RBFLAGS) $<
	@mkdir -p .depend
	@$(SLICE2RB) --output-dir $(notdir $(<D)) $(SLICE2RBFLAGS) --depend $< | \
		sed 's/\(.*: \\\)/IceBox\/\1/' > .depend/IceBox_$(*F).ice.d

IceGrid/%.rb: $(slicedir)/IceGrid/%.ice $(SLICE2RB) $(SLICEPARSERLIB)
	@mkdir -p $(notdir $(<D))
	$(SLICE2RB) --output-dir $(notdir $(<D)) $(SLICE2RBFLAGS) $<
	@mkdir -p .depend
	@$(SLICE2RB) --output-dir $(notdir $(<D)) $(SLICE2RBFLAGS) --depend $< | \
		sed 's/\(.*: \\\)/IceGrid\/\1/' > .depend/IceGrid_$(*F).ice.d

IcePatch2/%.rb: $(slicedir)/IcePatch2/%.ice $(SLICE2RB) $(SLICEPARSERLIB)
	@mkdir -p $(notdir $(<D))
	$(SLICE2RB) --output-dir $(notdir $(<D)) $(SLICE2RBFLAGS) $<
	@mkdir -p .depend
	@$(SLICE2RB) --output-dir $(notdir $(<D)) $(SLICE2RBFLAGS) --depend $< | \
		sed 's/\(.*: \\\)/IcePatch2\/\1/' > .depend/IcePatch2_$(*F).ice.d

IceStorm/%.rb: $(slicedir)/IceStorm/%.ice $(SLICE2RB) $(SLICEPARSERLIB)
	@mkdir -p $(notdir $(<D))
	$(SLICE2RB) --output-dir $(notdir $(<D)) $(SLICE2RBFLAGS) $<
	@mkdir -p .depend
	@$(SLICE2RB) --output-dir $(notdir $(<D)) $(SLICE2RBFLAGS) --depend $< | \
		sed 's/\(.*: \\\)/IceStorm\/\1/' > .depend/IceStorm_$(*F).ice.d

install:: $(ALL_SRCS)
	@echo "Installing generated code"
	@for i in $(MODULES) ; \
	do \
	    $(INSTALL_DATA) $${i}.rb $(DESTDIR)$(install_rubydir) ; \
	    $(INSTALL_DATA) -r $$i $(DESTDIR)$(install_rubydir) ; \
	done

clean::
	rm -rf $(MODULES)
