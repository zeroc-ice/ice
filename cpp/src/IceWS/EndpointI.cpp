// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceWS/EndpointI.h>
#include <IceWS/AcceptorI.h>
#include <IceWS/ConnectorI.h>
#include <IceWS/TransceiverI.h>
#include <IceWS/Instance.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <Ice/IPEndpointI.h>
#include <Ice/HashUtil.h>

using namespace std;
using namespace Ice;
using namespace IceWS;

IceWS::EndpointI::EndpointI(const InstancePtr& instance, const IceInternal::EndpointIPtr& del, const string& res) :
    _instance(instance), _delegate(IceInternal::IPEndpointIPtr::dynamicCast(del)), _resource(res)
{
}

IceWS::EndpointI::EndpointI(const InstancePtr& instance, const IceInternal::EndpointIPtr& del, vector<string>& args) :
    _instance(instance), _delegate(IceInternal::IPEndpointIPtr::dynamicCast(del))
{
    initWithOptions(args);

    if(_resource.empty())
    {
        const_cast<string&>(_resource) = "/";
    }
}

IceWS::EndpointI::EndpointI(const InstancePtr& instance, const IceInternal::EndpointIPtr& del, 
                            IceInternal::BasicStream* s) :
    _instance(instance), _delegate(IceInternal::IPEndpointIPtr::dynamicCast(del))
{
    s->read(const_cast<string&>(_resource), false);
}

Ice::EndpointInfoPtr
IceWS::EndpointI::getInfo() const
{
    class InfoI : public IceWS::EndpointInfo
    {
    public:

        InfoI(const IceInternal::EndpointIPtr& e) : _endpoint(e)
        {
        }

        virtual Short
        type() const
        {
            return _endpoint->type();
        }

        virtual bool
        datagram() const
        {
            return _endpoint->datagram();
        }

        virtual bool
        secure() const
        {
            return _endpoint->secure();
        }
        
    private:
        
        const IceInternal::EndpointIPtr _endpoint;
    };

    EndpointInfoPtr info = new InfoI(const_cast<EndpointI*>(this));
    info->timeout = _delegate->timeout();
    info->compress = _delegate->compress();
    _delegate->fillEndpointInfo(info.get());
    info->resource = _resource;
    return info;
}

Ice::Short
IceWS::EndpointI::type() const
{
    return _delegate->type();
}

const string&
IceWS::EndpointI::protocol() const
{
    return _delegate->protocol();
}

void
IceWS::EndpointI::streamWrite(IceInternal::BasicStream* s) const
{
    s->startWriteEncaps();
    _delegate->streamWriteImpl(s);
    s->write(_resource, false);
    s->endWriteEncaps();
}

Int
IceWS::EndpointI::timeout() const
{
    return _delegate->timeout();
}

IceInternal::EndpointIPtr
IceWS::EndpointI::timeout(Int timeout) const
{
    if(timeout == _delegate->timeout())
    {
        return const_cast<EndpointI*>(this);
    }
    else
    {
        return new EndpointI(_instance, _delegate->timeout(timeout), _resource);
    }
}

const string&
IceWS::EndpointI::connectionId() const
{
    return _delegate->connectionId();
}

IceInternal::EndpointIPtr
IceWS::EndpointI::connectionId(const string& connectionId) const
{
    if(connectionId == _delegate->connectionId())
    {
        return const_cast<EndpointI*>(this);
    }
    else
    {
        return new EndpointI(_instance, _delegate->connectionId(connectionId), _resource);
    }
}

bool
IceWS::EndpointI::compress() const
{
    return _delegate->compress();
}

IceInternal::EndpointIPtr
IceWS::EndpointI::compress(bool compress) const
{
    if(compress == _delegate->compress())
    {
        return const_cast<EndpointI*>(this);
    }
    else
    {
        return new EndpointI(_instance, _delegate->compress(compress), _resource);
    }
}

bool
IceWS::EndpointI::datagram() const
{
    return _delegate->datagram();
}

bool
IceWS::EndpointI::secure() const
{
    return _delegate->secure();
}

IceInternal::TransceiverPtr
IceWS::EndpointI::transceiver(IceInternal::EndpointIPtr& endp) const
{
    endp = const_cast<EndpointI*>(this);
    return 0;
}

vector<IceInternal::ConnectorPtr>
IceWS::EndpointI::connectors(Ice::EndpointSelectionType selType) const
{
    vector<IceInternal::ConnectorPtr> connectors = _delegate->connectors(selType);
    for(vector<IceInternal::ConnectorPtr>::iterator p = connectors.begin(); p != connectors.end(); ++p)
    {
        *p = new ConnectorI(_instance, *p, _delegate->host(), _delegate->port(), _resource);
    }
    return connectors;
}

