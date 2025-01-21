// Copyright (c) ZeroC, Inc.

#ifndef ICE_UDP_TRANSCEIVER_H
#define ICE_UDP_TRANSCEIVER_H

#include "Network.h"
#include "ProtocolInstanceF.h"
#include "Transceiver.h"

namespace IceInternal
{
    class UdpEndpoint;

    class UdpTransceiver final : public Transceiver,
                                 public NativeInfo,
                                 public std::enable_shared_from_this<UdpTransceiver>
    {
        enum State
        {
            StateNeedConnect,
            StateConnectPending,
            StateConnected,
            StateNotConnected
        };

    public:
        UdpTransceiver(ProtocolInstancePtr, const Address&, const Address&, const std::string&, int);
        UdpTransceiver(UdpEndpointIPtr, const ProtocolInstancePtr&, const std::string&, int, std::string);

        ~UdpTransceiver() final;

        NativeInfoPtr getNativeInfo() final;
#if defined(ICE_USE_IOCP)
        AsyncInfo* getAsyncInfo(SocketOperation) final;
#endif

        SocketOperation initialize(Buffer&, Buffer&) final;
        SocketOperation closing(bool, std::exception_ptr) final;
        void close() final;
        EndpointIPtr bind() final;
        SocketOperation write(Buffer&) final;
        SocketOperation read(Buffer&) final;
#if defined(ICE_USE_IOCP)
        bool startWrite(Buffer&) final;
        void finishWrite(Buffer&) final;
        void startRead(Buffer&) final;
        void finishRead(Buffer&) final;
#endif
        [[nodiscard]] std::string protocol() const final;
        [[nodiscard]] std::string toString() const final;
        [[nodiscard]] std::string toDetailedString() const final;
        [[nodiscard]] Ice::ConnectionInfoPtr
        getInfo(bool incoming, std::string adapterName, std::string connectionId) const final;
        void checkSendSize(const Buffer&) final;
        void setBufferSize(int rcvSize, int sndSize) final;

        [[nodiscard]] int effectivePort() const;

    private:
        void setBufSize(int, int);

        UdpEndpointIPtr _endpoint;
        const ProtocolInstancePtr _instance;
        const bool _incoming;
        bool _bound;

        const Address _addr;
        Address _mcastAddr;
        const std::string _mcastInterface;
        Address _peerAddr;

#if defined(_WIN32)
        int _port;
#endif

        State _state;
        int _rcvSize;
        int _sndSize;
        static const int _udpOverhead;
        static const int _maxPacketSize;

#if defined(ICE_USE_IOCP)
        AsyncInfo _read;
        AsyncInfo _write;
        Address _readAddr;
        socklen_t _readAddrLen;
#endif
    };
}
#endif
