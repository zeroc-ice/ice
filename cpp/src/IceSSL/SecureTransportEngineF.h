//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_SECURE_TRANSPORT_ENGINE_F_H
#define ICESSL_SECURE_TRANSPORT_ENGINE_F_H

#ifdef __APPLE__

#include <IceSSL/Plugin.h>

#include <memory>

namespace IceSSL
{

namespace SecureTransport
{

class SSLEngine;
using SSLEnginePtr = std::shared_ptr<SSLEngine>;

}

}

#endif

#endif
