// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_WS_PLUGIN_H
#define ICE_WS_PLUGIN_H

#include <Ice/Plugin.h>

#ifndef ICE_WS_API
#   ifdef ICE_WS_API_EXPORTS
#       define ICE_WS_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_WS_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace IceWS
{

class Plugin : public Ice::Plugin
{
public:
    // TODO
};
typedef IceUtil::Handle<Plugin> PluginPtr;

}

#endif
