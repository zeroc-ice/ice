// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/UdpEndpointI.h>
#include <Ice/Network.h>
#include <Ice/UdpConnector.h>
#include <Ice/UdpTransceiver.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <Ice/Protocol.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/HashUtil.h>
#include <Ice/Logger.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(UdpEndpointI* p) { return p; }

extern "C"
{

Plugin*
createIceUDP(const CommunicatorPtr& c, const string&, const StringSeq&)
{
    return new EndpointFactoryPlugin(c, new UdpEndpointFactory(new ProtocolInstance(c, UDPEndpointType, "udp", false)));
}

}

IceInternal::UdpEndpointI::UdpEndpointI(const ProtocolInstancePtr& instance, const string& host, Int port,
                                        const Address& sourceAddr, const string& mcastInterface, Int mttl, bool conn,
                                        const string& conId, bool co) :
    IPEndpointI(instance, host, port, sourceAddr, conId),
    _mcastTtl(mttl),
    _mcastInterface(mcastInterface),
    _connect(conn),
    _compress(co)
{
}

IceInternal::UdpEndpointI::UdpEndpointI(const ProtocolInstancePtr& instance) :
    IPEndpointI(instance),
    _mcastTtl(-1),
    _connect(false),
    _compress(false)
{
}

IceInternal::UdpEndpointI::UdpEndpointI(const ProtocolInstancePtr& instance, BasicStream* s) :
    IPEndpointI(instance, s),
    _mcastTtl(-1),
    _connect(false),
    _compress(false)
{
    if(s->getReadEncoding() == Ice::Encoding_1_0)
    {
        Ice::Byte b;
        s->read(b);
        s->read(b);
        s->read(b);
        s->read(b);
    }
    // Not transmitted.
    //s->read(const_cast<bool&>(_connect));
    s->read(const_cast<bool&>(_compress));
}

EndpointInfoPtr
IceInternal::UdpEndpointI::getInfo() const
{
    Ice::UDPEndpointInfoPtr info = new InfoI<Ice::UDPEndpointInfo>(const_cast<UdpEndpointI*>(this));
    fillEndpointInfo(info.get());
    return info;
}

Int
IceInternal::UdpEndpointI::timeout() const
{
    return -1;
}

EndpointIPtr
IceInternal::UdpEndpointI::timeout(Int) const
{
    return const_cast<UdpEndpointI*>(this);
}

bool
IceInternal::UdpEndpointI::compress() const
{
    return _compress;
}

EndpointIPtr
IceInternal::UdpEndpointI::compress(bool compress) const
{
    if(compress == _compress)
    {
        return const_cast<UdpEndpointI*>(this);
    }
    else
    {
        return new UdpEndpointI(_instance, _host, _port, _sourceAddr, _mcastInterface, _mcastTtl, _connect,
                                _connectionId, compress);
    }
}

bool
IceInternal::UdpEndpointI::datagram() const
{
    return true;
}

TransceiverPtr
IceInternal::UdpEndpointI::transceiver() const
{
    return new UdpTransceiver(const_cast<UdpEndpointI*>(this), _instance, _host, _port, _mcastInterface, _connect);
}

AcceptorPtr
IceInternal::UdpEndpointI::acceptor(const string&) const
{
    return 0;
}

UdpEndpointIPtr
IceInternal::UdpEndpointI::endpoint(const UdpTransceiverPtr& transceiver) const
{
    return new UdpEndpointI(_instance, _host, transceiver->effectivePort(), _sourceAddr, _mcastInterface, _mcastTtl,
                            _connect, _connectionId, _compress);
}

string
IceInternal::UdpEndpointI::options() const
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

    if(_mcastInterface.length() > 0)
    {
        s << " --interface " << _mcastInterface;
    }

    if(_mcastTtl != -1)
    {
        s << " --ttl " << _mcastTtl;
    }

    if(_connect)
    {
        s << " -c";
    }

    if(_compress)
    {
        s << " -z";
    }

    return s.str();
}

bool
IceInternal::UdpEndpointI::operator==(const LocalObject& r) const
{
    if(!IPEndpointI::operator==(r))
    {
        return false;
    }

    const UdpEndpointI* p = dynamic_cast<const UdpEndpointI*>(&r);
    if(!p)
    {
        return false;
    }

    if(this == p)
    {
        return true;
    }

    if(_compress != p->_compress)
    {
        return false;
    }

    if(_connect != p->_connect)
    {
        return false;
    }

    if(_mcastTtl != p->_mcastTtl)
    {
        return false;
    }

    if(_mcastInterface != p->_mcastInterface)
    {
        return false;
    }

    return true;
}

bool
IceInternal::UdpEndpointI::operator<(const LocalObject& r) const
{
    const UdpEndpointI* p = dynamic_cast<const UdpEndpointI*>(&r);
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

    if(!_compress && p->_compress)
    {
        return true;
    }
    else if(p->_compress < _compress)
    {
        return false;
    }

    if(!_connect && p->_connect)
    {
        return true;
    }
    else if(!p->_connect && _connect)
    {
        return false;
    }

    if(_mcastTtl < p->_mcastTtl)
    {
        return true;
    }
    else if(p->_mcastTtl < _mcastTtl)
    {
        return false;
    }

    if(_mcastInterface < p->_mcastInterface)
    {
        return true;
    }
    else if(p->_mcastInterface < _mcastInterface)
    {
        return false;
    }

    return IPEndpointI::operator<(r);
}

