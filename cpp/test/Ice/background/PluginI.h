// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_PLUGIN_H
#define TEST_PLUGIN_H

#include <Ice/Plugin.h>
#include <Configuration.h>

class PluginI : public Ice::Plugin
{
public:
    
    virtual ConfigurationPtr getConfiguration() = 0;
};

#endif
