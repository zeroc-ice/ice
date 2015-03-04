// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Acceptor.h>
#include <Transceiver.h>

using namespace std;

IceInternal::NativeInfoPtr
Acceptor::getNativeInfo()
{
    return _acceptor->getNativeInfo();
}

void
Acceptor::close()
{
    _acceptor->close();
}

void
Acceptor::listen()
{
    _acceptor->listen();
}

#ifdef ICE_USE_IOCP
void 
Acceptor::startAccept()
{
    _acceptor->startAccept();
}

void
Acceptor::finishAccept()
{
    _acceptor->finishAccept();
}
#endif

IceInternal::TransceiverPtr
Acceptor::accept()
{
    return new Transceiver(_acceptor->accept());
}

string
Acceptor::toString() const
{
    return _acceptor->toString();
}

Acceptor::Acceptor(const IceInternal::AcceptorPtr& acceptor) : _acceptor(acceptor)
{
}

