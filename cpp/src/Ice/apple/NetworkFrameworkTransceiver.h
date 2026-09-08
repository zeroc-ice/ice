// Copyright (c) ZeroC, Inc.

#ifndef ICE_NETWORK_FRAMEWORK_TRANSCEIVER_H
#define ICE_NETWORK_FRAMEWORK_TRANSCEIVER_H

#include "../Network.h"
#include "../NetworkProxyF.h"
#include "../ProtocolInstanceF.h"
#include "../Transceiver.h"
#include "Ice/SSL/ConnectionInfoF.h"

#include <Network/Network.h>
#include <dispatch/dispatch.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace IceInternal
{
    //
    // Shared async I/O state for Network.framework completion blocks.
    // These structs are held by shared_ptr to ensure they outlive the transceiver
    // if a completion block fires after the transceiver is destroyed.
    //
    struct ReadState
    {
        std::mutex mutex;
        std::vector<std::byte> data;
        int error{0};
    };

    struct WriteState
    {
        std::mutex mutex;
        size_t count{0};
        int error{0};
    };

    struct ConnectState
    {
        std::atomic<bool> connected{false};
        std::atomic<int> error{0};
        std::atomic<bool> tlsError{false};
    };

    class NetworkFrameworkTransceiver final : public Transceiver
    {
    public:
        // With a network proxy, the connection is established with the proxy and, for plain TCP connections, the
        // transceiver performs the proxy handshake for the destination address (addr) during initialize(). For
        // secure connections Network.framework performs the proxy handshake itself; the proxy is only used to
        // describe the connection.
        NetworkFrameworkTransceiver(
            ProtocolInstancePtr,
            nw_connection_t,
            bool secure = false,
            NetworkProxyPtr proxy = nullptr,
            const Address& addr = Address());
        ~NetworkFrameworkTransceiver();

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

        void setLocalVerifyRejected(std::shared_ptr<std::atomic<bool>> flag) { _localVerifyRejected = std::move(flag); }

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

        const ProtocolInstancePtr _instance;
        const NetworkProxyPtr _proxy;
        const Address _addr; // The destination address, only used with a network proxy.
        NativeInfoPtr _nativeInfo;

        nw_connection_t _connection;
        dispatch_queue_t _dispatchQueue;
        State _state{StateNeedsConnect};

        // Shared async state — captured by completion blocks via shared_ptr.
        std::shared_ptr<ConnectState> _connectState;
        std::shared_ptr<ReadState> _readState;
        std::shared_ptr<WriteState> _writeState;

        std::string _desc;
        bool _secure;

        int _rcvSize{0};
        int _sndSize{0};

        // Optional flag shared with the verify block in SSLConnectorI. When the
        // local verify block calls complete(false), it sets this flag so we can
        // distinguish "we rejected the peer" (SecurityException) from "the peer
        // rejected us" (ConnectionLostException).
        std::shared_ptr<std::atomic<bool>> _localVerifyRejected;

        // See setPeerVerifier.
        std::function<void(const Ice::SSL::ConnectionInfoPtr&)> _peerVerifier;
        bool _incoming{false};
        std::string _adapterName;
    };
}

#endif
