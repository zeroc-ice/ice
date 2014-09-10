# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

include $(top_srcdir)/config/Make.rules.php

#
# IMPORTANT: If you add or remove Slice files, you also need to check Ice.php!
#
ICE_SRCS	= Ice/LocalException.php \
		  Ice/Communicator.php \
		  Ice/CommunicatorF.php \
		  Ice/Logger.php \
		  Ice/LoggerF.php \
		  Ice/BuiltinSequences.php \
		  Ice/ObjectAdapterF.php \
		  Ice/PropertiesAdmin.php \
		  Ice/Properties.php \
		  Ice/PropertiesF.php \
		  Ice/ObjectFactory.php \
		  Ice/ObjectFactoryF.php \
		  Ice/Identity.php \
		  Ice/Current.php \
		  Ice/ImplicitContextF.php \
		  Ice/ImplicitContext.php \
		  Ice/Router.php \
		  Ice/RouterF.php \
		  Ice/Plugin.php \
		  Ice/PluginF.php \
		  Ice/Locator.php \
		  Ice/LocatorF.php \
		  Ice/Process.php \
		  Ice/ProcessF.php \
		  Ice/FacetMap.php \
		  Ice/Connection.php \
		  Ice/ConnectionF.php \
		  Ice/SliceChecksumDict.php \
		  Ice/Endpoint.php \
		  Ice/EndpointF.php \
		  Ice/EndpointTypes.php \
		  Ice/Version.php \
		  Ice/InstrumentationF.php \
		  Ice/Instrumentation.php \
		  Ice/Metrics.php

#
# IMPORTANT: If you add or remove Slice files, you also need to check Glacier2.php!
#
GLACIER2_SRCS	= Glacier2/RouterF.php \
		  Glacier2/Router.php \
		  Glacier2/Session.php \
		  Glacier2/PermissionsVerifierF.php \
		  Glacier2/PermissionsVerifier.php \
		  Glacier2/SSLInfo.php \
		  Glacier2/Metrics.php

#
# IMPORTANT: If you add or remove Slice files, you also need to check IceBox.php!
#
ICEBOX_SRCS	= IceBox/IceBox.php

#
# IMPORTANT: If you add or remove Slice files, you also need to check IceGrid.php!
#
ICEGRID_SRCS	= IceGrid/Admin.php \
		  IceGrid/Descriptor.php \
		  IceGrid/Exception.php \
		  IceGrid/FileParser.php \
		  IceGrid/Locator.php \
		  IceGrid/Observer.php \
		  IceGrid/Query.php \
		  IceGrid/Registry.php \
		  IceGrid/Session.php \
		  IceGrid/UserAccountMapper.php

#
# IMPORTANT: If you add or remove Slice files, you also need to check IcePatch2.php!
#
ICEPATCH2_SRCS	= IcePatch2/FileInfo.php \
		  IcePatch2/FileServer.php

#
# IMPORTANT: If you add or remove Slice files, you also need to check IceStorm.php!
#
ICESTORM_SRCS	= IceStorm/IceStorm.php \
		  IceStorm/Metrics.php

ALL_SRCS	= $(ICE_SRCS) \
		  $(GLACIER2_SRCS) \
		  $(ICEBOX_SRCS) \
		  $(ICEGRID_SRCS) \
		  $(ICEPATCH2_SRCS) \
		  $(ICESTORM_SRCS)

ICE_SLICE       = $(patsubst %.php,$(slicedir)/%.ice,$(ICE_SRCS))
GLACIER2_SLICE  = $(patsubst %.php,$(slicedir)/%.ice,$(GLACIER2_SRCS))
ICEBOX_SLICE    = $(patsubst %.php,$(slicedir)/%.ice,$(ICEBOX_SRCS))
ICEGRID_SLICE   = $(patsubst %.php,$(slicedir)/%.ice,$(ICEGRID_SRCS))
ICEPATCH2_SLICE = $(patsubst %.php,$(slicedir)/%.ice,$(ICEPATCH2_SRCS))
ICESTORM_SLICE  = $(patsubst %.php,$(slicedir)/%.ice,$(ICESTORM_SRCS))

