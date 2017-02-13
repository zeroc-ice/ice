// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Acceptor.h>
#include <Transceiver.h>
#include <EndpointI.h>

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

IceInternal::EndpointIPtr
Acceptor::listen()
{
    _endpoint = _endpoint->endpoint(_acceptor->listen());
    return _endpoint;
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
Acceptor::protocol() const
{
    return _acceptor->protocol();
}

string
Acceptor::toString() const
{
    return _acceptor->toString();
}

string
Acceptor::toDetailedString() const
{
    return _acceptor->toDetailedString();
}

Acceptor::Acceptor(const EndpointIPtr& endpoint, const IceInternal::AcceptorPtr& acceptor) :
    _endpoint(endpoint),
    _acceptor(acceptor)
{
}
