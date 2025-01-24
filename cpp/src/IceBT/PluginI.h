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
        virtual void initialize();
        virtual void destroy();

        //
        // From IceBT::Plugin.
        //
        virtual void startDiscovery(
            const std::string& address,
            std::function<void(const std::string& addr, const PropertyMap& props)>);
        virtual void stopDiscovery(const std::string&);

        [[nodiscard]] virtual DeviceMap getDevices() const;

    private:
        EnginePtr _engine;
    };
}

#endif
