// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef LOCATOR_DISCOVERY_PLUGIN_I_H
#define LOCATOR_DISCOVERY_PLUGIN_I_H

#include <Ice/Plugin.h>

namespace IceLocatorDiscovery
{

class PluginI : public Ice::Plugin
{
public:
    
    PluginI(const Ice::CommunicatorPtr&);

    virtual void initialize();
    virtual void destroy();

private:

    const Ice::CommunicatorPtr _communicator;
    Ice::ObjectAdapterPtr _locatorAdapter;
    Ice::ObjectAdapterPtr _replyAdapter;
};

};

#endif
