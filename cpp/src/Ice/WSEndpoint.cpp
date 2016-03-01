// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/WSEndpoint.h>
#include <Ice/WSAcceptor.h>
#include <Ice/WSConnector.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <Ice/IPEndpointI.h>
#include <Ice/HashUtil.h>
#include <Ice/EndpointFactoryManager.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(WSEndpoint* p) { return p; }

IceInternal::WSEndpoint::WSEndpoint(const ProtocolInstancePtr& instance, const EndpointIPtr& del, const string& res) :
    _instance(instance), _delegate(IPEndpointIPtr::dynamicCast(del)), _resource(res)
{
}

IceInternal::WSEndpoint::WSEndpoint(const ProtocolInstancePtr& inst, const EndpointIPtr& del, vector<string>& args) :
    _instance(inst), _delegate(IPEndpointIPtr::dynamicCast(del))
{
    initWithOptions(args);

    if(_resource.empty())
    {
        const_cast<string&>(_resource) = "/";
    }
}

IceInternal::WSEndpoint::WSEndpoint(const ProtocolInstancePtr& instance, const EndpointIPtr& del, BasicStream* s) :
    _instance(instance), _delegate(IPEndpointIPtr::dynamicCast(del))
{
    s->read(const_cast<string&>(_resource), false);
}

Ice::EndpointInfoPtr
IceInternal::WSEndpoint::getInfo() const
{
    assert(dynamic_cast<WSEndpointDelegate*>(_delegate.get()));
    return dynamic_cast<WSEndpointDelegate*>(_delegate.get())->getWSInfo(_resource);
}

Ice::Short
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
IceInternal::WSEndpoint::streamWrite(BasicStream* s) const
{
    s->startWriteEncaps();
    _delegate->streamWriteImpl(s);
    s->write(_resource, false);
    s->endWriteEncaps();
}

Int
IceInternal::WSEndpoint::timeout() const
{
    return _delegate->timeout();
}

