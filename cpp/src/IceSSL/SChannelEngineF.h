//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_SCHANNEL_ENGINE_F_H
#define ICESSL_SCHANNEL_ENGINE_F_H

#ifdef _WIN32

#    include "Ice/SChannel.h"

#    include <memory>

namespace IceSSL::SChannel
{
    class SSLEngine;
    using SSLEnginePtr = std::shared_ptr<SSLEngine>;
}

#endif

#endif
