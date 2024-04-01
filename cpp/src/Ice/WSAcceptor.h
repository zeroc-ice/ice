//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_WS_ACCEPTOR_I_H
#define ICE_WS_ACCEPTOR_I_H

#include "Ice/Logger.h"
#include "TransceiverF.h"
#include "Acceptor.h"
#include "Network.h"
#include "ProtocolInstance.h"

namespace IceInternal
{
    class WSEndpoint;

    class WSAcceptor final : public Acceptor, public NativeInfo
    {
    public:
        WSAcceptor(const WSEndpointPtr&, const ProtocolInstancePtr&, const AcceptorPtr&);
        ~WSAcceptor();
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
        std::string protocol() const final;
        std::string toString() const final;
        std::string toDetailedString() const final;

    private:
        friend class WSEndpoint;

        WSEndpointPtr _endpoint;
        const ProtocolInstancePtr _instance;
        const AcceptorPtr _delegate;
    };
}

#endif
