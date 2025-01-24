// Copyright (c) ZeroC, Inc.

#ifndef ICE_BT_PLUGIN_I_H
#define ICE_BT_PLUGIN_I_H

#include "EngineF.h"
#include "Ice/CommunicatorF.h"
#include "IceBT/Plugin.h"

namespace IceBT
{
    class PluginI : public Plugin
    {
    public:
        PluginI(const Ice::CommunicatorPtr&);

        //
        // From Ice::Plugin.
        //
        void initialize() override;
        void destroy() override;

        //
        // From IceBT::Plugin.
        //
        void startDiscovery(
            const std::string& address,
            std::function<void(const std::string& addr, const PropertyMap& props)>) override;
        void stopDiscovery(const std::string&) override;

        [[nodiscard]] DeviceMap getDevices() const override;

    private:
        EnginePtr _engine;
    };
}

#endif
