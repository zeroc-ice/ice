// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
