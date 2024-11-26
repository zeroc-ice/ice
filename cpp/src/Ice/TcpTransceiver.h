//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_TCP_TRANSCEIVER_H
#define ICE_TCP_TRANSCEIVER_H

#include "Network.h"
#include "ProtocolInstanceF.h"
#include "StreamSocket.h"
#include "Transceiver.h"

namespace IceInternal
{
    class TcpConnector;
    class TcpAcceptor;

    class TcpTransceiver final : public Transceiver
    {
    public:
        TcpTransceiver(const ProtocolInstancePtr&, const StreamSocketPtr&);
        ~TcpTransceiver();
        NativeInfoPtr getNativeInfo() final;

        SocketOperation initialize(Buffer&, Buffer&) final;
        SocketOperation closing(bool, std::exception_ptr) final;

        void close() final;
        SocketOperation write(Buffer&) final;
        SocketOperation read(Buffer&) final;
#if defined(ICE_USE_IOCP)
        bool startWrite(Buffer&) final;
        void finishWrite(Buffer&) final;
        void startRead(Buffer&) final;
        void finishRead(Buffer&) final;
#endif

        std::string protocol() const final;
        std::string toString() const final;
        std::string toDetailedString() const final;
        Ice::ConnectionInfoPtr getInfo(bool incoming, std::string adapterName, std::string connectionId) const final;
        void checkSendSize(const Buffer&) final;
        void setBufferSize(int rcvSize, int sndSize) final;

    private:
        friend class TcpConnector;
        friend class TcpAcceptor;

        const ProtocolInstancePtr _instance;
        const StreamSocketPtr _stream;
    };
}

#endif
