// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_SCHANNEL_ENGINE_F_H
#define ICE_SSL_SCHANNEL_ENGINE_F_H

#ifdef _WIN32

#    include <memory>

namespace Ice::SSL::Schannel
{
    class SSLEngine;
    using SSLEnginePtr = std::shared_ptr<SSLEngine>;
}

#endif

#endif
