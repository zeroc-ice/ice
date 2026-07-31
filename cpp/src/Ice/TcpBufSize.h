// Copyright (c) ZeroC, Inc.

#ifndef ICE_TCP_BUF_SIZE_H
#define ICE_TCP_BUF_SIZE_H

#include "Ice/Properties.h"

#include <cstdint>

namespace IceInternal
{
    // The TCP send and receive buffer sizes, as configured by the Ice.TCP.SndSize and Ice.TCP.RcvSize properties.
    class TcpBufSize
    {
    public:
        // Reads the Ice.TCP.RcvSize and Ice.TCP.SndSize properties. Throws PropertyException if a value is not a
        // valid integer.
        explicit TcpBufSize(const Ice::PropertiesPtr& properties)
            : _rcvSize(properties->getPropertyAsIntWithDefault("Ice.TCP.RcvSize", dfltBufSize)),
              _sndSize(properties->getPropertyAsIntWithDefault("Ice.TCP.SndSize", dfltBufSize))
        {
        }

        [[nodiscard]] std::int32_t rcvSize() const noexcept { return _rcvSize; }
        [[nodiscard]] std::int32_t sndSize() const noexcept { return _sndSize; }

    private:
        // By default, on Windows we use a 128KB buffer size. On Unix platforms, we use the system defaults.
#ifdef _WIN32
        static constexpr std::int32_t dfltBufSize = 128 * 1024;
#else
        static constexpr std::int32_t dfltBufSize = 0;
#endif

        std::int32_t _rcvSize;
        std::int32_t _sndSize;
    };
}

#endif
