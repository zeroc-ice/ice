// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

namespace IceInternal
{

ICE_API void loadPlugin(const ::Ice::CommunicatorPtr&, const std::string&, const std::string&, Ice::StringSeq&);

}

namespace Ice
{

class PluginManagerI : public PluginManager, public IceUtil::Mutex
{
public:

    virtual void initializePlugins();
    virtual StringSeq getPlugins();
    virtual PluginPtr getPlugin(const std::string&);
    virtual void addPlugin(const std::string&, const PluginPtr&);
    virtual void destroy();

private:

    PluginManagerI(const CommunicatorPtr&, const IceInternal::DynamicLibraryListPtr&);
    friend class IceInternal::Instance;
    friend void IceInternal::loadPlugin(const Ice::CommunicatorPtr&, const std::string&, const std::string&, 
                                        Ice::StringSeq&);

    void loadPlugins(int&, char*[]);
    void loadPlugin(const std::string&, const std::string&, StringSeq&);

    CommunicatorPtr _communicator;
    IceInternal::DynamicLibraryListPtr _libraries;

    std::map<std::string, PluginPtr> _plugins;
    std::vector<PluginPtr> _initOrder;
    bool _initialized;
    static const char * const _kindOfObject;
};
typedef IceUtil::Handle<PluginManagerI> PluginManagerIPtr;

}


#endif
