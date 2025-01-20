// Copyright (c) ZeroC, Inc.

#ifndef PLUGIN_I_H
#define PLUGIN_I_H

#include "Ice/Plugin.h"
#include "LookupI.h"

namespace IceDiscovery
{
    class PluginI : public Ice::Plugin
    {
    public:
        PluginI(Ice::CommunicatorPtr);

        void initialize() override;
        void destroy() override;

    private:
        const Ice::CommunicatorPtr _communicator;
        Ice::ObjectAdapterPtr _multicastAdapter;
        Ice::ObjectAdapterPtr _replyAdapter;
        Ice::ObjectAdapterPtr _locatorAdapter;
        LookupIPtr _lookup;
        std::optional<Ice::LocatorPrx> _locator;
        std::optional<Ice::LocatorPrx> _defaultLocator;
    };

};

#endif
