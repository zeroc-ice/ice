// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BT_PLUGIN_I_H
#define ICE_BT_PLUGIN_I_H

#include <Ice/CommunicatorF.h>
#include <Ice/Plugin.h>
#include <IceBT/EngineF.h>

namespace IceBT
{

class PluginI : public Ice::Plugin
{
public:

    PluginI(const Ice::CommunicatorPtr&);

    //
    // From Ice::Plugin.
    //
    virtual void initialize();
    virtual void destroy();

private:

    EnginePtr _engine;
};

}

#endif
