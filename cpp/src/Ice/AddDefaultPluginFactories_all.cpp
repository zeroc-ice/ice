// Copyright (c) ZeroC, Inc.

#include "AddDefaultPluginFactories.h"

#include <algorithm>

using namespace std;

extern "C"
{
    Ice::Plugin* createIceUDP(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
    Ice::Plugin* createIceTCP(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
    Ice::Plugin* createIceWS(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
    Ice::Plugin* createIceSSL(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
}

void
IceInternal::addDefaultPluginFactories(vector<Ice::PluginFactory>& pluginFactories)
{
    // These built-in plug-ins are always available and not returned by a Ice::namePluginFactory function.
    vector<Ice::PluginFactory> defaultPluginFactories{{"IceTCP", createIceTCP}, {"IceSSL", createIceSSL}};

    Ice::PluginFactory udpPluginFactory{Ice::udpPluginFactory()};
    Ice::PluginFactory wsPluginFactory{Ice::wsPluginFactory()};

    if (none_of(
            pluginFactories.begin(),
            pluginFactories.end(),
            [&udpPluginFactory](const Ice::PluginFactory& factory)
            { return factory.pluginName == udpPluginFactory.pluginName; }))
    {
        defaultPluginFactories.push_back(std::move(udpPluginFactory));
    }

    if (none_of(
            pluginFactories.begin(),
            pluginFactories.end(),
            [&wsPluginFactory](const Ice::PluginFactory& factory)
            { return factory.pluginName == wsPluginFactory.pluginName; }))
    {
        defaultPluginFactories.push_back(Ice::wsPluginFactory());
    }

    pluginFactories.insert(pluginFactories.begin(), defaultPluginFactories.begin(), defaultPluginFactories.end());
}

bool
IceInternal::isMinBuild()
{
    return false;
}
