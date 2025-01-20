// Copyright (c) ZeroC, Inc.

#ifndef TEST_PLUGIN_H
#define TEST_PLUGIN_H

#include "Configuration.h"
#include "Ice/Plugin.h"

#ifndef TEST_API
#    if defined(TEST_API_EXPORTS)
#        define TEST_API ICE_DECLSPEC_EXPORT
#    else
#        define TEST_API ICE_DECLSPEC_IMPORT
#    endif
#endif

class TEST_API PluginI : public Ice::Plugin
{
public:
    virtual ConfigurationPtr getConfiguration() = 0;
};

#endif
