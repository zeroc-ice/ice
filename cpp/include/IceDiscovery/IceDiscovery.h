// Copyright (c) ZeroC, Inc.

#ifndef ICE_DISCOVERY_H
#define ICE_DISCOVERY_H

#include "Ice/Ice.h"

#ifndef ICE_DISCOVERY_API
#    ifdef ICE_DISCOVERY_API_EXPORTS
#        define ICE_DISCOVERY_API ICE_DECLSPEC_EXPORT
#    else
#        define ICE_DISCOVERY_API /**/
#    endif
#endif

#if defined(_MSC_VER) && !defined(ICE_DISCOVERY_API_EXPORTS) && !defined(ICE_DISABLE_PRAGMA_COMMENT)
#    pragma comment(lib, ICE_LIBNAME("IceDiscovery")) // Automatically link with IceDiscovery[D].lib
#endif

namespace IceDiscovery
{
    /// Returns the factory for the IceDiscovery plug-in.
    /// @return The factory for the IceDiscovery plug-in.
    /// @see Ice::InitializationData::pluginFactories
    ICE_DISCOVERY_API Ice::PluginFactory discoveryPluginFactory();
}

#endif
