// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/WSEndpoint.h>
#include <Ice/WSAcceptor.h>
#include <Ice/WSConnector.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>
#include <Ice/LocalException.h>
#include <Ice/IPEndpointI.h>
#include <Ice/HashUtil.h>
#include <Ice/EndpointFactoryManager.h>
#include <Ice/Comparable.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

class WSEndpointFactoryPlugin : public Plugin
{
public:

    WSEndpointFactoryPlugin(const CommunicatorPtr&);
    virtual void initialize();
    virtual void destroy();
};

IPEndpointInfoPtr
getIPEndpointInfo(const EndpointInfoPtr& info)
{
    for(EndpointInfoPtr p = info; p; p = p->underlying)
    {
        IPEndpointInfoPtr ipInfo = ICE_DYNAMIC_CAST(IPEndpointInfo, p);
        if(ipInfo)
        {
            return ipInfo;
        }
    }
    return ICE_NULLPTR;
}

}

extern "C"
{

Plugin*
createIceWS(const CommunicatorPtr& c, const string&, const StringSeq&)
{
    return new WSEndpointFactoryPlugin(c);
}

}

namespace Ice
{

ICE_API void
registerIceWS(bool loadOnInitialize)
{
    registerPluginFactory("IceWS", createIceWS, loadOnInitialize);
}

}

//
// Objective-C function to allow Objective-C programs to register plugin.
//
extern "C" ICE_API void
ICEregisterIceWS(bool loadOnInitialize)
{
    Ice::registerIceWS(loadOnInitialize);
}

#ifndef ICE_CPP11_MAPPING
IceUtil::Shared* IceInternal::upCast(WSEndpoint* p) { return p; }
#endif

WSEndpointFactoryPlugin::WSEndpointFactoryPlugin(const CommunicatorPtr& communicator)
{
    assert(communicator);

    const EndpointFactoryManagerPtr efm = getInstance(communicator)->endpointFactoryManager();
    efm->add(new WSEndpointFactory(new ProtocolInstance(communicator, WSEndpointType, "ws", false), TCPEndpointType));
    efm->add(new WSEndpointFactory(new ProtocolInstance(communicator, WSSEndpointType, "wss", true), SSLEndpointType));
}

void
WSEndpointFactoryPlugin::initialize()
{
}

void
WSEndpointFactoryPlugin::destroy()
{
}

IceInternal::WSEndpoint::WSEndpoint(const ProtocolInstancePtr& instance, const EndpointIPtr& del, const string& res) :
    _instance(instance), _delegate(del), _resource(res)
{
}

IceInternal::WSEndpoint::WSEndpoint(const ProtocolInstancePtr& inst, const EndpointIPtr& del, vector<string>& args) :
    _instance(inst), _delegate(del)
{
    initWithOptions(args);

    if(_resource.empty())
    {
        const_cast<string&>(_resource) = "/";
    }
}

IceInternal::WSEndpoint::WSEndpoint(const ProtocolInstancePtr& instance, const EndpointIPtr& del, InputStream* s) :
    _instance(instance), _delegate(del)
{
    s->read(const_cast<string&>(_resource), false);
}

EndpointInfoPtr
IceInternal::WSEndpoint::getInfo() const
{
    WSEndpointInfoPtr info = ICE_MAKE_SHARED(InfoI<WSEndpointInfo>, ICE_SHARED_FROM_CONST_THIS(WSEndpoint));
    info->underlying = _delegate->getInfo();
    info->compress = info->underlying->compress;
    info->timeout = info->underlying->timeout;
    info->resource = _resource;
    return info;
}

