//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PLUGIN_F_H
#define ICE_PLUGIN_F_H

#include <memory>

namespace Ice
{

class Plugin;
using PluginPtr = std::shared_ptr<Plugin>;

class PluginManager;
using PluginManagerPtr = std::shared_ptr<PluginManager>;

}

#endif
