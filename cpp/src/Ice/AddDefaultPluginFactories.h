// Copyright (c) ZeroC, Inc.

#ifndef ICE_ADD_DEFAULT_PLUGIN_FACTORIES_H
#define ICE_ADD_DEFAULT_PLUGIN_FACTORIES_H

#include "Ice/PluginFactory.h"

#include <vector>

namespace IceInternal
{
    /// Adds the default plug-in factories for IceSSL, IceTCP, and, depending on the build, IceUDP and IceWS.
    void addDefaultPluginFactories(std::vector<Ice::PluginFactory>&);
}

#endif
