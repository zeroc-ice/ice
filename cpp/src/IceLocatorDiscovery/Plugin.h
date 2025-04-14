// Copyright (c) ZeroC, Inc.

#ifndef LOCATOR_DISCOVERY_PLUGIN_I_H
#define LOCATOR_DISCOVERY_PLUGIN_I_H

#include "Ice/Config.h"
#include "Ice/Locator.h"
#include "Ice/Plugin.h"
#include "Ice/PluginFactory.h" // for ICE_LOCATOR_DISCOVERY_API

#include <chrono>

namespace IceLocatorDiscovery
{
    // Exported for IceGrid
    class ICE_LOCATOR_DISCOVERY_API Plugin : public Ice::Plugin
    {
    public:
        ~Plugin() override;

        [[nodiscard]] virtual std::vector<Ice::LocatorPrx>
        getLocators(const std::string&, const std::chrono::milliseconds&) const = 0;
    };
    using PluginPtr = std::shared_ptr<Plugin>;
}

#endif
