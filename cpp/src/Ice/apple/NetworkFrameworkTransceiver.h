// Copyright (c) ZeroC, Inc.

#ifndef ICE_NETWORK_FRAMEWORK_TRANSCEIVER_H
#define ICE_NETWORK_FRAMEWORK_TRANSCEIVER_H

#include "../Network.h"
#include "../NetworkProxyF.h"
#include "../ProtocolInstanceF.h"
#include "../Transceiver.h"
#include "AsyncOperation.h"
#include "Ice/SSL/ConnectionInfoF.h"
#include "ObjectRef.h"

#include <Network/Network.h>
#include <dispatch/dispatch.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

namespace IceInternal::NetworkFrameworkTLS
{
    class PeerRejection;
}

namespace IceInternal
{
    // Completion-based transceiver for the TCP and TLS connections of Network.framework. The thread pool starts an
    // asynchronous connect, read or write operation; the transceiver starts the corresponding Network.framework
    // operation, whose completion block records the result and posts one completion through the NativeInfo; the
    // thread pool then finishes the operation and the transceiver consumes the result. Once connected, write()
    // hands the data to Network.framework and completes synchronously as long as the data whose send completion has
    // not run yet stays below a bound (sendCapacity in the implementation); the write operation only waits for room.
    class NetworkFrameworkTransceiver final : public Transceiver
    {
    public:
        // The outstanding operations and their results, shared between the transceiver and the blocks
        // Network.framework invokes on the connection's dispatch queue. The blocks only use this state, never the
        // transceiver, so they can run after the transceiver is destroyed. The read and write operations are
        // independent: both can be pending at the same time.
        struct AsyncState
        {
            struct ConnectResult
            {
                int error{0};         // 0 when the connection is established.
                bool tlsError{false}; // The error is a TLS handshake failure.
            };

            struct ReadResult
            {
                // The received content, null on error and when the peer closed the connection. The transceiver
                // copies it into the connection's buffer; the block that receives it cannot, because the buffer can
                // be gone by the time a late block runs.
                DispatchRef<dispatch_data_t> data;
                size_t size{0}; // The size of data, 0 when data is null.
                int error{0};
            };

            struct WriteResult
            {
                // During the proxy handshake, the number of bytes sent, 0 on error. Once connected, the write
                // operation only waits for room to hand more data to Network.framework and count is 0.
                size_t count{0};
                int error{0};
            };

            explicit AsyncState(NativeInfoPtr);
            AsyncState(const AsyncState&) = delete;
            AsyncState& operator=(const AsyncState&) = delete;

            std::mutex mutex; // Protects the operations and the send state below.
            const NativeInfoPtr nativeInfo;
            AsyncOperation<ConnectResult> connect;
            AsyncOperation<ReadResult> read;
            AsyncOperation<WriteResult> write;

            // The send state of a connected connection, see write(). unsent counts the bytes write() accepted whose
            // send completion has not run yet: the outstanding send and queuedSend, the data written while a send is
            // outstanding. sendConnection is set while a send is outstanding: it keeps the connection alive for the
            // completion, which sends the queued data. sendError is the error of the first send that failed,
            // reported by the next write.
            size_t unsent{0};
            DispatchRef<dispatch_data_t> queuedSend;
            NetworkRef<nw_connection_t> sendConnection;
            int sendError{0};

            // Set by close() while sends are in flight: the connection to cancel once they all completed, see close().
            NetworkRef<nw_connection_t> cancelAfterSends;
        };

        // With a network proxy, the connection is established with the proxy and, for plain TCP connections, the
        // transceiver performs the proxy handshake for the destination address (addr) during initialize(). For
        // secure connections Network.framework performs the proxy handshake itself; the proxy is only used to
        // describe the connection.
        // The transceiver owns the connection it is given.
        NetworkFrameworkTransceiver(
            ProtocolInstancePtr,
            NetworkRef<nw_connection_t>,
            bool secure = false,
            NetworkProxyPtr proxy = nullptr,
            const Address& addr = Address());
        ~NetworkFrameworkTransceiver() = default;

        NativeInfoPtr getNativeInfo() final;

        SocketOperation initialize(Buffer&, Buffer&) final;
        SocketOperation closing(bool, std::exception_ptr) final;

        void close() final;
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

