// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
