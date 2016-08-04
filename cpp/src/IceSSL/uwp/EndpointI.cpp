// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/uwp/EndpointI.h>
#include <IceSSL/uwp/AcceptorI.h>
#include <IceSSL/uwp/ConnectorI.h>
#include <IceSSL/uwp/TransceiverI.h>

#include <Ice/Network.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>
#include <Ice/LocalException.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/HashUtil.h>

#include <IceSSL/EndpointInfo.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

extern "C"
{

Plugin*
createIceSSL(const CommunicatorPtr& com, const string&, const StringSeq&)
{
    IceInternal::ProtocolInstancePtr instance = new IceInternal::ProtocolInstance(com, SSLEndpointType, "ssl", true);
    return new IceInternal::EndpointFactoryPlugin(com, new EndpointFactoryI(instance));
}

}

namespace Ice
{

void
registerIceSSL(bool loadOnInitialize)
{
    Ice::registerPluginFactory("IceSSL", createIceSSL, true);
}

}

#ifndef ICE_CPP11_MAPPING
IceUtil::Shared* IceInternal::upCast(EndpointI* p) { return p; }
#endif

IceSSL::EndpointI::EndpointI(const IceInternal::ProtocolInstancePtr& instance, const string& ho, Int po, Int ti,
                                              const string& conId, bool co) :
    IPEndpointI(instance, ho, po, IceInternal::Address(), conId),
    _timeout(ti),
    _compress(co)
{
}

IceSSL::EndpointI::EndpointI(const IceInternal::ProtocolInstancePtr& instance) :
    IPEndpointI(instance),
    _timeout(-2),
    _compress(false)
{
}

IceSSL::EndpointI::EndpointI(const IceInternal::ProtocolInstancePtr& instance, InputStream* s) :
    IPEndpointI(instance, s),
    _timeout(-1),
    _compress(false)
{
    s->read(const_cast<Int&>(_timeout));
    s->read(const_cast<bool&>(_compress));
}

Ice::EndpointInfoPtr
IceSSL::EndpointI::getInfo() const
{
    IPEndpointInfoPtr info;
    if(_instance->secure())
    {
        info = ICE_MAKE_SHARED(IceInternal::InfoI<IceSSL::EndpointInfo>, ICE_SHARED_FROM_CONST_THIS(EndpointI));
    }
    else
    {
        info = ICE_MAKE_SHARED(IceInternal::InfoI<Ice::TCPEndpointInfo>, ICE_SHARED_FROM_CONST_THIS(EndpointI));
    }
    fillEndpointInfo(info.get());
    return info;
}

Ice::EndpointInfoPtr
IceSSL::EndpointI::getWSInfo(const string& resource) const
{
    IPEndpointInfoPtr info;
    IceSSL::WSSEndpointInfoPtr i = ICE_MAKE_SHARED(IceInternal::InfoI<IceSSL::WSSEndpointInfo>, ICE_SHARED_FROM_CONST_THIS(EndpointI));
    i->resource = resource;
    info = i;
    fillEndpointInfo(info.get());
    return info;
}

Int
IceSSL::EndpointI::timeout() const
{
    return _timeout;
}

IceInternal::EndpointIPtr
IceSSL::EndpointI::timeout(Int timeout) const
{
    if(timeout == _timeout)
    {
        return ICE_SHARED_FROM_CONST_THIS(EndpointI);
    }
    else
    {
        return ICE_MAKE_SHARED(EndpointI, _instance, _host, _port, timeout, _connectionId, _compress);
    }
}

IceInternal::EndpointIPtr
IceSSL::EndpointI::connectionId(const string& connectionId) const
{
    if(connectionId == _connectionId)
    {
        return ICE_SHARED_FROM_CONST_THIS(EndpointI);
    }
    else
    {
      return ICE_MAKE_SHARED(EndpointI, _instance, _host, _port, _timeout, connectionId, _compress);
    }
}

bool
IceSSL::EndpointI::compress() const
{
    return _compress;
}

IceInternal::EndpointIPtr
IceSSL::EndpointI::compress(bool compress) const
{
    if(compress == _compress)
    {
        return ICE_SHARED_FROM_CONST_THIS(EndpointI);
    }
    else
    {
        return ICE_MAKE_SHARED(EndpointI, _instance, _host, _port, _timeout, _connectionId, compress);
    }
}

bool
IceSSL::EndpointI::datagram() const
{
    return false;
}

bool
IceSSL::EndpointI::secure() const
{
    return true;
}

IceInternal::TransceiverPtr
IceSSL::EndpointI::transceiver() const
{
    return 0;
}

IceInternal::AcceptorPtr
IceSSL::EndpointI::acceptor(const string&) const
{
    return new AcceptorI(ICE_DYNAMIC_CAST(EndpointI, ICE_SHARED_FROM_CONST_THIS(EndpointI)), _instance, _host, _port);
}

EndpointIPtr
IceSSL::EndpointI::endpoint(const AcceptorIPtr& acceptor) const
{
    return ICE_MAKE_SHARED(EndpointI, _instance, _host, acceptor->effectivePort(), _timeout, _connectionId, _compress);
}

