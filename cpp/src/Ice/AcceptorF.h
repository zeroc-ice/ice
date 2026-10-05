// Copyright (c) ZeroC, Inc.

#ifndef ICE_ACCEPTOR_F_H
#define ICE_ACCEPTOR_F_H

#include <memory>

namespace IceInternal
{
    class Acceptor;
    using AcceptorPtr = std::shared_ptr<Acceptor>;

    class TcpAcceptor;
    using TcpAcceptorPtr = std::shared_ptr<TcpAcceptor>;

#if defined(__APPLE__)
    class NetworkFrameworkAcceptor;
    using NetworkFrameworkAcceptorPtr = std::shared_ptr<NetworkFrameworkAcceptor>;
#endif
}

#endif
