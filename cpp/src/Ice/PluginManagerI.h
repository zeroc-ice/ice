// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PLUGIN_MANAGER_I_H
#define ICE_PLUGIN_MANAGER_I_H

#include <Ice/Plugin.h>
#include <Ice/InstanceF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/DynamicLibraryF.h>
#include <Ice/BuiltinSequences.h>
#include <IceUtil/Mutex.h>
#include <map>

namespace Ice
{

class PluginManagerI : public PluginManager, public IceUtil::Mutex
{
public:

    virtual PluginPtr getPlugin(const std::string&);

    virtual void addPlugin(const std::string&, const PluginPtr&);

    virtual void destroy();

    struct PluginInfo
    {
        PluginPtr plugin;
        IceInternal::DynamicLibraryPtr library;
    };

private:

    PluginManagerI(const IceInternal::InstancePtr&);
    void loadPlugins(int&, char*[]);
    friend class IceInternal::Instance;

    void loadPlugin(const std::string&, const std::string&, const StringSeq&);

    IceInternal::InstancePtr _instance;
    std::map<std::string, PluginInfo> _plugins;
};

}

#endif
