//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "SSLEndpointI.h"
#include "../Ice/DefaultsAndOverrides.h"
#include "../Ice/EndpointFactoryManager.h"
#include "../Ice/HashUtil.h"
#include "Ice/Comparable.h"
#include "Ice/InputStream.h"
#include "Ice/LocalException.h"
#include "Ice/Object.h"
#include "Ice/OutputStream.h"
#include "SSLAcceptorI.h"
#include "SSLConnectorI.h"
#include "SSLInstance.h"

using namespace std;
using namespace Ice;
using namespace IceSSL;

extern "C"
{
    Plugin* createIceSSL(const CommunicatorPtr& communicator, const string&, const StringSeq&)
    {
        IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
        IceSSL::SSLEnginePtr engine = instance->sslEngine();
        IceInternal::EndpointFactoryManagerPtr endpointFactoryManager = instance->endpointFactoryManager();
        IceSSL::InstancePtr sslInstance = make_shared<IceSSL::Instance>(engine, SSLEndpointType, "ssl");

        return new IceInternal::EndpointFactoryPlugin(
            communicator,
            make_shared<IceSSL::EndpointFactoryI>(sslInstance, TCPEndpointType));
    }
}

namespace Ice
{
    ICE_API void registerIceSSL(bool loadOnInitialize)
    {
        Ice::registerPluginFactory("IceSSL", createIceSSL, loadOnInitialize);
    }
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

IceSSL::ConnectionInfo::~ConnectionInfo() {}

IceSSL::EndpointInfo::~EndpointInfo() {}

IceSSL::EndpointI::EndpointI(const InstancePtr& instance, const IceInternal::EndpointIPtr& del)
    : _instance(instance),
      _delegate(del)
{
}

void
IceSSL::EndpointI::streamWriteImpl(Ice::OutputStream* stream) const
{
    _delegate->streamWriteImpl(stream);
}

Ice::EndpointInfoPtr
IceSSL::EndpointI::getInfo() const noexcept
{
    EndpointInfoPtr info =
        make_shared<IceInternal::InfoI<EndpointInfo>>(const_cast<EndpointI*>(this)->shared_from_this());
    info->underlying = _delegate->getInfo();
    info->compress = info->underlying->compress;
    info->timeout = info->underlying->timeout;
    return info;
}

int16_t
IceSSL::EndpointI::type() const
{
    return _delegate->type();
}

const std::string&
IceSSL::EndpointI::protocol() const
{
    return _delegate->protocol();
}

int32_t
IceSSL::EndpointI::timeout() const
{
    return _delegate->timeout();
}

IceInternal::EndpointIPtr
IceSSL::EndpointI::timeout(int32_t timeout) const
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
IceSSL::EndpointI::connectionId() const
{
    return _delegate->connectionId();
}

IceInternal::EndpointIPtr
IceSSL::EndpointI::connectionId(const string& connectionId) const
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
IceSSL::EndpointI::compress() const
{
    return _delegate->compress();
}

IceInternal::EndpointIPtr
IceSSL::EndpointI::compress(bool compress) const
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
IceSSL::EndpointI::datagram() const
{
    return _delegate->datagram();
}

bool
IceSSL::EndpointI::secure() const
{
    return _delegate->secure();
}

IceInternal::TransceiverPtr
IceSSL::EndpointI::transceiver() const
{
    return nullptr;
}

void
IceSSL::EndpointI::connectorsAsync(
    Ice::EndpointSelectionType selType,
    function<void(vector<IceInternal::ConnectorPtr>)> response,
    function<void(exception_ptr)> exception) const
{
    IPEndpointInfoPtr info = getIPEndpointInfo(_delegate->getInfo());
    string host = info ? info->host : string();

    _delegate->connectorsAsync(
        selType,
        [response, this, host](vector<IceInternal::ConnectorPtr> connectors)
        {
            for (vector<IceInternal::ConnectorPtr>::iterator it = connectors.begin(); it != connectors.end(); ++it)
            {
                *it = make_shared<ConnectorI>(_instance, *it, host);
            }
            response(std::move(connectors));
        },
        exception);
}

IceInternal::AcceptorPtr
IceSSL::EndpointI::acceptor(const string& adapterName) const
{
    return make_shared<AcceptorI>(
        const_cast<EndpointI*>(this)->shared_from_this(),
        _instance,
        _delegate->acceptor(adapterName),
        adapterName);
}

EndpointIPtr
IceSSL::EndpointI::endpoint(const IceInternal::EndpointIPtr& delEndp) const
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
IceSSL::EndpointI::expandIfWildcard() const
{
    vector<IceInternal::EndpointIPtr> endps = _delegate->expandIfWildcard();
    for (vector<IceInternal::EndpointIPtr>::iterator p = endps.begin(); p != endps.end(); ++p)
    {
        if (p->get() == _delegate.get())
        {
            *p = const_cast<EndpointI*>(this)->shared_from_this();
        }
        else
        {
            *p = make_shared<EndpointI>(_instance, *p);
        }
    }
    return endps;
}

vector<IceInternal::EndpointIPtr>
IceSSL::EndpointI::expandHost(IceInternal::EndpointIPtr& publish) const
{
    vector<IceInternal::EndpointIPtr> endps = _delegate->expandHost(publish);
    if (publish.get() == _delegate.get())
    {
        publish = const_cast<EndpointI*>(this)->shared_from_this();
    }
    else if (publish.get())
    {
        publish = make_shared<EndpointI>(_instance, publish);
    }
    for (vector<IceInternal::EndpointIPtr>::iterator p = endps.begin(); p != endps.end(); ++p)
    {
        if (p->get() == _delegate.get())
        {
            *p = const_cast<EndpointI*>(this)->shared_from_this();
        }
        else
        {
            *p = make_shared<EndpointI>(_instance, *p);
        }
    }
    return endps;
}

bool
IceSSL::EndpointI::equivalent(const IceInternal::EndpointIPtr& endpoint) const
{
    const EndpointI* endpointI = dynamic_cast<const EndpointI*>(endpoint.get());
    if (!endpointI)
    {
        return false;
    }
    return _delegate->equivalent(endpointI->_delegate);
}

int32_t
IceSSL::EndpointI::hash() const
{
    return _delegate->hash();
}

string
IceSSL::EndpointI::options() const
{
    return _delegate->options();
}

bool
IceSSL::EndpointI::operator==(const Ice::Endpoint& r) const
{
    const EndpointI* p = dynamic_cast<const EndpointI*>(&r);
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
IceSSL::EndpointI::operator<(const Ice::Endpoint& r) const
{
    const EndpointI* p = dynamic_cast<const EndpointI*>(&r);
    if (!p)
    {
        const IceInternal::EndpointI* e = dynamic_cast<const IceInternal::EndpointI*>(&r);
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
IceSSL::EndpointI::checkOption(const string& /*option*/, const string& /*argument*/, const string& /*endpoint*/)
{
    return false;
}

IceSSL::EndpointFactoryI::EndpointFactoryI(const InstancePtr& instance, int16_t type)
    : IceInternal::EndpointFactoryWithUnderlying(instance, type),
      _sslInstance(instance)
{
}

void
IceSSL::EndpointFactoryI::destroy()
{
    _sslInstance = nullptr;
}

IceInternal::EndpointFactoryPtr
IceSSL::EndpointFactoryI::cloneWithUnderlying(const IceInternal::ProtocolInstancePtr& instance, int16_t underlying)
    const
{
    return make_shared<EndpointFactoryI>(
        make_shared<Instance>(_sslInstance->engine(), instance->type(), instance->protocol()),
        underlying);
}

IceInternal::EndpointIPtr
IceSSL::EndpointFactoryI::createWithUnderlying(const IceInternal::EndpointIPtr& underlying, vector<string>&, bool) const
{
    return make_shared<EndpointI>(_sslInstance, underlying);
}

IceInternal::EndpointIPtr
IceSSL::EndpointFactoryI::readWithUnderlying(const IceInternal::EndpointIPtr& underlying, Ice::InputStream*) const
{
    return make_shared<EndpointI>(_sslInstance, underlying);
}
