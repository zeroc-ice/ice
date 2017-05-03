# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries := IceDiscovery

IceDiscovery_targetdir                  := $(libdir)
IceDiscovery_dependencies               := Ice
IceDiscovery_sliceflags                 := --include-dir IceDiscovery
IceDiscovery_generated_includedir       := $(project)/generated/IceDiscovery
IceDiscovery_cppflags                   := -DICE_DISCOVERY_API_EXPORTS

projects += $(project)
