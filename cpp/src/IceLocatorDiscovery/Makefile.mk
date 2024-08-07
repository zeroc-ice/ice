#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries := IceLocatorDiscovery

IceLocatorDiscovery_targetdir                   := $(libdir)
IceLocatorDiscovery_dependencies                := Ice
IceLocatorDiscovery[shared]_cppflags            := -DICE_LOCATOR_DISCOVERY_API_EXPORTS

projects += $(project)
