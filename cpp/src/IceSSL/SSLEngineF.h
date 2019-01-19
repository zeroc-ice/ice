//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_ENGINE_F_H
#define ICESSL_ENGINE_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>
#include <IceSSL/Plugin.h>

namespace IceSSL
{

class SSLEngine;
ICESSL_API IceUtil::Shared* upCast(SSLEngine*);
typedef IceInternal::Handle<SSLEngine> SSLEnginePtr;

}

#endif
