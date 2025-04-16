// Copyright (c) ZeroC, Inc.

#include "SSLEndpointI.h"
#include "../TargetCompare.h"
#include "SSLAcceptorI.h"
#include "SSLConnectorI.h"
#include "SSLInstance.h"

#include <algorithm>

using namespace std;
using namespace Ice;
using namespace Ice::SSL;

namespace
{
    const char* const sslPluginName = "IceSSL";

    Plugin* createIceSSL(const CommunicatorPtr& communicator, const string& name, const StringSeq&)
    {
        string pluginName{sslPluginName};

        if (name != pluginName)
        {
            throw PluginInitializationException{
                __FILE__,
                __LINE__,
                "the SSL plug-in must be named '" + pluginName + "'"};
        }

        IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
        SSLEnginePtr engine = instance->sslEngine();
        IceInternal::EndpointFactoryManagerPtr endpointFactoryManager = instance->endpointFactoryManager();
        Ice::SSL::InstancePtr sslInstance = make_shared<Ice::SSL::Instance>(engine, SSLEndpointType, "ssl");

        return new IceInternal::EndpointFactoryPlugin(
            communicator,
            make_shared<Ice::SSL::EndpointFactoryI>(sslInstance, TCPEndpointType));
    }
}

Ice::PluginFactory
Ice::sslPluginFactory()
{
    return {sslPluginName, createIceSSL};
}

namespace
{
    Ice::IPEndpointInfoPtr getIPEndpointInfo(const Ice::EndpointInfoPtr& info)
    {
        for (Ice::EndpointInfoPtr p = info; p; p = p->underlying)
        {
            Ice::IPEndpointInfoPtr ipInfo = dynamic_pointer_cast<Ice::IPEndpointInfo>(p);
            if (ipInfo)
            {
                return ipInfo;
            }
        }
        return nullptr;
    }
}
// Implement virtual destructors out of line to avoid weak vtables.
#if defined(ICE_USE_SCHANNEL)
Ice::SSL::SchannelConnectionInfo::~SchannelConnectionInfo()
{
    if (peerCertificate)
    {
        CertFreeCertificateContext(peerCertificate);
    }
}
#elif defined(ICE_USE_SECURE_TRANSPORT)
Ice::SSL::SecureTransportConnectionInfo::~SecureTransportConnectionInfo()
{
    if (peerCertificate)
    {
        CFRelease(peerCertificate);
    }
}
#else
Ice::SSL::OpenSSLConnectionInfo::~OpenSSLConnectionInfo()
{
    if (peerCertificate)
    {
        X509_free(peerCertificate);
    }
}
#endif

Ice::SSL::EndpointInfo::~EndpointInfo() = default; // Out of line to avoid weak vtable

Ice::SSL::EndpointI::EndpointI(InstancePtr instance, IceInternal::EndpointIPtr del)
    : _instance(std::move(instance)),
      _delegate(std::move(del))
{
}

void
Ice::SSL::EndpointI::streamWriteImpl(Ice::OutputStream* stream) const
{
    _delegate->streamWriteImpl(stream);
}

Ice::EndpointInfoPtr
Ice::SSL::EndpointI::getInfo() const noexcept
{
    return make_shared<Ice::SSL::EndpointInfo>(_delegate->getInfo());
}

int16_t
Ice::SSL::EndpointI::type() const
{
    return _delegate->type();
}

const std::string&
Ice::SSL::EndpointI::protocol() const
{
    return _delegate->protocol();
}

int32_t
Ice::SSL::EndpointI::timeout() const
{
    return _delegate->timeout();
}

IceInternal::EndpointIPtr
Ice::SSL::EndpointI::timeout(int32_t timeout) const
{
    if (timeout == _delegate->timeout())
    {
        return const_cast<EndpointI*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<EndpointI>(_instance, _delegate->timeout(timeout));
    }
}

const string&
Ice::SSL::EndpointI::connectionId() const
{
    return _delegate->connectionId();
}

IceInternal::EndpointIPtr
Ice::SSL::EndpointI::connectionId(const string& connectionId) const
{
    if (connectionId == _delegate->connectionId())
    {
        return const_cast<EndpointI*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<EndpointI>(_instance, _delegate->connectionId(connectionId));
    }
}

bool
Ice::SSL::EndpointI::compress() const
{
    return _delegate->compress();
}

IceInternal::EndpointIPtr
Ice::SSL::EndpointI::compress(bool compress) const
{
    if (compress == _delegate->compress())
    {
        return const_cast<EndpointI*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<EndpointI>(_instance, _delegate->compress(compress));
    }
}

bool
Ice::SSL::EndpointI::datagram() const
{
    return _delegate->datagram();
}

bool
Ice::SSL::EndpointI::secure() const
{
    return _delegate->secure();
}

IceInternal::TransceiverPtr
Ice::SSL::EndpointI::transceiver() const
{
    return nullptr;
}