string
IceSSL::EndpointI::options() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    ostringstream s;

    s << IPEndpointI::options();

    if(_timeout == -1)
    {
        s << " -t infinite";
    }
    else
    {
        s << " -t " << _timeout;
    }

    if(_compress)
    {
        s << " -z";
    }

    return s.str();
}

bool
#ifdef ICE_CPP11_MAPPING
IceSSL::EndpointI::operator==(const Endpoint& r) const
#else
IceSSL::EndpointI::operator==(const LocalObject& r) const
#endif
{
    if(!IPEndpointI::operator==(r))
    {
        return false;
    }

    const EndpointI* p = dynamic_cast<const EndpointI*>(&r);
    if(!p)
    {
        return false;
    }

    if(this == p)
    {
        return true;
    }

    if(_timeout != p->_timeout)
    {
        return false;
    }

    if(_compress != p->_compress)
    {
        return false;
    }

    return true;
}

bool
#ifdef ICE_CPP11_MAPPING
IceSSL::EndpointI::operator<(const Endpoint& r) const
#else
IceSSL::EndpointI::operator<(const LocalObject& r) const
#endif
{
    const EndpointI* p = dynamic_cast<const EndpointI*>(&r);
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

    if(_timeout < p->_timeout)
    {
        return true;
    }
    else if(p->_timeout < _timeout)
    {
        return false;
    }

    if(!_compress && p->_compress)
    {
        return true;
    }
    else if(p->_compress < _compress)
    {
        return false;
    }

    return IPEndpointI::operator<(r);
}

void
IceSSL::EndpointI::streamWriteImpl(OutputStream* s) const
{
    IPEndpointI::streamWriteImpl(s);
    s->write(_timeout);
    s->write(_compress);
}

void
IceSSL::EndpointI::hashInit(Ice::Int& h) const
{
    IPEndpointI::hashInit(h);
    IceInternal::hashAdd(h, _timeout);
    IceInternal::hashAdd(h, _compress);
}

void
IceSSL::EndpointI::fillEndpointInfo(IPEndpointInfo* info) const
{
    IPEndpointI::fillEndpointInfo(info);
    info->timeout = _timeout;
    info->compress = _compress;
}

void
IceSSL::EndpointI::initWithOptions(vector<string>& args, bool oaEndpoint)
{
    IPEndpointI::initWithOptions(args, oaEndpoint);

    if(_timeout == -2)
    {
        const_cast<Int&>(_timeout) = _instance->defaultTimeout();
    }
}

bool
IceSSL::EndpointI::checkOption(const string& option, const string& argument, const string& endpoint)
{
    if(IPEndpointI::checkOption(option, argument, endpoint))
    {
        return true;
    }

    switch(option[1])
    {
    case 't':
    {
        if(argument.empty())
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "no argument provided for -t option in endpoint " + endpoint;
            throw ex;
        }

        if(argument == "infinite")
        {
            const_cast<Int&>(_timeout) = -1;
        }
        else
        {
            istringstream t(argument);
            if(!(t >> const_cast<Int&>(_timeout)) || !t.eof() || _timeout < 1)
            {
                EndpointParseException ex(__FILE__, __LINE__);
                ex.str = "invalid timeout value `" + argument + "' in endpoint " + endpoint;
                throw ex;
            }
        }
        return true;
    }

    case 'z':
    {
        if(!argument.empty())
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "unexpected argument `" + argument + "' provided for -z option in " + endpoint;
            throw ex;
        }
        const_cast<bool&>(_compress) = true;
        return true;
    }

    default:
    {
        return false;
    }
    }
}

IceInternal::ConnectorPtr
IceSSL::EndpointI::createConnector(const IceInternal::Address& address, const IceInternal::NetworkProxyPtr& proxy) const
{
    // TODO: Add support for network proxies?
    return new ConnectorI(_instance, address, _timeout, _connectionId);
}

IceInternal::IPEndpointIPtr
IceSSL::EndpointI::createEndpoint(const string& host, int port, const string& connectionId) const
{
    return ICE_MAKE_SHARED(EndpointI, _instance, host, port, _timeout, connectionId, _compress);
}

IceSSL::EndpointFactoryI::EndpointFactoryI(const IceInternal::ProtocolInstancePtr& instance) : _instance(instance)
{
}

Short
IceSSL::EndpointFactoryI::type() const
{
    return _instance->type();
}

string
IceSSL::EndpointFactoryI::protocol() const
{
    return _instance->protocol();
}

IceInternal::EndpointIPtr
IceSSL::EndpointFactoryI::create(vector<string>& args, bool oaEndpoint) const
{
    IceInternal::IPEndpointIPtr endpt = ICE_MAKE_SHARED(EndpointI, _instance);
    endpt->initWithOptions(args, oaEndpoint);
    return endpt;
}

IceInternal::EndpointIPtr
IceSSL::EndpointFactoryI::read(InputStream* s) const
{
    return ICE_MAKE_SHARED(EndpointI, _instance, s);
}

void
IceSSL::EndpointFactoryI::destroy()
{
    _instance = 0;
}

IceInternal::EndpointFactoryPtr
IceSSL::EndpointFactoryI::clone(const IceInternal::ProtocolInstancePtr& instance) const
{
    return new EndpointFactoryI(instance);
}