void
IceWS::EndpointI::connectors_async(Ice::EndpointSelectionType selType,
                                   const IceInternal::EndpointI_connectorsPtr& callback) const
{
    class CallbackI : public IceInternal::EndpointI_connectors
    {
    public:

        CallbackI(const IceInternal::EndpointI_connectorsPtr& callback, const InstancePtr& instance, 
                  const string& host, int port, const string& resource) : 
            _callback(callback), _instance(instance), _host(host), _port(port), _resource(resource)
        {
        }

        virtual void connectors(const vector<IceInternal::ConnectorPtr>& c)
        {
            vector<IceInternal::ConnectorPtr> connectors = c;
            for(vector<IceInternal::ConnectorPtr>::iterator p = connectors.begin(); p != connectors.end(); ++p)
            {
                *p = new ConnectorI(_instance, *p, _host, _port, _resource);
            }
            _callback->connectors(connectors);
        }

        virtual void exception(const Ice::LocalException& ex)
        {
            _callback->exception(ex);
        }

    private:
        
        const IceInternal::EndpointI_connectorsPtr _callback;
        const InstancePtr _instance;
        const string _host;
        const int _port;
        const string _resource;
    };
    _delegate->connectors_async(selType, new CallbackI(callback, _instance, _delegate->host(), _delegate->port(), 
                                                       _resource));
}

IceInternal::AcceptorPtr
IceWS::EndpointI::acceptor(IceInternal::EndpointIPtr& endp, const string& adapterName) const
{
    IceInternal::EndpointIPtr delEndp;
    IceInternal::AcceptorPtr delAcc = _delegate->acceptor(delEndp, adapterName);
    if(delEndp)
    {
        endp = new EndpointI(_instance, delEndp, _resource);
    }
    return new AcceptorI(_instance, delAcc);
}

vector<IceInternal::EndpointIPtr>
IceWS::EndpointI::expand() const
{
    vector<IceInternal::EndpointIPtr> endps = _delegate->expand();
    for(vector<IceInternal::EndpointIPtr>::iterator p = endps.begin(); p != endps.end(); ++p)
    {
        *p = p->get() == _delegate.get() ? const_cast<EndpointI*>(this) : new EndpointI(_instance, *p, _resource);
    }
    return endps;
}

bool
IceWS::EndpointI::equivalent(const IceInternal::EndpointIPtr& endpoint) const
{
    const EndpointI* wsEndpointI = dynamic_cast<const EndpointI*>(endpoint.get());
    if(!wsEndpointI)
    {
        return false;
    }
    return _delegate->equivalent(wsEndpointI->_delegate);
}

Ice::Int
IceWS::EndpointI::hash() const
{
    int h = _delegate->hash();
    IceInternal::hashAdd(h, _resource);
    return h;
}

string
IceWS::EndpointI::options() const
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

bool
IceWS::EndpointI::operator==(const Ice::LocalObject& r) const
{
    const EndpointI* p = dynamic_cast<const EndpointI*>(&r);
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
IceWS::EndpointI::operator<(const Ice::LocalObject& r) const
{
    const EndpointI* p = dynamic_cast<const EndpointI*>(&r);
    if(!p)
    {
        const IceInternal::EndpointI* e = dynamic_cast<const IceInternal::EndpointI*>(&r);
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
IceWS::EndpointI::checkOption(const string& option, const string& argument, const string& endpoint)
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

IceWS::EndpointFactoryI::EndpointFactoryI(const InstancePtr& instance, const IceInternal::EndpointFactoryPtr& del) :
    _instance(instance),
    _delegate(del)
{
}

IceWS::EndpointFactoryI::~EndpointFactoryI()
{
}

Short
IceWS::EndpointFactoryI::type() const
{
    return _instance->type();
}

string
IceWS::EndpointFactoryI::protocol() const
{
    return _instance->protocol();
}

IceInternal::EndpointIPtr
IceWS::EndpointFactoryI::create(vector<string>& args, bool oaEndpoint) const
{
    return new EndpointI(_instance, _delegate->create(args, oaEndpoint), args);
}

IceInternal::EndpointIPtr
IceWS::EndpointFactoryI::read(IceInternal::BasicStream* s) const
{
    return new EndpointI(_instance, _delegate->read(s), s);
}

void
IceWS::EndpointFactoryI::destroy()
{
    _delegate->destroy();
    _instance = 0;
}

IceInternal::EndpointFactoryPtr
IceWS::EndpointFactoryI::clone(const IceInternal::ProtocolInstancePtr&) const
{
    assert(false); // We don't support cloning this transport.
    return 0;
}
