//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "SSLAcceptorI.h"
#include "Ice/LocalException.h"
#include "SSLEndpointI.h"
#include "SSLEngine.h"
#include "SSLInstance.h"
#include "SSLUtil.h"

#if defined(_WIN32)
#    include "SChannelEngine.h"
#    include "SChannelTransceiverI.h"
#elif defined(__APPLE__)
#    include "SecureTransportTransceiverI.h"
#else
#    include "OpenSSLTransceiverI.h"
#endif

using namespace std;
using namespace Ice;
using namespace IceSSL;

IceInternal::NativeInfoPtr
IceSSL::AcceptorI::getNativeInfo()
{
    return _delegate->getNativeInfo();
}

#if defined(ICE_USE_IOCP)
IceInternal::AsyncInfo*
IceSSL::AcceptorI::getAsyncInfo(IceInternal::SocketOperation status)
{
    return _delegate->getNativeInfo()->getAsyncInfo(status);
}
#endif

void
IceSSL::AcceptorI::close()
{
    _delegate->close();
}

IceInternal::EndpointIPtr
IceSSL::AcceptorI::listen()
{
    _endpoint = _endpoint->endpoint(_delegate->listen());
    return _endpoint;
}

#if defined(ICE_USE_IOCP)
void
IceSSL::AcceptorI::startAccept()
{
    _delegate->startAccept();
}

void
IceSSL::AcceptorI::finishAccept()
{
    _delegate->finishAccept();
}
#endif

IceInternal::TransceiverPtr
IceSSL::AcceptorI::accept()
{
    optional<Ice::SSL::ServerAuthenticationOptions> serverAuthenticationOptions = _serverAuthenticationOptions;
    if (!serverAuthenticationOptions)
    {
        serverAuthenticationOptions = _instance->engine()->createServerAuthenticationOptions();
    }
    assert(serverAuthenticationOptions);
#if defined(_WIN32)
    return make_shared<IceSSL::SChannel::TransceiverI>(
        _instance,
        _delegate->accept(),
        _adapterName,
        *serverAuthenticationOptions);
#elif defined(__APPLE__)
    return make_shared<IceSSL::SecureTransport::TransceiverI>(
        _instance,
        _delegate->accept(),
        _adapterName,
        *serverAuthenticationOptions);
#else
    return make_shared<IceSSL::OpenSSL::TransceiverI>(
        _instance,
        _delegate->accept(),
        _adapterName,
        *serverAuthenticationOptions);
#endif
}

string
IceSSL::AcceptorI::protocol() const
{
    return _delegate->protocol();
}

string
IceSSL::AcceptorI::toString() const
{
    return _delegate->toString();
}

string
IceSSL::AcceptorI::toDetailedString() const
{
    return _delegate->toDetailedString();
}

IceSSL::AcceptorI::AcceptorI(
    const EndpointIPtr& endpoint,
    const InstancePtr& instance,
    const IceInternal::AcceptorPtr& del,
    const string& adapterName,
    const optional<Ice::SSL::ServerAuthenticationOptions>& serverAuthenticationOptions)
    : _endpoint(endpoint),
      _instance(instance),
      _delegate(del),
      _adapterName(adapterName),
      _serverAuthenticationOptions(serverAuthenticationOptions)
{
}

IceSSL::AcceptorI::~AcceptorI() {}
