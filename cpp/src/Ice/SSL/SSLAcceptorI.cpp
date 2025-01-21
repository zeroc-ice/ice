// Copyright (c) ZeroC, Inc.

#include "SSLAcceptorI.h"
#include "SSLEndpointI.h"
#include "SSLEngine.h"
#include "SSLInstance.h"

#if defined(_WIN32)
#    include "SchannelEngine.h"
#    include "SchannelTransceiverI.h"
#elif defined(__APPLE__)
#    include "SecureTransportTransceiverI.h"
#else
#    include "OpenSSLTransceiverI.h"
#endif

using namespace std;
using namespace Ice;
using namespace Ice::SSL;

IceInternal::NativeInfoPtr
Ice::SSL::AcceptorI::getNativeInfo()
{
    return _delegate->getNativeInfo();
}

#if defined(ICE_USE_IOCP)
IceInternal::AsyncInfo*
Ice::SSL::AcceptorI::getAsyncInfo(IceInternal::SocketOperation status)
{
    return _delegate->getNativeInfo()->getAsyncInfo(status);
}
#endif

void
Ice::SSL::AcceptorI::close()
{
    _delegate->close();
}

IceInternal::EndpointIPtr
Ice::SSL::AcceptorI::listen()
{
    _endpoint = _endpoint->endpoint(_delegate->listen());
    return _endpoint;
}

#if defined(ICE_USE_IOCP)
void
Ice::SSL::AcceptorI::startAccept()
{
    _delegate->startAccept();
}

void
Ice::SSL::AcceptorI::finishAccept()
{
    _delegate->finishAccept();
}
#endif

IceInternal::TransceiverPtr
Ice::SSL::AcceptorI::accept()
{
    optional<Ice::SSL::ServerAuthenticationOptions> serverAuthenticationOptions = _serverAuthenticationOptions;
    if (!serverAuthenticationOptions)
    {
        serverAuthenticationOptions = _instance->engine()->createServerAuthenticationOptions();
    }
    assert(serverAuthenticationOptions);
#if defined(_WIN32)
    return make_shared<Ice::SSL::Schannel::TransceiverI>(
        _instance,
        _delegate->accept(),
        _adapterName,
        *serverAuthenticationOptions);
#elif defined(__APPLE__)
    return make_shared<Ice::SSL::SecureTransport::TransceiverI>(
        _instance,
        _delegate->accept(),
        _adapterName,
        *serverAuthenticationOptions);
#else
    return make_shared<Ice::SSL::OpenSSL::TransceiverI>(
        _instance,
        _delegate->accept(),
        _adapterName,
        *serverAuthenticationOptions);
#endif
}

string
Ice::SSL::AcceptorI::protocol() const
{
    return _delegate->protocol();
}

string
Ice::SSL::AcceptorI::toString() const
{
    return _delegate->toString();
}

string
Ice::SSL::AcceptorI::toDetailedString() const
{
    return _delegate->toDetailedString();
}

Ice::SSL::AcceptorI::AcceptorI(
    EndpointIPtr endpoint,
    InstancePtr instance,
    IceInternal::AcceptorPtr del,
    string adapterName,
    const optional<Ice::SSL::ServerAuthenticationOptions>& serverAuthenticationOptions)
    : _endpoint(std::move(endpoint)),
      _instance(std::move(instance)),
      _delegate(std::move(del)),
      _adapterName(std::move(adapterName)),
      _serverAuthenticationOptions(serverAuthenticationOptions)
{
}

Ice::SSL::AcceptorI::~AcceptorI() = default;
