// Copyright (c) ZeroC, Inc.

#ifndef ICE_NETWORK_FRAMEWORK_UDP_TRANSCEIVER_H
#define ICE_NETWORK_FRAMEWORK_UDP_TRANSCEIVER_H

#include "../EndpointIF.h"
#include "../Network.h"
#include "../ProtocolInstanceF.h"
#include "../Transceiver.h"

#include <Network/Network.h>
#include <dispatch/dispatch.h>

#include <atomic>
#include <deque>
#include <mutex>
#include <set>
#include <vector>

namespace IceInternal
{
    class NetworkFrameworkUdpTransceiver final : public Transceiver
    {
    public:
        // Client constructor (connected UDP via connector)
        NetworkFrameworkUdpTransceiver(ProtocolInstancePtr, nw_connection_t);

        // Server constructor (listener UDP via endpoint->transceiver())
        NetworkFrameworkUdpTransceiver(
            UdpEndpointIPtr,
            ProtocolInstancePtr,
            const std::string& host,
            int port,
            std::string mcastInterface);

        ~NetworkFrameworkUdpTransceiver();

        NativeInfoPtr getNativeInfo() final;

        SocketOperation initialize(Buffer&, Buffer&) final;
        SocketOperation closing(bool, std::exception_ptr) final;

        void close() final;
        EndpointIPtr bind() final;
        SocketOperation write(Buffer&) final;
        SocketOperation read(Buffer&) final;

        bool startWrite(Buffer&) final;
        void finishWrite(Buffer&) final;
        void startRead(Buffer&) final;
        void finishRead(Buffer&) final;

        [[nodiscard]] std::string protocol() const final;
        [[nodiscard]] std::string toString() const final;
        [[nodiscard]] std::string toDetailedString() const final;
        [[nodiscard]] Ice::ConnectionInfoPtr
        getInfo(bool incoming, std::string adapterName, std::string connectionId) const final;
        void checkSendSize(const Buffer&) final;
        void setBufferSize(int rcvSize, int sndSize) final;

        [[nodiscard]] int effectivePort() const;

        // Shared server state — captures datagrams from all peer connections.
        // Public so that the startPeerReceive() free function can use it.
        struct ServerState
        {
            std::mutex mutex;
            struct Datagram
            {
                std::vector<std::byte> data;
                nw_connection_t source; // May be nullptr for multicast datagrams.
            };
            std::deque<Datagram> received;
            bool readWaiting{false};
            // Active peer connections
            std::vector<nw_connection_t> connections;
        };
    private:
        enum State
        {
            StateNeedConnect,
            StateConnectPending,
            StateConnected,
            StateNotConnected
        };

        UdpEndpointIPtr _endpoint; // Server only
        const ProtocolInstancePtr _instance;
        NativeInfoPtr _nativeInfo;

        nw_connection_t _connection;               // Client only
        nw_listener_t _listener;                   // Server only (unicast)
        SOCKET _mcastFd{INVALID_SOCKET};           // Server only (multicast) — BSD socket
        dispatch_source_t _mcastReadSource;        // Server only (multicast) — dispatch source for read
        dispatch_queue_t _dispatchQueue;

        const bool _incoming;
        State _state;
        std::string _host;
        uint16_t _port;
        std::string _mcastInterface;

        // Connect state (client only) — same pattern as TCP
        struct ConnectState
        {
            std::atomic<bool> connected{false};
            std::atomic<int> error{0};
        };
        std::shared_ptr<ConnectState> _connectState;

        std::shared_ptr<ServerState> _serverState;

        // Write state
        struct WriteState
        {
            std::mutex mutex;
            int error{0};
        };
        std::shared_ptr<WriteState> _writeState;

        // Read state (client only, for connected receive)
        struct ReadState
        {
            std::mutex mutex;
            std::vector<std::byte> data;
            int error{0};
        };
        std::shared_ptr<ReadState> _readState;

        // Server: connection that delivered the last datagram (for replies)
        nw_connection_t _currentPeer;

        int _rcvSize;
        int _sndSize;
        static const int _udpOverhead;
        static const int _maxPacketSize;
    };
}

#endif
