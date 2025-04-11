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

    if (none_of(
            pluginFactories.begin(),
            pluginFactories.end(),
            [](const Ice::PluginFactory& factory) { return factory.pluginName == "IceUDP"; }))
    {
        defaultPluginFactories.push_back({"IceUDP", createIceUDP});
    }

    if (none_of(
            pluginFactories.begin(),
            pluginFactories.end(),
            [](const Ice::PluginFactory& factory) { return factory.pluginName == "IceWS"; }))
    {
        defaultPluginFactories.push_back({"IceWS", createIceWS});
    }

    pluginFactories.insert(pluginFactories.begin(), defaultPluginFactories.begin(), defaultPluginFactories.end());
}

bool
IceInternal::isMinBuild()
{
    return false;
}
