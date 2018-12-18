// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef TEST_PLUGIN_H
#define TEST_PLUGIN_H

#include <Ice/Plugin.h>
#include <Configuration.h>

#ifndef TEST_API
#   if defined(ICE_STATIC_LIBS)
#       define TEST_API /**/
#   elif defined(TEST_API_EXPORTS)
#       define TEST_API ICE_DECLSPEC_EXPORT
#   else
#       define TEST_API ICE_DECLSPEC_IMPORT
#   endif
#endif

class TEST_API PluginI : public Ice::Plugin
{
public:

    virtual ConfigurationPtr getConfiguration() = 0;
};

#endif