MODULES		= Glacier2 Ice IceBox IceGrid IcePatch2 IceStorm
ifeq ("$(USE_NAMESPACES)","yes")
MODULE_SRCS	= Glacier2.php Ice_ns.php IceBox.php IceGrid.php IcePatch2.php IceStorm.php
else
MODULE_SRCS	= Glacier2.php Ice.php IceBox.php IceGrid.php IcePatch2.php IceStorm.php
endif

all:: $(ALL_SRCS)

SLICE2PHPFLAGS	+= --ice 

Ice/%.php: $(slicedir)/Ice/%.ice
	@mkdir -p $(notdir $(<D))
	$(SLICE2PHP) --output-dir $(notdir $(<D)) $(SLICE2PHPFLAGS) $<
	@mkdir -p Ice/.depend
	@$(SLICE2PHP) --output-dir $(notdir $(<D)) $(SLICE2PHPFLAGS) $< > Ice/.depend/$(*F).ice.d

Glacier2/%.php: $(slicedir)/Glacier2/%.ice
	@mkdir -p $(notdir $(<D))
	$(SLICE2PHP) --output-dir $(notdir $(<D)) $(SLICE2PHPFLAGS) $<
	@mkdir -p Glacier2/.depend
	@$(SLICE2PHP) --output-dir $(notdir $(<D)) $(SLICE2PHPFLAGS) $< > Glacier2/.depend/$(*F).ice.d

IceBox/%.php: $(slicedir)/IceBox/%.ice
	@mkdir -p $(notdir $(<D))
	$(SLICE2PHP) --output-dir $(notdir $(<D)) $(SLICE2PHPFLAGS) $<
	@mkdir IceBox/.depend
	@$(SLICE2PHP) --output-dir $(notdir $(<D)) $(SLICE2PHPFLAGS) $< > IceBox/.depend/$(*F).ice.d

IceGrid/%.php: $(slicedir)/IceGrid/%.ice
	@mkdir -p $(notdir $(<D))
	$(SLICE2PHP) --output-dir $(notdir $(<D)) $(SLICE2PHPFLAGS) $<
	@mkdir -p IceGrid/.depend
	@$(SLICE2PHP) --output-dir $(notdir $(<D)) $(SLICE2PHPFLAGS) $< > IceGrid/.depend/$(*F).ice.d

IcePatch2/%.php: $(slicedir)/IcePatch2/%.ice
	@mkdir -p $(notdir $(<D))
	$(SLICE2PHP) --output-dir $(notdir $(<D)) $(SLICE2PHPFLAGS) $<
	@mkdir -p IcePatch2/.depend
	@$(SLICE2PHP) --output-dir $(notdir $(<D)) $(SLICE2PHPFLAGS) $< > IcePatch2/.depend/$(*F).ice.d

IceStorm/%.php: $(slicedir)/IceStorm/%.ice
	@mkdir -p $(notdir $(<D))
	$(SLICE2PHP) --output-dir $(notdir $(<D)) $(SLICE2PHPFLAGS) $<
	@mkdir -p IceStorm/.depend
	@$(SLICE2PHP) --output-dir $(notdir $(<D)) $(SLICE2PHPFLAGS) $< > IceStorm/.depend/$(*F).ice.d

install:: $(ALL_SRCS)
	@echo "Installing generated code"
	@for i in $(MODULES) ; \
	do \
	    $(INSTALL_DATA) -r $$i $(DESTDIR)$(install_phpdir) ; \
	done
	@for i in $(MODULE_SRCS) ; \
	do \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(install_phpdir) ; \
	done

clean::
	rm -rf $(MODULES)

-include $(wildcard */.depend/*.d)
