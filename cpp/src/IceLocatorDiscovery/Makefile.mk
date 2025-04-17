# Copyright (c) ZeroC, Inc.

$(project)_libraries := IceLocatorDiscovery

IceLocatorDiscovery_sliceflags                  := --include-dir IceLocatorDiscovery
IceLocatorDiscovery_targetdir                   := $(libdir)
IceLocatorDiscovery_dependencies                := Ice
IceLocatorDiscovery_cppflags                    := -DICE_LOCATOR_DISCOVERY_API_EXPORTS $(api_exports_cppflags)

IceLocatorDiscovery_install_generated_headers   := no

projects += $(project)