void
Ice::SSL::EndpointI::connectorsAsync(
    function<void(vector<IceInternal::ConnectorPtr>)> response,
    function<void(exception_ptr)> exception) const
{
    IPEndpointInfoPtr info = getIPEndpointInfo(_delegate->getInfo());
    string host = info ? info->host : string();

    _delegate->connectorsAsync(
        [response, this, host](vector<IceInternal::ConnectorPtr> connectors)
        {
            for (auto& connector : connectors)
            {
                connector = make_shared<ConnectorI>(_instance, connector, host);
            }
            response(std::move(connectors));
        },
        exception);
}

IceInternal::AcceptorPtr
Ice::SSL::EndpointI::acceptor(
    const string& adapterName,
    const optional<Ice::SSL::ServerAuthenticationOptions>& serverAuthenticationOptions) const
{
    return make_shared<AcceptorI>(
        const_cast<EndpointI*>(this)->shared_from_this(),
        _instance,
        _delegate->acceptor(adapterName, serverAuthenticationOptions),
        adapterName,
        serverAuthenticationOptions);
}

EndpointIPtr
Ice::SSL::EndpointI::endpoint(const IceInternal::EndpointIPtr& delEndp) const
{
    if (delEndp.get() == _delegate.get())
    {
        return dynamic_pointer_cast<EndpointI>(const_cast<EndpointI*>(this)->shared_from_this());
    }
    else
    {
        return make_shared<EndpointI>(_instance, delEndp);
    }
}

vector<IceInternal::EndpointIPtr>
Ice::SSL::EndpointI::expandHost() const
{
    vector<IceInternal::EndpointIPtr> endpoints = _delegate->expandHost();

    transform(
        endpoints.begin(),
        endpoints.end(),
        endpoints.begin(),
        [this](const IceInternal::EndpointIPtr& p) { return endpoint(p); });

    return endpoints;
}

bool
Ice::SSL::EndpointI::isLoopbackOrMulticast() const
{
    return _delegate->isLoopbackOrMulticast();
}

shared_ptr<IceInternal::EndpointI>
Ice::SSL::EndpointI::toPublishedEndpoint(string publishedHost) const
{
    return endpoint(_delegate->toPublishedEndpoint(std::move(publishedHost)));
}

bool
Ice::SSL::EndpointI::equivalent(const IceInternal::EndpointIPtr& endpoint) const
{
    auto endpointI = dynamic_pointer_cast<EndpointI>(endpoint);
    if (!endpointI)
    {
        return false;
    }
    return _delegate->equivalent(endpointI->_delegate);
}

size_t
Ice::SSL::EndpointI::hash() const noexcept
{
    return _delegate->hash();
}

string
Ice::SSL::EndpointI::options() const
{
    return _delegate->options();
}

bool
Ice::SSL::EndpointI::operator==(const Ice::Endpoint& r) const
{
    const auto* p = dynamic_cast<const EndpointI*>(&r);
    if (!p)
    {
        return false;
    }

    if (this == p)
    {
        return true;
    }

    if (!Ice::targetEqualTo(_delegate, p->_delegate))
    {
        return false;
    }

    return true;
}

bool
Ice::SSL::EndpointI::operator<(const Ice::Endpoint& r) const
{
    const auto* p = dynamic_cast<const EndpointI*>(&r);
    if (!p)
    {
        const auto* e = dynamic_cast<const IceInternal::EndpointI*>(&r);
        if (!e)
        {
            return false;
        }
        return type() < e->type();
    }

    if (this == p)
    {
        return false;
    }

    if (Ice::targetLess(_delegate, p->_delegate))
    {
        return true;
    }
    else if (Ice::targetLess(p->_delegate, _delegate))
    {
        return false;
    }

    return false;
}

bool
Ice::SSL::EndpointI::checkOption(const string& /*option*/, const string& /*argument*/, const string& /*endpoint*/)
{
    return false;
}

Ice::SSL::EndpointFactoryI::EndpointFactoryI(const InstancePtr& instance, int16_t type)
    : IceInternal::EndpointFactoryWithUnderlying(instance, type),
      _sslInstance(instance)
{
}

IceInternal::EndpointFactoryPtr
Ice::SSL::EndpointFactoryI::cloneWithUnderlying(const IceInternal::ProtocolInstancePtr& instance, int16_t underlying)
    const
{
    return make_shared<EndpointFactoryI>(
        make_shared<Instance>(_sslInstance->engine(), instance->type(), instance->protocol()),
        underlying);
}

IceInternal::EndpointIPtr
Ice::SSL::EndpointFactoryI::createWithUnderlying(const IceInternal::EndpointIPtr& underlying, vector<string>&, bool)
    const
{
    return make_shared<EndpointI>(_sslInstance, underlying);
}

IceInternal::EndpointIPtr
Ice::SSL::EndpointFactoryI::readWithUnderlying(const IceInternal::EndpointIPtr& underlying, Ice::InputStream*) const
{
    return make_shared<EndpointI>(_sslInstance, underlying);
}
