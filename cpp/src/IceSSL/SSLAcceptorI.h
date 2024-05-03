//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_ACCEPTOR_I_H
#define ICESSL_ACCEPTOR_I_H

#include "../Ice/Acceptor.h"
#include "../Ice/Network.h"
#include "../Ice/TransceiverF.h"
#include "Ice/ServerAuthenticationOptions.h"
#include "SSLInstanceF.h"

#include <optional>
#include <vector>

namespace IceSSL
{
    class AcceptorI final : public IceInternal::Acceptor, public IceInternal::NativeInfo
    {
    public:
        AcceptorI(
            const EndpointIPtr&,
            const InstancePtr&,
            const IceInternal::AcceptorPtr&,
            const std::string&,
            const std::optional<Ice::SSL::ServerAuthenticationOptions>&);
        ~AcceptorI();
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
        std::string protocol() const final;
        std::string toString() const final;
        std::string toDetailedString() const final;

    private:
        EndpointIPtr _endpoint;
        const InstancePtr _instance;
        const IceInternal::AcceptorPtr _delegate;
        const std::string _adapterName;
        const std::optional<Ice::SSL::ServerAuthenticationOptions> _serverAuthenticationOptions;
    };
}

#endif // IceSSL namespace end
