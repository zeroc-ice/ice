// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/PluginI.h>
#include <IceSSL/Instance.h>
#include <IceSSL/SSLEngine.h>
#include <IceSSL/EndpointI.h>

#include <Ice/ProtocolPluginFacade.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

//
// Plug-in factory function.
//

namespace Ice
{

ICE_SSL_API void
registerIceSSL(bool loadOnInitialize)
{
    Ice::registerPluginFactory("IceSSL", createIceSSL, true);
}

}
