// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PROTOCOL_PLUGIN_FACADE_F_H
#define ICE_PROTOCOL_PLUGIN_FACADE_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class ProtocolPluginFacade;
ICE_PROTOCOL_API void incRef(ProtocolPluginFacade*);
ICE_PROTOCOL_API void decRef(ProtocolPluginFacade*);
typedef Handle<ProtocolPluginFacade> ProtocolPluginFacadePtr;

}

#endif