void
IceInternal::UdpEndpointI::streamWriteImpl(BasicStream* s) const
{
    IPEndpointI::streamWriteImpl(s);
    if(s->getWriteEncoding() == Ice::Encoding_1_0)
    {
        s->write(Ice::Protocol_1_0);
        s->write(Ice::Encoding_1_0);
    }
    // Not transmitted.
    //s->write(_connect);
    s->write(_compress);
}

void
IceInternal::UdpEndpointI::hashInit(Ice::Int& h) const
{
    IPEndpointI::hashInit(h);
    hashAdd(h, _mcastInterface);
    hashAdd(h, _mcastTtl);
    hashAdd(h, _connect);
    hashAdd(h, _compress);
}

void
IceInternal::UdpEndpointI::fillEndpointInfo(IPEndpointInfo* info) const
{
    IPEndpointI::fillEndpointInfo(info);
    UDPEndpointInfo* udpInfo = dynamic_cast<UDPEndpointInfo*>(info);
    if(udpInfo)
    {
        udpInfo->timeout = -1;
        udpInfo->compress = _compress;
        udpInfo->mcastTtl = _mcastTtl;
        udpInfo->mcastInterface = _mcastInterface;
    }
}

bool
IceInternal::UdpEndpointI::checkOption(const string& option, const string& argument, const string& endpoint)
{
    if(IPEndpointI::checkOption(option, argument, endpoint))
    {
        return true;
    }

    if(option == "-c")
    {
        if(!argument.empty())
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "unexpected argument `" + argument + "' provided for -c option in " + endpoint;
            throw ex;
        }
        const_cast<bool&>(_connect) = true;
    }
    else if(option == "-z")
    {
        if(!argument.empty())
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "unexpected argument `" + argument + "' provided for -z option in " + endpoint;
            throw ex;
        }
        const_cast<bool&>(_compress) = true;
    }
    else if(option == "-v" || option == "-e")
    {
        if(argument.empty())
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "no argument provided for " + option + " option in endpoint " + endpoint;
            throw ex;
        }
        try
        {
            Ice::Byte major, minor;
            IceInternal::stringToMajorMinor(argument, major, minor);
            if(major != 1 || minor != 0)
            {
                _instance->logger()->warning("deprecated udp endpoint option: " + option);
            }
        }
        catch(const VersionParseException& e)
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "invalid version `" + argument + "' in endpoint " + endpoint + ":\n" + e.str;
            throw ex;
        }
    }
    else if(option == "--interface")
    {
        if(argument.empty())
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "no argument provided for --interface option in endpoint " + endpoint;
            throw ex;
        }
        const_cast<string&>(_mcastInterface) = argument;
    }
    else if(option == "--ttl")
    {
        if(argument.empty())
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "no argument provided for --ttl option in endpoint " + endpoint;
            throw ex;
        }
        istringstream p(argument);
        if(!(p >> const_cast<Int&>(_mcastTtl)) || !p.eof())
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "invalid TTL value `" + argument + "' in endpoint " + endpoint;
            throw ex;
        }
    }
    else
    {
        return false;
    }
    return true;
}

ConnectorPtr
IceInternal::UdpEndpointI::createConnector(const Address& address, const NetworkProxyPtr&) const
{
    return new UdpConnector(_instance, address, _sourceAddr, _mcastInterface, _mcastTtl, _connectionId);
}

IPEndpointIPtr
IceInternal::UdpEndpointI::createEndpoint(const string& host, int port, const string& connectionId) const
{
    return new UdpEndpointI(_instance, host, port, _sourceAddr, _mcastInterface, _mcastTtl, _connect, connectionId,
                            _compress);
}

IceInternal::UdpEndpointFactory::UdpEndpointFactory(const ProtocolInstancePtr& instance) : _instance(instance)
{
}

IceInternal::UdpEndpointFactory::~UdpEndpointFactory()
{
}

Short
IceInternal::UdpEndpointFactory::type() const
{
    return _instance->type();
}

string
IceInternal::UdpEndpointFactory::protocol() const
{
    return _instance->protocol();
}

EndpointIPtr
IceInternal::UdpEndpointFactory::create(vector<string>& args, bool oaEndpoint) const
{
    IPEndpointIPtr endpt = new UdpEndpointI(_instance);
    endpt->initWithOptions(args, oaEndpoint);
    return endpt;
}

EndpointIPtr
IceInternal::UdpEndpointFactory::read(BasicStream* s) const
{
    return new UdpEndpointI(_instance, s);
}

void
IceInternal::UdpEndpointFactory::destroy()
{
    _instance = 0;
}

EndpointFactoryPtr
IceInternal::UdpEndpointFactory::clone(const ProtocolInstancePtr& instance) const
{
    return new UdpEndpointFactory(instance);
}
