//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ACCEPTOR_F_H
#define ICE_ACCEPTOR_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class Acceptor;
using AcceptorPtr = std::shared_ptr<Acceptor>;

class TcpAcceptor;
using TcpAcceptorPtr = std::shared_ptr<TcpAcceptor>;

}

#endif
