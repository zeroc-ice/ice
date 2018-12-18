// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_SECURE_TRANSPORT_ENGINE_F_H
#define ICESSL_SECURE_TRANSPORT_ENGINE_F_H

#ifdef __APPLE__
#include <IceSSL/Plugin.h>
#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace IceSSL
{

namespace SecureTransport
{

class SSLEngine;
ICESSL_API IceUtil::Shared* upCast(SSLEngine*);
typedef IceInternal::Handle<SSLEngine> SSLEnginePtr;

}

}

#endif

#endif