        // Sets the rejection the TLS verify block of an outgoing connection records, see initialize.
        void setPeerRejection(std::shared_ptr<NetworkFrameworkTLS::PeerRejection> rejection)
        {
            _peerRejection = std::move(rejection);
        }

        // Sets the function that verifies the peer of a TLS connection once the handshake completed and the peer did
        // not present a certificate. TLS only validates the certificate the peer presents; the SSL engine's trust
        // rules (IceSSL.TrustOnly and friends) must also run when there is no certificate, otherwise a rule that
        // requires a specific client would let an anonymous client through. When the peer presents a certificate,
        // the TLS verify block already runs these rules. The function throws to reject the connection.
        void setPeerVerifier(
            std::function<void(const Ice::SSL::ConnectionInfoPtr&)> verifier,
            bool incoming,
            std::string adapterName)
        {
            _peerVerifier = std::move(verifier);
            _incoming = incoming;
            _adapterName = std::move(adapterName);
        }

    private:
        enum State
        {
            StateNeedsConnect,
            StateConnectPending,
            StateProxyWrite,     // Sending the proxy connection request.
            StateProxyRead,      // Reading the proxy response.
            StateProxyConnected, // The proxy accepted the request.
            StateConnected
        };

        [[nodiscard]] std::string describe() const;
        [[nodiscard]] static State toState(SocketOperation);

        // Hands as much of buf to Network.framework as the send capacity allows and advances buf.i accordingly,
        // possibly not at all. Throws ConnectionLostException when a previous send failed.
        void sendAvailable(Buffer& buf);

        // Fills buf from the read-ahead data, as far as both go, and releases the data once it is all consumed.
        void consumeReadAhead(Buffer& buf);

        // Reports to the thread pool whether read-ahead data is available, when that changed since the last report.
        void updateReadReady();

        const ProtocolInstancePtr _instance;
        const NetworkProxyPtr _proxy;
        const Address _addr; // The destination address, only used with a network proxy.
        NativeInfoPtr _nativeInfo;

        const NetworkRef<nw_connection_t> _connection;
        const DispatchRef<dispatch_queue_t> _dispatchQueue;
        const std::shared_ptr<AsyncState> _async;
        State _state{StateNeedsConnect};

        // The bytes a receive delivered beyond what the connection asked for: _readAhead holds the received content
        // and the unconsumed bytes are [_readAheadOffset, _readAheadSize). read() serves them before a new receive
        // is started, so a receive can deliver several messages at once. Only the connection's thread pool threads
        // use them, serialized by the connection, never the blocks.
        DispatchRef<dispatch_data_t> _readAhead;
        size_t _readAheadOffset{0};
        size_t _readAheadSize{0};

        // The number of bytes the pending or last receive was started for, see finishRead.
        size_t _readLength{0};

        // The read readiness last reported through the NativeInfo, true while read-ahead data is available. It lets
        // the thread pool dispatch the buffered messages without starting a receive. See updateReadReady.
        bool _readReady{false};

        // Set by close(). Like the other members, only used by the threads the connection serializes.
        bool _closed{false};

        std::string _desc;
        bool _secure;

        int _rcvSize{0};
        int _sndSize{0};

        // Shared with the TLS verify block of an outgoing connection, which records why it rejected the server. Null
        // for an incoming connection: the verify block of a listener is shared by the connections it accepts.
        std::shared_ptr<NetworkFrameworkTLS::PeerRejection> _peerRejection;

        // See setPeerVerifier.
        std::function<void(const Ice::SSL::ConnectionInfoPtr&)> _peerVerifier;
        bool _incoming{false};
        std::string _adapterName;
    };

    // Creates the parameters of the Network.framework TCP and TLS connections and listeners. Every connection gets
    // the TCP options the BSD-socket transports set on their sockets: Nagle's algorithm disabled and keepalive
    // enabled with the system default timings. The connections a listener accepts inherit the listener's options.
    // configureTLS configures the TLS protocol, or is NW_PARAMETERS_DISABLE_PROTOCOL for plain TCP; it is invoked
    // synchronously. Returns null if Network.framework cannot create the parameters.
    NetworkRef<nw_parameters_t> createTcpParameters(nw_parameters_configure_protocol_block_t configureTLS);
}

#endif
