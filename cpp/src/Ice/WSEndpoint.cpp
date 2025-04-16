// Copyright (c) ZeroC, Inc.

#include "WSEndpoint.h"
#include "EndpointFactoryManager.h"
#include "HashUtil.h"
#include "IPEndpointI.h"
#include "Ice/InputStream.h"
#include "Ice/LocalExceptions.h"
#include "Ice/OutputStream.h"
#include "TargetCompare.h"
#include "WSAcceptor.h"
#include "WSConnector.h"

#include <algorithm>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    const char* const wsPluginName = "IceWS";

    class WSEndpointFactoryPlugin : public Plugin
    {
    public:
        WSEndpointFactoryPlugin(const CommunicatorPtr&);
        void initialize() override;
        void destroy() override;
    };

    IPEndpointInfoPtr getIPEndpointInfo(const EndpointInfoPtr& info)
    {
        for (EndpointInfoPtr p = info; p; p = p->underlying)
        {
            IPEndpointInfoPtr ipInfo = dynamic_pointer_cast<IPEndpointInfo>(p);
            if (ipInfo)
            {
                return ipInfo;
            }
        }
        return nullptr;
    }

    Plugin* createIceWS(const CommunicatorPtr& c, const string& name, const StringSeq&)
    {
        string pluginName{wsPluginName};

        if (name != pluginName)
        {
            throw PluginInitializationException{
                __FILE__,
                __LINE__,
                "the WebSocket plug-in must be named '" + pluginName + "'"};
        }

        return new WSEndpointFactoryPlugin(c);
    }
}

Ice::PluginFactory
Ice::wsPluginFactory()
{
    return {wsPluginName, createIceWS};
}

WSEndpointFactoryPlugin::WSEndpointFactoryPlugin(const CommunicatorPtr& communicator)
{
    assert(communicator);

    const EndpointFactoryManagerPtr efm = getInstance(communicator)->endpointFactoryManager();
    efm->add(make_shared<WSEndpointFactory>(
        make_shared<ProtocolInstance>(communicator, WSEndpointType, "ws", false),
        TCPEndpointType));
    efm->add(make_shared<WSEndpointFactory>(
        make_shared<ProtocolInstance>(communicator, WSSEndpointType, "wss", true),
        SSLEndpointType));
}

void
WSEndpointFactoryPlugin::initialize()
{
}

void
WSEndpointFactoryPlugin::destroy()
{
}

IceInternal::WSEndpoint::WSEndpoint(ProtocolInstancePtr instance, EndpointIPtr del, string res)
    : _instance(std::move(instance)),
      _delegate(std::move(del)),
      _resource(std::move(res))
{
}

IceInternal::WSEndpoint::WSEndpoint(ProtocolInstancePtr inst, EndpointIPtr del, vector<string>& args)
    : _instance(std::move(inst)),
      _delegate(std::move(del))
{
    initWithOptions(args);

    if (_resource.empty())
    {
        const_cast<string&>(_resource) = "/";
    }
}

IceInternal::WSEndpoint::WSEndpoint(ProtocolInstancePtr instance, EndpointIPtr del, InputStream* s)
    : _instance(std::move(instance)),
      _delegate(std::move(del))
{
    s->read(const_cast<string&>(_resource), false);
}

EndpointInfoPtr
IceInternal::WSEndpoint::getInfo() const noexcept
{
    return make_shared<WSEndpointInfo>(_delegate->getInfo(), _resource);
}

int16_t
IceInternal::WSEndpoint::type() const
{
    return _delegate->type();
}

const string&
IceInternal::WSEndpoint::protocol() const
{
    return _delegate->protocol();
}

void
IceInternal::WSEndpoint::streamWriteImpl(OutputStream* s) const
{
    _delegate->streamWriteImpl(s);
    s->write(_resource, false);
}

int32_t
IceInternal::WSEndpoint::timeout() const
{
    return _delegate->timeout();
}

EndpointIPtr
IceInternal::WSEndpoint::timeout(int32_t timeout) const
{
    if (timeout == _delegate->timeout())
    {
        return const_cast<WSEndpoint*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<WSEndpoint>(_instance, _delegate->timeout(timeout), _resource);
    }
}

const string&
IceInternal::WSEndpoint::connectionId() const
{
    return _delegate->connectionId();
}

EndpointIPtr
IceInternal::WSEndpoint::connectionId(const string& connectionId) const
{
    if (connectionId == _delegate->connectionId())
    {
        return const_cast<WSEndpoint*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<WSEndpoint>(_instance, _delegate->connectionId(connectionId), _resource);
    }
}

bool
IceInternal::WSEndpoint::compress() const
{
    return _delegate->compress();
}

EndpointIPtr
IceInternal::WSEndpoint::compress(bool compress) const
{
    if (compress == _delegate->compress())
    {
        return const_cast<WSEndpoint*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<WSEndpoint>(_instance, _delegate->compress(compress), _resource);
    }
}

bool
IceInternal::WSEndpoint::datagram() const
{
    return _delegate->datagram();
}

bool
IceInternal::WSEndpoint::secure() const
{
    return _delegate->secure();
}

TransceiverPtr
IceInternal::WSEndpoint::transceiver() const
{
    return nullptr;
}

