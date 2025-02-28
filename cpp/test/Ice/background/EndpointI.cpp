// Copyright (c) ZeroC, Inc.

#ifndef TEST_API_EXPORTS
#    define TEST_API_EXPORTS
#endif

#include "EndpointI.h"
#include "Acceptor.h"
#include "Connector.h"
#include "Transceiver.h"

#include <algorithm>

#ifdef _MSC_VER
// For 'Ice::Object::ice_getHash': was declared deprecated
#    pragma warning(disable : 4996)
#endif

#if defined(__GNUC__)
#    pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

using namespace std;

int16_t EndpointI::TYPE_BASE = 100;

EndpointI::EndpointI(IceInternal::EndpointIPtr endpoint)
    : _endpoint(std::move(endpoint)),
      _configuration(Configuration::getInstance())
{
}

void
EndpointI::streamWriteImpl(Ice::OutputStream* s) const
{
    s->write(_endpoint->type());
    _endpoint->streamWrite(s);
}

int16_t
EndpointI::type() const
{
    return (int16_t)(TYPE_BASE + _endpoint->type());
}

const std::string&
EndpointI::protocol() const
{
    return _endpoint->protocol();
}

int
EndpointI::timeout() const
{
    return _endpoint->timeout();
}

const std::string&
EndpointI::connectionId() const
{
    return _endpoint->connectionId();
}

IceInternal::EndpointIPtr
EndpointI::timeout(int timeout) const
{
    IceInternal::EndpointIPtr endpoint = _endpoint->timeout(timeout);
    if (endpoint == _endpoint)
    {
        return const_cast<EndpointI*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<EndpointI>(endpoint);
    }
}

IceInternal::EndpointIPtr
EndpointI::connectionId(const string& connectionId) const
{
    IceInternal::EndpointIPtr endpoint = _endpoint->connectionId(connectionId);
    if (endpoint == _endpoint)
    {
        return const_cast<EndpointI*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<EndpointI>(endpoint);
    }
}

bool
EndpointI::compress() const
{
    return _endpoint->compress();
}

IceInternal::EndpointIPtr
EndpointI::compress(bool compress) const
{
    IceInternal::EndpointIPtr endpoint = _endpoint->compress(compress);
    if (endpoint == _endpoint)
    {
        return const_cast<EndpointI*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<EndpointI>(endpoint);
    }
}

bool
EndpointI::datagram() const
{
    return _endpoint->datagram();
}

bool
EndpointI::secure() const
{
    return _endpoint->secure();
}

IceInternal::TransceiverPtr
EndpointI::transceiver() const
{
    IceInternal::TransceiverPtr transceiver = _endpoint->transceiver();
    if (transceiver)
    {
        return make_shared<Transceiver>(transceiver);
    }
    else
    {
        return nullptr;
    }
}

void
EndpointI::connectorsAsync(
    function<void(vector<IceInternal::ConnectorPtr>)> response,
    function<void(exception_ptr)> exception) const
{
    try
    {
        _configuration->checkConnectorsException();
        _endpoint->connectorsAsync(
            [response](vector<IceInternal::ConnectorPtr> connectors)
            {
                for (auto& connector : connectors)
                {
                    connector = make_shared<Connector>(connector);
                }
                response(std::move(connectors));
            },
            exception);
    }
    catch (const Ice::LocalException&)
    {
        exception(current_exception());
    }
}

IceInternal::AcceptorPtr
EndpointI::acceptor(
    const string& adapterName,
    const std::optional<Ice::SSL::ServerAuthenticationOptions>& serverAuthenticationOptions) const
{
    return make_shared<Acceptor>(
        const_cast<EndpointI*>(this)->shared_from_this(),
        _endpoint->acceptor(adapterName, serverAuthenticationOptions));
}

EndpointIPtr
EndpointI::endpoint(const IceInternal::EndpointIPtr& delEndp) const
{
    if (delEndp.get() == _endpoint.get())
    {
        return dynamic_pointer_cast<EndpointI>(const_cast<EndpointI*>(this)->shared_from_this());
    }
    else
    {
        return make_shared<EndpointI>(delEndp);
    }
}

vector<IceInternal::EndpointIPtr>
EndpointI::expandHost() const
{
    vector<IceInternal::EndpointIPtr> endpoints = _endpoint->expandHost();

    transform(
        endpoints.begin(),
        endpoints.end(),
        endpoints.begin(),
        [this](const IceInternal::EndpointIPtr& p) { return endpoint(p); });

    return endpoints;
}

bool
EndpointI::isLoopbackOrMulticast() const
{
    return _endpoint->isLoopbackOrMulticast();
}

shared_ptr<IceInternal::EndpointI>
EndpointI::toPublishedEndpoint(string publishedHost) const
{
    return endpoint(_endpoint->toPublishedEndpoint(std::move(publishedHost)));
}

bool
EndpointI::equivalent(const IceInternal::EndpointIPtr& endpoint) const
{
    auto testEndpointI = dynamic_pointer_cast<EndpointI>(endpoint);
    if (!testEndpointI)
    {
        return false;
    }
    return testEndpointI->_endpoint->equivalent(_endpoint);
}

string
EndpointI::toString() const noexcept
{
    return "test-" + _endpoint->toString();
}

Ice::EndpointInfoPtr
EndpointI::getInfo() const noexcept
{
    return _endpoint->getInfo();
}

bool
EndpointI::operator==(const Ice::Endpoint& r) const
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

    return *p->_endpoint == *_endpoint;
}

bool
EndpointI::operator<(const Ice::Endpoint& r) const
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

    return *p->_endpoint < *_endpoint;
}

size_t
EndpointI::hash() const noexcept
{
    return _endpoint->hash();
}

string
EndpointI::options() const
{
    return _endpoint->options();
}

IceInternal::EndpointIPtr
EndpointI::delegate() const
{
    return _endpoint;
}
