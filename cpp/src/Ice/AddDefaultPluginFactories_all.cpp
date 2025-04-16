// Copyright (c) ZeroC, Inc.

#include "AddDefaultPluginFactories.h"

#include <algorithm>

using namespace std;

void
IceInternal::addDefaultPluginFactories(vector<Ice::PluginFactory>& pluginFactories)
{
    // These built-in plug-ins are always available and not returned by a Ice::namePluginFactory function.
    vector<Ice::PluginFactory> defaultPluginFactories{{Ice::tcpPluginFactory(), Ice::sslPluginFactory()}};

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
        defaultPluginFactories.push_back(std::move(wsPluginFactory));
    }

    pluginFactories.insert(pluginFactories.begin(), defaultPluginFactories.begin(), defaultPluginFactories.end());
}

bool
IceInternal::isMinBuild()
{
    return false;
}
