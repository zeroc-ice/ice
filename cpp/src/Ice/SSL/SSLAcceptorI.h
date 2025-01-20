// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_ACCEPTOR_I_H
#define ICE_SSL_ACCEPTOR_I_H

#include "../Acceptor.h"
#include "../Network.h"
#include "../TransceiverF.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"
#include "SSLInstanceF.h"

#include <optional>
#include <vector>

namespace Ice::SSL
{
    class AcceptorI final : public IceInternal::Acceptor, public IceInternal::NativeInfo
    {
    public:
        AcceptorI(
            EndpointIPtr,
            InstancePtr,
            IceInternal::AcceptorPtr,
            std::string,
            const std::optional<Ice::SSL::ServerAuthenticationOptions>&);
        ~AcceptorI() override;
        IceInternal::NativeInfoPtr getNativeInfo() final;
#if defined(ICE_USE_IOCP)
        IceInternal::AsyncInfo* getAsyncInfo(IceInternal::SocketOperation) final;
#endif

        void close() final;
        IceInternal::EndpointIPtr listen() final;
#if defined(ICE_USE_IOCP)
        void startAccept() final;
        void finishAccept() final;
#endif
        IceInternal::TransceiverPtr accept() final;
        [[nodiscard]] std::string protocol() const final;
        [[nodiscard]] std::string toString() const final;
        [[nodiscard]] std::string toDetailedString() const final;

    private:
        EndpointIPtr _endpoint;
        const InstancePtr _instance;
        const IceInternal::AcceptorPtr _delegate;
        const std::string _adapterName;
        const std::optional<Ice::SSL::ServerAuthenticationOptions> _serverAuthenticationOptions;
    };
}

#endif
