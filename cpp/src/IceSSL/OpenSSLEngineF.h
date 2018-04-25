// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_OPENSSL_ENGINE_F_H
#define ICESSL_OPENSSL_ENGINE_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

#include <IceSSL/OpenSSL.h>

namespace IceSSL
{

namespace OpenSSL
{

class SSLEngine;
ICESSL_OPENSSL_API IceUtil::Shared* upCast(SSLEngine*);
typedef IceInternal::Handle<SSLEngine> SSLEnginePtr;

} // OpenSSL namespace end

} // IceSSL namespace end

#endif
