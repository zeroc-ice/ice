//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "SSLConnectorI.h"
#include "SSLInstance.h"

#include "../Ice/NetworkProxy.h"
#include "../Ice/StreamSocket.h"
#include "Ice/Communicator.h"
#include "Ice/LocalException.h"
#include "Ice/LoggerUtil.h"
#include "Ice/SSL.h"
#include "SChannelEngine.h"
#include "SChannelTransceiverI.h"
#include "SSLEndpointI.h"
#include "SSLEngine.h"
#include "SSLUtil.h"

using namespace std;
using namespace Ice;
using namespace IceSSL;

IceInternal::TransceiverPtr
IceSSL::ConnectorI::connect()
{
#if defined(_WIN32)
    optional<Ice::SSL::ClientAuthenticationOptions> clientAuthenticationOptions =
        _instance->engine()->getInitializationData().clientAuthenticationOptions;
    if (!clientAuthenticationOptions)
    {
        clientAuthenticationOptions =
            dynamic_pointer_cast<SChannel::SSLEngine>(_instance->engine())->createClientAuthenticationOptions();
    }
    assert(clientAuthenticationOptions);
    return make_shared<SChannel::TransceiverI>(_instance, _delegate->connect(), _host, *clientAuthenticationOptions);
#elif defined(__APPLE__)
    return make_shared<IceSSL::SecureTransport::TransceiverI>(
#else
    return make_shared<IceSSL::OpenSSL::TransceiverI>(
#endif
}

int16_t
IceSSL::ConnectorI::type() const
{
    return _delegate->type();
}

string
IceSSL::ConnectorI::toString() const
{
    return _delegate->toString();
}

bool
IceSSL::ConnectorI::operator==(const IceInternal::Connector& r) const
{
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
    if (!p)
    {
        return false;
    }

    if (this == p)
    {
        return true;
    }

    return Ice::targetEqualTo(_delegate, p->_delegate);
}

bool
IceSSL::ConnectorI::operator<(const IceInternal::Connector& r) const
{
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
    if (!p)
    {
        return type() < r.type();
    }

    if (this == p)
    {
        return false;
    }

    return Ice::targetLess(_delegate, p->_delegate);
}

IceSSL::ConnectorI::ConnectorI(const InstancePtr& instance, const IceInternal::ConnectorPtr& del, const string& h)
    : _instance(instance),
      _delegate(del),
      _host(h)
{
}

IceSSL::ConnectorI::~ConnectorI() {}
