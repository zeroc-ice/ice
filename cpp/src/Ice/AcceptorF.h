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
}

#endif
