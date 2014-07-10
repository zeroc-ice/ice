// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef PLUGIN_I_H
#define PLUGIN_I_H

#include <Ice/Plugin.h>

namespace IceGrid
{

class DiscoveryPluginI : public Ice::Plugin
{
public:
    
    DiscoveryPluginI(const Ice::CommunicatorPtr&);

    virtual void initialize();
    virtual void destroy();

private:

    const Ice::CommunicatorPtr _communicator;
    Ice::ObjectAdapterPtr _locatorAdapter;
    Ice::ObjectAdapterPtr _replyAdapter;
};

};

#endif
