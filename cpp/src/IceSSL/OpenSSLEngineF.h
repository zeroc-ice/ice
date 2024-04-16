//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_OPENSSL_ENGINE_F_H
#define ICESSL_OPENSSL_ENGINE_F_H

#include <memory>

namespace IceSSL::OpenSSL
{
    class SSLEngine;
    using SSLEnginePtr = std::shared_ptr<SSLEngine>;
}
#endif
