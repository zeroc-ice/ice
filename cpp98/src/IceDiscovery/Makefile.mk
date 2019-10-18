#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries := IceDiscovery

IceDiscovery_targetdir                  := $(libdir)
IceDiscovery_dependencies               := Ice
IceDiscovery_sliceflags                 := --include-dir IceDiscovery
IceDiscovery_generated_includedir       := $(project)/generated/IceDiscovery
IceDiscovery_cppflags                   := -DICE_DISCOVERY_API_EXPORTS

projects += $(project)
