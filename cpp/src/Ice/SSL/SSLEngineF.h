//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SSL_ENGINE_F_H
#define ICE_SSL_ENGINE_F_H

#include <memory>

namespace Ice::SSL
{
    class SSLEngine;
    using SSLEnginePtr = std::shared_ptr<SSLEngine>;
}

#endif
