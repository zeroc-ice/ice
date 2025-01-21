// Copyright (c) ZeroC, Inc.

#include "WSAcceptor.h"
#include "WSEndpoint.h"
#include "WSTransceiver.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::NativeInfoPtr
IceInternal::WSAcceptor::getNativeInfo()
{
    return _delegate->getNativeInfo();
}

#if defined(ICE_USE_IOCP)
IceInternal::AsyncInfo*
IceInternal::WSAcceptor::getAsyncInfo(IceInternal::SocketOperation status)
{
    return _delegate->getNativeInfo()->getAsyncInfo(status);
}
#endif

void
IceInternal::WSAcceptor::close()
{
    _delegate->close();
}

EndpointIPtr
IceInternal::WSAcceptor::listen()
{
    _endpoint = _endpoint->endpoint(_delegate->listen());
    return _endpoint;
}

#if defined(ICE_USE_IOCP)
void
IceInternal::WSAcceptor::startAccept()
{
    _delegate->startAccept();
}

void
IceInternal::WSAcceptor::finishAccept()
{
    _delegate->finishAccept();
}
#endif

IceInternal::TransceiverPtr
IceInternal::WSAcceptor::accept()
{
    //
    // WebSocket handshaking is performed in TransceiverI::initialize, since
    // accept must not block.
    //
    return make_shared<WSTransceiver>(_instance, _delegate->accept());
}

string
IceInternal::WSAcceptor::protocol() const
{
    return _delegate->protocol();
}

string
IceInternal::WSAcceptor::toString() const
{
    return _delegate->toString();
}

string
IceInternal::WSAcceptor::toDetailedString() const
{
    return _delegate->toDetailedString();
}

IceInternal::WSAcceptor::WSAcceptor(WSEndpointPtr endpoint, ProtocolInstancePtr instance, AcceptorPtr del)
    : _endpoint(std::move(endpoint)),
      _instance(std::move(instance)),
      _delegate(std::move(del))
{
}

IceInternal::WSAcceptor::~WSAcceptor() = default;
