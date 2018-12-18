// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_ACCEPTOR_F_H
#define ICE_ACCEPTOR_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class Acceptor;
ICE_API IceUtil::Shared* upCast(Acceptor*);
typedef Handle<Acceptor> AcceptorPtr;

class TcpAcceptor;
ICE_API IceUtil::Shared* upCast(TcpAcceptor*);
typedef Handle<TcpAcceptor> TcpAcceptorPtr;

}

#endif
