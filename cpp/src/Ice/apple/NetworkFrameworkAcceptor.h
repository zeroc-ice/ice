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

        //
        // Shared state for accepted connections — accessed by dispatch blocks
        // and the Ice thread pool. Protected by its own mutex.
        //
        struct AcceptState
        {
            std::mutex mutex;
            std::deque<NetworkRef<nw_connection_t>> connections; // Owned until a transceiver adopts them.
            bool waiting{false};
        };
        std::shared_ptr<AcceptState> _acceptState;
    };
}

#endif
