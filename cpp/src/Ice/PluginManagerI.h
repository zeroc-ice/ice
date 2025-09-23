// Copyright (c) ZeroC, Inc.

#ifndef ICE_PLUGIN_MANAGER_I_H
#define ICE_PLUGIN_MANAGER_I_H

#include "Ice/BuiltinSequences.h"
#include "Ice/CommunicatorF.h"
#include "Ice/InstanceF.h"
#include "Ice/Plugin.h"
#include "Ice/PluginFactory.h"

#include <map>
#include <mutex>

namespace Ice
{
    class PluginManagerI final : public PluginManager
    {
    public:
        void initializePlugins() final;
        StringSeq getPlugins() final;
        PluginPtr getPlugin(std::string_view) final;
        void addPlugin(std::string, PluginPtr) final;
        void destroy() noexcept final;

        // Constructs the plugin manager (internal).
        PluginManagerI(CommunicatorPtr);

        // Loads all the plugins (internal).
        // Returns true when one or more libraries may have been loaded dynamically; returns false when definitely no
        // library was loaded dynamically.
        bool loadPlugins();

    private:
        bool loadPlugin(PluginFactoryFunc pluginFactoryFunc, const std::string& name, const std::string& pluginSpec);

        [[nodiscard]] PluginPtr findPlugin(std::string_view) const;

        struct PluginInfo
        {
            std::string name;
            PluginPtr plugin;
        };
        using PluginInfoList = std::vector<PluginInfo>;

        CommunicatorPtr _communicator;
        PluginInfoList _plugins;
        bool _initialized{false};
        std::mutex _mutex;
        static const char* const _kindOfObject;
    };
}

#endif
