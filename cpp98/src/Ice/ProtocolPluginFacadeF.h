//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PROTOCOL_PLUGIN_FACADE_F_H
#define ICE_PROTOCOL_PLUGIN_FACADE_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class ProtocolPluginFacade;
ICE_API IceUtil::Shared* upCast(ProtocolPluginFacade*);
typedef Handle<ProtocolPluginFacade> ProtocolPluginFacadePtr;

}

#endif
