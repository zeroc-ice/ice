// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
