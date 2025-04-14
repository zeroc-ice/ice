# Copyright (c) ZeroC, Inc.

$(project)_libraries := IceDiscovery

# Tell the slice2cpp rule to ignore the top-level IceDiscovery include directory.
IceDiscovery_includedir                 := $(project)/generated/IceDiscovery

IceDiscovery_targetdir                  := $(libdir)
IceDiscovery_dependencies               := Ice
IceDiscovery_cppflags                   := -DICE_DISCOVERY_API_EXPORTS $(api_exports_cppflags)

projects += $(project)