EndpointIPtr
IceInternal::WSEndpoint::timeout(Int timeout) const
{
    if(timeout == _delegate->timeout())
    {
        return const_cast<WSEndpoint*>(this);
    }
    else
    {
        return new WSEndpoint(_instance, _delegate->timeout(timeout), _resource);
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
    if(connectionId == _delegate->connectionId())
    {
        return const_cast<WSEndpoint*>(this);
    }
    else
    {
        return new WSEndpoint(_instance, _delegate->connectionId(connectionId), _resource);
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
    if(compress == _delegate->compress())
    {
        return const_cast<WSEndpoint*>(this);
    }
    else
    {
        return new WSEndpoint(_instance, _delegate->compress(compress), _resource);
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
    return 0;
}

void
IceInternal::WSEndpoint::connectors_async(Ice::EndpointSelectionType selType,
                                         const EndpointI_connectorsPtr& callback) const
{
    class CallbackI : public EndpointI_connectors
    {
    public:

        CallbackI(const EndpointI_connectorsPtr& callback, const ProtocolInstancePtr& instance,
                  const string& host, int port, const string& resource) :
            _callback(callback), _instance(instance), _host(host), _port(port), _resource(resource)
        {
        }

        virtual void connectors(const vector<ConnectorPtr>& c)
        {
            vector<ConnectorPtr> connectors = c;
            for(vector<ConnectorPtr>::iterator p = connectors.begin(); p != connectors.end(); ++p)
            {
                *p = new WSConnector(_instance, *p, _host, _port, _resource);
            }
            _callback->connectors(connectors);
        }

        virtual void exception(const Ice::LocalException& ex)
        {
            _callback->exception(ex);
        }

    private:

        const EndpointI_connectorsPtr _callback;
        const ProtocolInstancePtr _instance;
        const string _host;
        const int _port;
        const string _resource;
    };
    _delegate->connectors_async(selType, new CallbackI(callback, _instance, _delegate->host(), _delegate->port(),
                                                       _resource));
}

AcceptorPtr
IceInternal::WSEndpoint::acceptor(const string& adapterName) const
{
    AcceptorPtr delAcc = _delegate->acceptor(adapterName);
    return new WSAcceptor(const_cast<WSEndpoint*>(this), _instance, delAcc);
}

WSEndpointPtr
IceInternal::WSEndpoint::endpoint(const EndpointIPtr& delEndp) const
{
    return new WSEndpoint(_instance, delEndp, _resource);
}

vector<EndpointIPtr>
IceInternal::WSEndpoint::expand() const
{
    vector<EndpointIPtr> endps = _delegate->expand();
    for(vector<EndpointIPtr>::iterator p = endps.begin(); p != endps.end(); ++p)
    {
        *p = p->get() == _delegate.get() ? const_cast<WSEndpoint*>(this) : new WSEndpoint(_instance, *p, _resource);
    }
    return endps;
}

bool
IceInternal::WSEndpoint::equivalent(const EndpointIPtr& endpoint) const
{
    const WSEndpoint* wsEndpointI = dynamic_cast<const WSEndpoint*>(endpoint.get());
    if(!wsEndpointI)
    {
        return false;
    }
    return _delegate->equivalent(wsEndpointI->_delegate);
}

Ice::Int
IceInternal::WSEndpoint::hash() const
{
    int h = _delegate->hash();
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

    if(!_resource.empty())
    {
        s << " -r ";
        bool addQuote = _resource.find(':') != string::npos;
        if(addQuote)
        {
            s << "\"";
        }
        s << _resource;
        if(addQuote)
        {
            s << "\"";
        }
    }

    return s.str();
}

EndpointIPtr
IceInternal::WSEndpoint::delegate() const
{
    return EndpointIPtr::dynamicCast(_delegate);
}

bool
IceInternal::WSEndpoint::operator==(const Ice::LocalObject& r) const
{
    const WSEndpoint* p = dynamic_cast<const WSEndpoint*>(&r);
    if(!p)
    {
        return false;
    }

    if(this == p)
    {
        return true;
    }

    if(_delegate != p->_delegate)
    {
        return false;
    }

    if(_resource != p->_resource)
    {
        return false;
    }

    return true;
}

bool
IceInternal::WSEndpoint::operator<(const Ice::LocalObject& r) const
{
    const WSEndpoint* p = dynamic_cast<const WSEndpoint*>(&r);
    if(!p)
    {
        const EndpointI* e = dynamic_cast<const WSEndpoint*>(&r);
        if(!e)
        {
            return false;
        }
        return type() < e->type();
    }

    if(this == p)
    {
        return false;
    }

    if(_delegate < p->_delegate)
    {
        return true;
    }
    else if (p->_delegate < _delegate)
    {
        return false;
    }

    if(_resource < p->_resource)
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
    switch(option[1])
    {
    case 'r':
    {
        if(argument.empty())
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "no argument provided for -r option in endpoint " + endpoint + _delegate->options();
            throw ex;
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

IceInternal::WSEndpointFactory::WSEndpointFactory(const ProtocolInstancePtr& instance, const EndpointFactoryPtr& del) :
    _instance(instance),
    _delegate(del)
{
}

IceInternal::WSEndpointFactory::~WSEndpointFactory()
{
}

Short
IceInternal::WSEndpointFactory::type() const
{
    return _instance->type();
}

string
IceInternal::WSEndpointFactory::protocol() const
{
    return _instance->protocol();
}

EndpointIPtr
IceInternal::WSEndpointFactory::create(vector<string>& args, bool oaEndpoint) const
{
    return new WSEndpoint(_instance, _delegate->create(args, oaEndpoint), args);
}

EndpointIPtr
IceInternal::WSEndpointFactory::read(BasicStream* s) const
{
    return new WSEndpoint(_instance, _delegate->read(s), s);
}

void
IceInternal::WSEndpointFactory::destroy()
{
    _delegate->destroy();
    _instance = 0;
}

EndpointFactoryPtr
IceInternal::WSEndpointFactory::clone(const ProtocolInstancePtr&) const
{
    assert(false); // We don't support cloning this transport.
    return 0;
}
