//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
