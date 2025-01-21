// Copyright (c) ZeroC, Inc.

#ifndef LOCATOR_DISCOVERY_PLUGIN_I_H
#define LOCATOR_DISCOVERY_PLUGIN_I_H

#include "Ice/Config.h"
#include "Ice/Locator.h"
#include "Ice/Plugin.h"

#include <chrono>

#ifndef ICE_LOCATOR_DISCOVERY_API
#    if defined(ICE_LOCATOR_DISCOVERY_API_EXPORTS)
#        define ICE_LOCATOR_DISCOVERY_API ICE_DECLSPEC_EXPORT
#    else
#        define ICE_LOCATOR_DISCOVERY_API ICE_DECLSPEC_IMPORT
#    endif
#endif

#if !defined(ICE_BUILDING_ICE_LOCATOR_DISCOVERY) && defined(ICE_LOCATOR_DISCOVERY_API_EXPORTS)
#    define ICE_BUILDING_ICE_LOCATOR_DISCOVERY
#endif

// For icegridadmin
#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICE_LOCATOR_DISCOVERY)
#    pragma comment(lib, ICE_LIBNAME("IceLocatorDiscovery")) // Automatically link IceLocatorDiscovery[D].lib
#endif

namespace IceLocatorDiscovery
{
    class ICE_LOCATOR_DISCOVERY_API Plugin : public Ice::Plugin
    {
    public:
        [[nodiscard]] virtual std::vector<Ice::LocatorPrx>
        getLocators(const std::string&, const std::chrono::milliseconds&) const = 0;
    };
    using PluginPtr = std::shared_ptr<Plugin>;
}

#endif
