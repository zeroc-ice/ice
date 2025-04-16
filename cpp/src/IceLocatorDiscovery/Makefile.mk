# Copyright (c) ZeroC, Inc.

$(project)_libraries := IceLocatorDiscovery

# Tell the slice2cpp rule to ignore the top-level IceLocatorDiscovery include directory.
IceLocatorDiscovery_includedir                  := $(project)/generated/IceLocatorDiscovery
$(project)_generated_srcdir                     := $(project)/generated/IceLocatorDiscovery
$(project)_generated_includedir                 := $(project)/generated/IceLocatorDiscovery

IceLocatorDiscovery_targetdir                   := $(libdir)
IceLocatorDiscovery_dependencies                := Ice
IceLocatorDiscovery_cppflags                    := -DICE_LOCATOR_DISCOVERY_API_EXPORTS $(api_exports_cppflags)

projects += $(project)
