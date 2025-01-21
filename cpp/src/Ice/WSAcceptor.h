// Copyright (c) ZeroC, Inc.

#ifndef ICE_WS_ACCEPTOR_I_H
#define ICE_WS_ACCEPTOR_I_H

#include "Acceptor.h"
#include "Ice/Logger.h"
#include "Network.h"
#include "ProtocolInstance.h"
#include "TransceiverF.h"

namespace IceInternal
{
    class WSEndpoint;

    class WSAcceptor final : public Acceptor, public NativeInfo
    {
    public:
        WSAcceptor(WSEndpointPtr, ProtocolInstancePtr, AcceptorPtr);
        ~WSAcceptor() override;
        NativeInfoPtr getNativeInfo() final;
#if defined(ICE_USE_IOCP)
        AsyncInfo* getAsyncInfo(SocketOperation) final;
#endif

        void close() final;
        EndpointIPtr listen() final;
#if defined(ICE_USE_IOCP)
        void startAccept() final;
        void finishAccept() final;
#endif
        TransceiverPtr accept() final;
        [[nodiscard]] std::string protocol() const final;
        [[nodiscard]] std::string toString() const final;
        [[nodiscard]] std::string toDetailedString() const final;

    private:
        friend class WSEndpoint;

        WSEndpointPtr _endpoint;
        const ProtocolInstancePtr _instance;
        const AcceptorPtr _delegate;
    };
}

#endif
