# Copyright (c) ZeroC, Inc.

$(project)_libraries := IceLocatorDiscovery

IceLocatorDiscovery_do_not_install_generated_headers := yes

IceLocatorDiscovery_sliceflags                  := --include-dir IceLocatorDiscovery
IceLocatorDiscovery_targetdir                   := $(libdir)
IceLocatorDiscovery_dependencies                := Ice
IceLocatorDiscovery_cppflags                    := -DICE_LOCATOR_DISCOVERY_API_EXPORTS $(api_exports_cppflags)

projects += $(project)
