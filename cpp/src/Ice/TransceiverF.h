// Copyright (c) ZeroC, Inc.

#ifndef ICE_TRANSCEIVER_F_H
#define ICE_TRANSCEIVER_F_H

#include <memory>

namespace IceInternal
{
    class Transceiver;
    using TransceiverPtr = std::shared_ptr<Transceiver>;

    class TcpTransceiver;
    using TcpTransceiverPtr = std::shared_ptr<TcpTransceiver>;

    class UdpTransceiver;
    using UdpTransceiverPtr = std::shared_ptr<UdpTransceiver>;

    class WSTransceiver;
    using WSTransceiverPtr = std::shared_ptr<WSTransceiver>;
}

#endif
