// Copyright (c) ZeroC, Inc.

#ifndef ICE_NETWORK_FRAMEWORK_ACCEPTOR_H
#define ICE_NETWORK_FRAMEWORK_ACCEPTOR_H

#include "../Acceptor.h"
#include "../Network.h"
#include "../ProtocolInstanceF.h"
#include "../TransceiverF.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"
#include "ObjectRef.h"

#include <Network/Network.h>
#include <dispatch/dispatch.h>

#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>

namespace IceInternal
{
    class NetworkFrameworkAcceptor final : public Acceptor,
                                           public std::enable_shared_from_this<NetworkFrameworkAcceptor>
    {
    public:
        NetworkFrameworkAcceptor(
            TcpEndpointIPtr,
            const ProtocolInstancePtr&,
            const std::string& host,
            int port,
            const std::string& adapterName = "",
            const std::optional<Ice::SSL::ServerAuthenticationOptions>& = std::nullopt);
        ~NetworkFrameworkAcceptor() override = default;

        NativeInfoPtr getNativeInfo() final;

        void close() final;
        EndpointIPtr listen() final;
        void startAccept() final;
        void finishAccept() final;

        TransceiverPtr accept() final;
        [[nodiscard]] std::string protocol() const final;
        [[nodiscard]] std::string toString() const final;
        [[nodiscard]] std::string toDetailedString() const final;

        [[nodiscard]] int effectivePort() const;

    private:
        friend class TcpEndpointI;

        TcpEndpointIPtr _endpoint;
        const ProtocolInstancePtr _instance;
        NativeInfoPtr _nativeInfo;

        NetworkRef<nw_listener_t> _listener;
        DispatchRef<dispatch_queue_t> _dispatchQueue;

        std::string _host;
        uint16_t _port;
        bool _secure{false};

        /// Whether listen() started the listener; only a started listener reports the cancelled state to close().
        bool _listenerStarted{false};

        //
        // Shared state for accepted connections — accessed by dispatch blocks
        // and the Ice thread pool. Protected by its own mutex.
        //
        struct AcceptState
        {
            std::mutex mutex;
            std::deque<NetworkRef<nw_connection_t>> connections; // Owned until a transceiver adopts them.
            bool waiting{false};

            /// The last state reported by the listener's state changed handler; invalid until the listener reports
            /// its first state after nw_listener_start.
            nw_listener_state_t listenerState{nw_listener_state_invalid};

            /// The POSIX error code reported with the last waiting or failed state, or 0 if none was reported.
            int listenerError{0};

            /// Whether accept() has already reported the failure of the listener.
            bool listenerFailureReported{false};

            /// Notified each time the listener reports a new state.
            std::condition_variable listenerStateChanged;
        };
        std::shared_ptr<AcceptState> _acceptState;
    };
}

#endif
