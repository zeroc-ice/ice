// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_SCHANNEL_ENGINE_F_H
#define ICESSL_SCHANNEL_ENGINE_F_H

#ifdef _WIN32

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

#include <IceSSL/SChannel.h>

namespace IceSSL
{

namespace SChannel
{

class SSLEngine;
ICESSL_API IceUtil::Shared* upCast(SSLEngine*);
typedef IceInternal::Handle<SSLEngine> SSLEnginePtr;

}

}

#endif

#endif
