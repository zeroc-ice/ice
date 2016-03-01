// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_ENGINE_F_H
#define ICE_SSL_ENGINE_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

#include <IceSSL/Plugin.h>

namespace IceSSL
{

class SSLEngine;
ICE_SSL_API IceUtil::Shared* upCast(IceSSL::SSLEngine*);
typedef IceInternal::Handle<SSLEngine> SSLEnginePtr;

#if defined(ICE_USE_SECURE_TRANSPORT)
class SecureTransportEngine;
ICE_SSL_API IceUtil::Shared* upCast(IceSSL::SecureTransportEngine*);
typedef IceInternal::Handle<SecureTransportEngine> SecureTransportEnginePtr;
#elif defined(ICE_USE_SCHANNEL)
class SChannelEngine;
ICE_SSL_API IceUtil::Shared* upCast(IceSSL::SChannelEngine*);
typedef IceInternal::Handle<SChannelEngine> SChannelEnginePtr;
#else // OpenSSL
class OpenSSLEngine;
ICE_SSL_API IceUtil::Shared* upCast(IceSSL::OpenSSLEngine*);
typedef IceInternal::Handle<OpenSSLEngine> OpenSSLEnginePtr;
#endif

}

#endif
