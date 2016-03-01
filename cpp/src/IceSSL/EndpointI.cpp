// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/EndpointI.h>
#include <IceSSL/AcceptorI.h>
#include <IceSSL/ConnectorI.h>
#include <IceSSL/Instance.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Object.h>
#include <Ice/HashUtil.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

IceUtil::Shared* IceSSL::upCast(EndpointI* p) { return p; }

IceSSL::EndpointI::EndpointI(const InstancePtr& instance, const string& ho, Int po,
                             const IceInternal::Address& sourceAddr, Int ti, const string& conId, bool co) :
    IceInternal::IPEndpointI(instance, ho, po, sourceAddr, conId),
    _instance(instance),
    _timeout(ti),
    _compress(co)
{
}

IceSSL::EndpointI::EndpointI(const InstancePtr& instance) :
    IceInternal::IPEndpointI(instance),
    _instance(instance),
    _timeout(instance->defaultTimeout()),
    _compress(false)
{
}

IceSSL::EndpointI::EndpointI(const InstancePtr& instance, IceInternal::BasicStream* s) :
    IPEndpointI(instance, s),
    _instance(instance),
    _timeout(-1),
    _compress(false)
{
    s->read(const_cast<Int&>(_timeout));
    s->read(const_cast<bool&>(_compress));
}

Ice::EndpointInfoPtr
IceSSL::EndpointI::getInfo() const
{
    EndpointInfoPtr info = new IceInternal::InfoI<EndpointInfo>(const_cast<EndpointI*>(this));
    fillEndpointInfo(info.get());
    return info;
}

Ice::EndpointInfoPtr
IceSSL::EndpointI::getWSInfo(const string& resource) const
{
    WSSEndpointInfoPtr info = new IceInternal::InfoI<WSSEndpointInfo>(const_cast<EndpointI*>(this));
    fillEndpointInfo(info.get());
    info->resource = resource;
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
        return const_cast<EndpointI*>(this);
    }
    else
    {
        return new EndpointI(_instance, _host, _port, _sourceAddr, timeout, _connectionId, _compress);
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
        return const_cast<EndpointI*>(this);
    }
    else
    {
        return new EndpointI(_instance, _host, _port, _sourceAddr, _timeout, _connectionId, compress);
    }
}

bool
IceSSL::EndpointI::datagram() const
{
    return false;
}

IceInternal::TransceiverPtr
IceSSL::EndpointI::transceiver() const
{
    return 0;
}

IceInternal::AcceptorPtr
IceSSL::EndpointI::acceptor(const string& adapterName) const
{
    return new AcceptorI(const_cast<EndpointI*>(this), _instance, adapterName, _host, _port);
}

EndpointIPtr
IceSSL::EndpointI::endpoint(const AcceptorIPtr& acceptor) const
{
    return new EndpointI(_instance, _host, acceptor->effectivePort(), _sourceAddr, _timeout, _connectionId, _compress);
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
IceSSL::EndpointI::operator==(const Ice::LocalObject& r) const
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
IceSSL::EndpointI::operator<(const Ice::LocalObject& r) const
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
IceSSL::EndpointI::streamWriteImpl(IceInternal::BasicStream* s) const
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
    return new ConnectorI(_instance, _host, address, proxy, _sourceAddr, _timeout, _connectionId);
}

IceInternal::IPEndpointIPtr
IceSSL::EndpointI::createEndpoint(const string& host, int port, const string& connectionId) const
{
    return new EndpointI(_instance, host, port, _sourceAddr, _timeout, connectionId, _compress);
}

IceSSL::EndpointFactoryI::EndpointFactoryI(const InstancePtr& instance) : _instance(instance)
{
}

IceSSL::EndpointFactoryI::~EndpointFactoryI()
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
    IceInternal::IPEndpointIPtr endpt = new EndpointI(_instance);
    endpt->initWithOptions(args, oaEndpoint);
    return endpt;
}

IceInternal::EndpointIPtr
IceSSL::EndpointFactoryI::read(IceInternal::BasicStream* s) const
{
    return new EndpointI(_instance, s);
}

void
IceSSL::EndpointFactoryI::destroy()
{
    _instance = 0;
}

IceInternal::EndpointFactoryPtr
IceSSL::EndpointFactoryI::clone(const IceInternal::ProtocolInstancePtr& instance) const
{
    return new EndpointFactoryI(new Instance(_instance->engine(), instance->type(), instance->protocol()));
}
