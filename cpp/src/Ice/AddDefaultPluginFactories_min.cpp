// Copyright (c) ZeroC, Inc.

#include "AddDefaultPluginFactories.h"

using namespace std;

void
IceInternal::addDefaultPluginFactories(vector<Ice::PluginFactory>& pluginFactories)
{
    pluginFactories.insert(pluginFactories.begin(), {Ice::tcpPluginFactory(), Ice::sslPluginFactory()});
}

bool
IceInternal::isMinBuild()
{
    return true;
}
