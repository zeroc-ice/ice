# Copyright (c) ZeroC, Inc.

$(project)_libraries := IceDiscovery

IceDiscovery_sliceflags                 := --include-dir IceDiscovery
IceDiscovery_targetdir                  := $(libdir)
IceDiscovery_dependencies               := Ice
IceDiscovery_cppflags                   := -DICE_DISCOVERY_API_EXPORTS $(api_exports_cppflags)

IceDiscovery_install_generated_headers  := no

projects += $(project)
