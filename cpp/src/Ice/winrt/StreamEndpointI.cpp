// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/winrt/StreamEndpointI.h>
#include <Ice/winrt/StreamAcceptor.h>
#include <Ice/winrt/StreamConnector.h>
#include <Ice/winrt/StreamTransceiver.h>

#include <Ice/Network.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/HashUtil.h>

#include <IceSSL/EndpointInfo.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

template<class T> class InfoI : public T
{
public:

    InfoI(Ice::Short type, Ice::Int to, bool comp, const string& host, Ice::Int port) :
        T(to, comp, host, port, ""), _type(type)
    {
    }

    virtual Ice::Short
    type() const
    {
        return _type;
    }

    virtual bool
    datagram() const
    {
        return false;
    }

    virtual bool
    secure() const
    {
        return _type == IceSSL::EndpointType;
    }

private:

    Ice::Short _type;
};

}

IceInternal::StreamEndpointI::StreamEndpointI(const ProtocolInstancePtr& instance, const string& ho, Int po, Int ti,
                                              const string& conId, bool co) :
    IPEndpointI(instance, ho, po, getInvalidAddress(), conId),
    _timeout(ti),
    _compress(co)
{
}

IceInternal::StreamEndpointI::StreamEndpointI(const ProtocolInstancePtr& instance) :
    IPEndpointI(instance),
    _timeout(-1),
    _compress(false)
{
}

IceInternal::StreamEndpointI::StreamEndpointI(const ProtocolInstancePtr& instance, BasicStream* s) :
    IPEndpointI(instance, s),
    _timeout(-1),
    _compress(false)
{
    s->read(const_cast<Int&>(_timeout));
    s->read(const_cast<bool&>(_compress));
}

EndpointInfoPtr
IceInternal::StreamEndpointI::getInfo() const
{
    switch(_instance->type())
    {
    case TCPEndpointType:
        return new InfoI<Ice::TCPEndpointInfo>(_instance->type(), _timeout, _compress, _host, _port);
    case IceSSL::EndpointType:
        return new InfoI<IceSSL::EndpointInfo>(_instance->type(), _timeout, _compress, _host, _port);
    default:
        assert(false);
        return 0;
    }
}

Int
IceInternal::StreamEndpointI::timeout() const
{
    return _timeout;
}

EndpointIPtr
IceInternal::StreamEndpointI::timeout(Int timeout) const
{
    if(timeout == _timeout)
    {
        return const_cast<StreamEndpointI*>(this);
    }
    else
    {
        return new StreamEndpointI(_instance, _host, _port, timeout, _connectionId, _compress);
    }
}

EndpointIPtr
IceInternal::StreamEndpointI::connectionId(const string& connectionId) const
{
    if(connectionId == _connectionId)
    {
        return const_cast<StreamEndpointI*>(this);
    }
    else
    {
      return new StreamEndpointI(_instance, _host, _port, _timeout, connectionId, _compress);
    }
}

bool
IceInternal::StreamEndpointI::compress() const
{
    return _compress;
}

EndpointIPtr
IceInternal::StreamEndpointI::compress(bool compress) const
{
    if(compress == _compress)
    {
        return const_cast<StreamEndpointI*>(this);
    }
    else
    {
        return new StreamEndpointI(_instance, _host, _port, _timeout, _connectionId, compress);
    }
}

bool
IceInternal::StreamEndpointI::datagram() const
{
    return false;
}

bool
IceInternal::StreamEndpointI::secure() const
{
    return _instance->type() == IceSSL::EndpointType;
}

TransceiverPtr
IceInternal::StreamEndpointI::transceiver(EndpointIPtr& endp) const
{
    endp = const_cast<StreamEndpointI*>(this);
    return 0;
}

AcceptorPtr
IceInternal::StreamEndpointI::acceptor(EndpointIPtr& endp, const string&) const
{
    StreamAcceptor* p = new StreamAcceptor(_instance, _host, _port);
    endp = createEndpoint(_host, p->effectivePort(), _connectionId);
    return p;
}

string
IceInternal::StreamEndpointI::options() const
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

    if(_timeout != -1)
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
IceInternal::StreamEndpointI::operator==(const LocalObject& r) const
{
    if(!IPEndpointI::operator==(r))
    {
        return false;
    }

    const StreamEndpointI* p = dynamic_cast<const StreamEndpointI*>(&r);
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
IceInternal::StreamEndpointI::operator<(const LocalObject& r) const
{
    const StreamEndpointI* p = dynamic_cast<const StreamEndpointI*>(&r);
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
IceInternal::StreamEndpointI::streamWriteImpl(BasicStream* s) const
{
    IPEndpointI::streamWriteImpl(s);
    s->write(_timeout);
    s->write(_compress);
}

void
IceInternal::StreamEndpointI::hashInit(Ice::Int& h) const
{
    IPEndpointI::hashInit(h);
    hashAdd(h, _timeout);
    hashAdd(h, _compress);
}

bool
IceInternal::StreamEndpointI::checkOption(const string& option, const string& argument, const string& endpoint)
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
        istringstream t(argument);
        if(!(t >> const_cast<Int&>(_timeout)) || !t.eof())
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "invalid timeout value `" + argument + "' in endpoint " + endpoint;
            throw ex;
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

ConnectorPtr
IceInternal::StreamEndpointI::createConnector(const Address& address, const NetworkProxyPtr& proxy) const
{
    // TODO: Add support for network proxies?
    return new StreamConnector(_instance, address, _timeout, _connectionId);
}

IPEndpointIPtr
IceInternal::StreamEndpointI::createEndpoint(const string& host, int port, const string& connectionId) const
{
    return new StreamEndpointI(_instance, host, port, _timeout, connectionId, _compress);
}

IceInternal::StreamEndpointFactory::StreamEndpointFactory(const ProtocolInstancePtr& instance) : _instance(instance)
{
}

IceInternal::StreamEndpointFactory::~StreamEndpointFactory()
{
}

Short
IceInternal::StreamEndpointFactory::type() const
{
    return _instance->type();
}

string
IceInternal::StreamEndpointFactory::protocol() const
{
    return _instance->protocol();
}

EndpointIPtr
IceInternal::StreamEndpointFactory::create(vector<string>& args, bool oaEndpoint) const
{
    IPEndpointIPtr endpt = new StreamEndpointI(_instance);
    endpt->initWithOptions(args, oaEndpoint);
    return endpt;
}

EndpointIPtr
IceInternal::StreamEndpointFactory::read(BasicStream* s) const
{
    return new StreamEndpointI(_instance, s);
}

void
IceInternal::StreamEndpointFactory::destroy()
{
    _instance = 0;
}

EndpointFactoryPtr
IceInternal::StreamEndpointFactory::clone(const ProtocolInstancePtr& instance) const
{
    return new StreamEndpointFactory(instance);
}
