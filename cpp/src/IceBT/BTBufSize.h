// Copyright (c) ZeroC, Inc.

#ifndef ICE_BT_BUF_SIZE_H
#define ICE_BT_BUF_SIZE_H

#include "Ice/Properties.h"

#include <cstdint>

namespace IceBT
{
    // The Bluetooth send and receive buffer sizes, as configured by the IceBT.SndSize and IceBT.RcvSize properties.
    class BTBufSize
    {
    public:
        // Reads the IceBT.RcvSize and IceBT.SndSize properties. Throws PropertyException if a value is not a valid
        // integer.
        explicit BTBufSize(const Ice::PropertiesPtr& properties)
            : _rcvSize(properties->getIcePropertyAsInt("IceBT.RcvSize")),
              _sndSize(properties->getIcePropertyAsInt("IceBT.SndSize"))
        {
        }

        [[nodiscard]] std::int32_t rcvSize() const noexcept { return _rcvSize; }
        [[nodiscard]] std::int32_t sndSize() const noexcept { return _sndSize; }

    private:
        std::int32_t _rcvSize;
        std::int32_t _sndSize;
    };
}

#endif