void
IceInternal::WSEndpoint::connectorsAsync(
    function<void(vector<IceInternal::ConnectorPtr>)> response,
    function<void(exception_ptr)> exception) const
{
    string host;
    IPEndpointInfoPtr info = getIPEndpointInfo(_delegate->getInfo());
    if (info)
    {
        ostringstream os;
        // Use to_string for locale independent formatting.
        os << info->host << ":" << to_string(info->port);
        host = os.str();
    }

    auto self = const_cast<WSEndpoint*>(this)->shared_from_this();
    _delegate->connectorsAsync(
        [response, host, self](vector<ConnectorPtr> connectors)
        {
            for (auto& connector : connectors)
            {
                connector = make_shared<WSConnector>(self->_instance, connector, host, self->_resource);
            }
            response(std::move(connectors));
        },
        exception);
}

AcceptorPtr
IceInternal::WSEndpoint::acceptor(
    const string& adapterName,
    const optional<Ice::SSL::ServerAuthenticationOptions>& serverAuthenticationOptions) const
{
    AcceptorPtr acceptor = _delegate->acceptor(adapterName, serverAuthenticationOptions);
    return make_shared<WSAcceptor>(const_cast<WSEndpoint*>(this)->shared_from_this(), _instance, acceptor);
}

WSEndpointPtr
IceInternal::WSEndpoint::endpoint(const EndpointIPtr& delEndp) const
{
    if (delEndp.get() == _delegate.get())
    {
        return dynamic_pointer_cast<WSEndpoint>(const_cast<WSEndpoint*>(this)->shared_from_this());
    }
    else
    {
        return make_shared<WSEndpoint>(_instance, delEndp, _resource);
    }
}

vector<EndpointIPtr>
IceInternal::WSEndpoint::expandHost() const
{
    vector<EndpointIPtr> endpoints = _delegate->expandHost();

    transform(
        endpoints.begin(),
        endpoints.end(),
        endpoints.begin(),
        [this](const EndpointIPtr& p) { return endpoint(p); });

    return endpoints;
}

bool
IceInternal::WSEndpoint::isLoopbackOrMulticast() const
{
    return _delegate->isLoopbackOrMulticast();
}

shared_ptr<EndpointI>
IceInternal::WSEndpoint::toPublishedEndpoint(string publishedHost) const
{
    return endpoint(_delegate->toPublishedEndpoint(std::move(publishedHost)));
}

bool
IceInternal::WSEndpoint::equivalent(const EndpointIPtr& endpoint) const
{
    auto wsEndpointI = dynamic_pointer_cast<WSEndpoint>(endpoint);
    if (!wsEndpointI)
    {
        return false;
    }
    return _delegate->equivalent(wsEndpointI->_delegate);
}

size_t
IceInternal::WSEndpoint::hash() const noexcept
{
    size_t h = _delegate->hash();
    hashAdd(h, _resource);
    return h;
}

string
IceInternal::WSEndpoint::options() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    ostringstream s;
    s << _delegate->options();

    if (!_resource.empty())
    {
        s << " -r ";
        bool addQuote = _resource.find(':') != string::npos;
        if (addQuote)
        {
            s << "\"";
        }
        s << _resource;
        if (addQuote)
        {
            s << "\"";
        }
    }

    return s.str();
}

bool
IceInternal::WSEndpoint::operator==(const Endpoint& r) const
{
    const auto* p = dynamic_cast<const WSEndpoint*>(&r);
    if (!p)
    {
        return false;
    }

    if (this == p)
    {
        return true;
    }

    if (!targetEqualTo(_delegate, p->_delegate))
    {
        return false;
    }

    if (_resource != p->_resource)
    {
        return false;
    }

    return true;
}

bool
IceInternal::WSEndpoint::operator<(const Endpoint& r) const
{
    const auto* p = dynamic_cast<const WSEndpoint*>(&r);
    if (!p)
    {
        const auto* e = dynamic_cast<const EndpointI*>(&r);
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

    if (targetLess(_delegate, p->_delegate))
    {
        return true;
    }
    else if (targetLess(p->_delegate, _delegate))
    {
        return false;
    }

    if (_resource < p->_resource)
    {
        return true;
    }
    else if (p->_resource < _resource)
    {
        return false;
    }

    return false;
}

bool
IceInternal::WSEndpoint::checkOption(const string& option, const string& argument, const string& endpoint)
{
    switch (option[1])
    {
        case 'r':
        {
            if (argument.empty())
            {
                throw ParseException(
                    __FILE__,
                    __LINE__,
                    "no argument provided for -r option in endpoint '" + endpoint + _delegate->options() + "'");
            }
            const_cast<string&>(_resource) = argument;
            return true;
        }

        default:
        {
            return false;
        }
    }
}

IceInternal::WSEndpointFactory::WSEndpointFactory(const ProtocolInstancePtr& instance, int16_t type)
    : EndpointFactoryWithUnderlying(instance, type)
{
}

EndpointFactoryPtr
IceInternal::WSEndpointFactory::cloneWithUnderlying(const ProtocolInstancePtr& instance, int16_t underlying) const
{
    return make_shared<WSEndpointFactory>(instance, underlying);
}

EndpointIPtr
IceInternal::WSEndpointFactory::createWithUnderlying(const EndpointIPtr& underlying, vector<string>& args, bool) const
{
    return make_shared<WSEndpoint>(_instance, underlying, args);
}

EndpointIPtr
IceInternal::WSEndpointFactory::readWithUnderlying(const EndpointIPtr& underlying, InputStream* s) const
{
    return make_shared<WSEndpoint>(_instance, underlying, s);
}
