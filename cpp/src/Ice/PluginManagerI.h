// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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

private:

    PluginManagerI(const CommunicatorPtr&, const IceInternal::DynamicLibraryListPtr&);
    friend class IceInternal::Instance;

    void loadPlugins(int&, char*[]);
    void loadPlugin(const std::string&, const std::string&, const StringSeq&);

    CommunicatorPtr _communicator;
    IceInternal::DynamicLibraryListPtr _libraries;

    std::map<std::string, PluginPtr> _plugins;
    static const char * const _kindOfObject = "plug-in";
};

}

#endif
