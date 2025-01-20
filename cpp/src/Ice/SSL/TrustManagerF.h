// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_TRUST_MANAGER_F_H
#define ICE_SSL_TRUST_MANAGER_F_H

#include <memory>

namespace Ice::SSL
{
    class TrustManager;
    using TrustManagerPtr = std::shared_ptr<TrustManager>;
}

#endif
