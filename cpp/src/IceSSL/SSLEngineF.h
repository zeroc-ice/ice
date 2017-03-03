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

#if defined(ICE_USE_SECURE_TRANSPORT)
class SecureTransportEngine;
ICESSL_API IceUtil::Shared* upCast(SecureTransportEngine*);
typedef IceInternal::Handle<SecureTransportEngine> SecureTransportEnginePtr;
#elif defined(ICE_USE_SCHANNEL)
class SChannelEngine;
ICESSL_API IceUtil::Shared* upCast(SChannelEngine*);
typedef IceInternal::Handle<SChannelEngine> SChannelEnginePtr;
#elif defined(ICE_OS_UWP)
class UWPEngine;
ICESSL_API IceUtil::Shared* upCast(UWPEngine*);
typedef IceInternal::Handle<UWPEngine> UWPEnginePtr;
#else // OpenSSL
class OpenSSLEngine;
ICESSL_API IceUtil::Shared* upCast(OpenSSLEngine*);
typedef IceInternal::Handle<OpenSSLEngine> OpenSSLEnginePtr;
#endif

}

#endif
