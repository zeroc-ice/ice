# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries := IceLocatorDiscovery

IceLocatorDiscovery_targetdir                   := $(libdir)
IceLocatorDiscovery_dependencies                := Ice
IceLocatorDiscovery_sliceflags                  := --include-dir IceLocatorDiscovery
IceLocatorDiscovery_generated_includedir        := $(project)/generated/IceLocatorDiscovery
IceLocatorDiscovery_cppflags                    := -DICE_LOCATOR_DISCOVERY_API_EXPORTS

projects += $(project)
