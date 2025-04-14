// Copyright (c) ZeroC, Inc.

#ifndef ICE_LOCATOR_DISCOVERY_H
#define ICE_LOCATOR_DISCOVERY_H

#include "Ice/Ice.h"

#ifndef ICE_LOCATOR_DISCOVERY_API
#    ifdef ICE_LOCATOR_DISCOVERY_API_EXPORTS
#        define ICE_LOCATOR_DISCOVERY_API ICE_DECLSPEC_EXPORT
#    else
#        define ICE_LOCATOR_DISCOVERY_API /**/
#    endif
#endif

#if defined(_MSC_VER) && !defined(ICE_LOCATOR_DISCOVERY_API_EXPORTS) && !defined(ICE_DISABLE_PRAGMA_COMMENT)
#    pragma comment(lib, ICE_LIBNAME("IceLocatorDiscovery")) // Automatically link with IceLocatorDiscovery[D].lib
#endif

namespace IceLocatorDiscovery
{
    /// Returns the factory for the IceLocatorDiscovery plug-in.
    /// @return The factory for the IceLocatorDiscovery plug-in.
    /// @see Ice::InitializationData::pluginFactories
    ICE_LOCATOR_DISCOVERY_API Ice::PluginFactory locatorDiscoveryPluginFactory();
}

#endif