Short
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
        return ICE_SHARED_FROM_CONST_THIS(WSEndpoint);
    }
    else
    {
        return ICE_MAKE_SHARED(WSEndpoint, _instance, _delegate->timeout(timeout), _resource);
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
        return ICE_SHARED_FROM_CONST_THIS(WSEndpoint);
    }
    else
    {
        return ICE_MAKE_SHARED(WSEndpoint, _instance, _delegate->connectionId(connectionId), _resource);
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
        return ICE_SHARED_FROM_CONST_THIS(WSEndpoint);
    }
    else
    {
        return ICE_MAKE_SHARED(WSEndpoint, _instance, _delegate->compress(compress), _resource);
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
IceInternal::WSEndpoint::connectors_async(EndpointSelectionType selType,
                                         const EndpointI_connectorsPtr& callback) const
{
    class CallbackI : public EndpointI_connectors
    {
    public:

        CallbackI(const EndpointI_connectorsPtr& callback, const ProtocolInstancePtr& instance,
                  const string& host, const string& resource) :
            _callback(callback), _instance(instance), _host(host), _resource(resource)
        {
        }

        virtual void connectors(const vector<ConnectorPtr>& c)
        {
            vector<ConnectorPtr> connectors = c;
            for(vector<ConnectorPtr>::iterator p = connectors.begin(); p != connectors.end(); ++p)
            {
                *p = new WSConnector(_instance, *p, _host, _resource);
            }
            _callback->connectors(connectors);
        }

        virtual void exception(const LocalException& ex)
        {
            _callback->exception(ex);
        }

    private:

        const EndpointI_connectorsPtr _callback;
        const ProtocolInstancePtr _instance;
        const string _host;
        const string _resource;
    };

    ostringstream host;
    IPEndpointInfoPtr info = getIPEndpointInfo(_delegate->getInfo());
    if(info)
    {
        host << info->host << ":" << info->port;
    }
    _delegate->connectors_async(selType, ICE_MAKE_SHARED(CallbackI, callback, _instance, host.str(), _resource));
}

AcceptorPtr
IceInternal::WSEndpoint::acceptor(const string& adapterName) const
{
    AcceptorPtr delAcc = _delegate->acceptor(adapterName);
    return new WSAcceptor(ICE_SHARED_FROM_CONST_THIS(WSEndpoint), _instance, delAcc);
}

WSEndpointPtr
IceInternal::WSEndpoint::endpoint(const EndpointIPtr& delEndp) const
{
    if(delEndp.get() == _delegate.get())
    {
        return ICE_DYNAMIC_CAST(WSEndpoint, ICE_SHARED_FROM_CONST_THIS(WSEndpoint));
    }
    else
    {
        return ICE_MAKE_SHARED(WSEndpoint, _instance, delEndp, _resource);
    }
}

vector<EndpointIPtr>
IceInternal::WSEndpoint::expandIfWildcard() const
{
    vector<EndpointIPtr> endps = _delegate->expandIfWildcard();
    for(vector<EndpointIPtr>::iterator p = endps.begin(); p != endps.end(); ++p)
    {
        if(p->get() == _delegate.get())
        {
            *p = ICE_SHARED_FROM_CONST_THIS(WSEndpoint);
        }
        else
        {
            *p = ICE_MAKE_SHARED(WSEndpoint, _instance, *p, _resource);
        }
    }
    return endps;
}

vector<EndpointIPtr>
IceInternal::WSEndpoint::expandHost(EndpointIPtr& publish) const
{
    vector<EndpointIPtr> endps = _delegate->expandHost(publish);
    if(publish.get() == _delegate.get())
    {
        publish = ICE_SHARED_FROM_CONST_THIS(WSEndpoint);
    }
    else if(publish.get())
    {
        publish = ICE_MAKE_SHARED(WSEndpoint, _instance, publish, _resource);
    }
    for(vector<EndpointIPtr>::iterator p = endps.begin(); p != endps.end(); ++p)
    {
        if(p->get() == _delegate.get())
        {
            *p = ICE_SHARED_FROM_CONST_THIS(WSEndpoint);
        }
        else
        {
            *p = ICE_MAKE_SHARED(WSEndpoint, _instance, *p, _resource);
        }
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

Int
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

bool
#ifdef ICE_CPP11_MAPPING
IceInternal::WSEndpoint::operator==(const Endpoint& r) const
#else
IceInternal::WSEndpoint::operator==(const LocalObject& r) const
#endif
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

    if(!targetEqualTo(_delegate, p->_delegate))
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
#ifdef ICE_CPP11_MAPPING
IceInternal::WSEndpoint::operator<(const Endpoint& r) const
#else
IceInternal::WSEndpoint::operator<(const LocalObject& r) const
#endif
{
    const WSEndpoint* p = dynamic_cast<const WSEndpoint*>(&r);
    if(!p)
    {
        const EndpointI* e = dynamic_cast<const EndpointI*>(&r);
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

    if(targetLess(_delegate, p->_delegate))
    {
        return true;
    }
    else if (targetLess(p->_delegate, _delegate))
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

IceInternal::WSEndpointFactory::WSEndpointFactory(const ProtocolInstancePtr& instance, Short type) :
    EndpointFactoryWithUnderlying(instance, type)
{
}

EndpointFactoryPtr
IceInternal::WSEndpointFactory::cloneWithUnderlying(const ProtocolInstancePtr& instance, Short underlying) const
{
    return new WSEndpointFactory(instance, underlying);
}

EndpointIPtr
IceInternal::WSEndpointFactory::createWithUnderlying(const EndpointIPtr& underlying, vector<string>& args, bool) const
{
    return ICE_MAKE_SHARED(WSEndpoint, _instance, underlying, args);
}

EndpointIPtr
IceInternal::WSEndpointFactory::readWithUnderlying(const EndpointIPtr& underlying, InputStream* s) const
{
    return ICE_MAKE_SHARED(WSEndpoint, _instance, underlying, s);
}
