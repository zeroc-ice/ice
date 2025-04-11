// Copyright (c) ZeroC, Inc.

#include "AddDefaultPluginFactories.h"

using namespace std;

extern "C"
{
    Ice::Plugin* createIceTCP(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
    Ice::Plugin* createIceSSL(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
}

void
IceInternal::addDefaultPluginFactories(vector<Ice::PluginFactory>& pluginFactories)
{
    pluginFactories.insert(pluginFactories.begin(), {{"IceTCP", createIceTCP}, {"IceSSL", createIceSSL}});
}

bool
IceInternal::isMinBuild()
{
    return true;
}
