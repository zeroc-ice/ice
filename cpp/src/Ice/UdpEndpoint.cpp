// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/UdpEndpoint.h>
#include <Ice/Network.h>
#include <Ice/UdpTransceiver.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/DefaultsAndOverrides.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::UdpEndpoint::UdpEndpoint(const InstancePtr& instance, const string& ho, Int po) :
    _instance(instance),
    _host(ho),
    _port(po),
    _connect(false)
{
}

IceInternal::UdpEndpoint::UdpEndpoint(const InstancePtr& instance, const string& str) :
    _instance(instance),
    _port(0),
    _connect(false)
{
    static const string delim = " \t\n\r";

    string::size_type beg;
    string::size_type end = 0;

    while (true)
    {
	beg = str.find_first_not_of(delim, end);
	if (beg == string::npos)
	{
	    break;
	}
	
	end = str.find_first_of(delim, beg);
	if (end == string::npos)
	{
	    end = str.length();
	}

	string option = str.substr(beg, end - beg);
	if (option.length() != 2 || option[0] != '-')
	{
	    throw EndpointParseException(__FILE__, __LINE__);
	}

	string argument;
	string::size_type argumentBeg = str.find_first_not_of(delim, end);
	if (argumentBeg != string::npos && str[argumentBeg] != '-')
	{
	    beg = argumentBeg;
	    end = str.find_first_of(delim + ":", beg);
	    if (end == string::npos)
	    {
		end = str.length();
	    }
	    argument = str.substr(beg, end - beg);
	}

	switch (option[1])
	{
	    case 'h':
	    {
		if (argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}
		const_cast<string&>(_host) = argument;
		break;
	    }

	    case 'p':
	    {
		if (argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}
		const_cast<Int&>(_port) = atoi(argument.c_str());
		break;
	    }

	    case 'c':
	    {
		if (!argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}
		const_cast<bool&>(_connect) = true;
		break;
	    }

	    default:
	    {
		throw EndpointParseException(__FILE__, __LINE__);
	    }
	}
    }

    if (_host.empty())
    {
	const_cast<string&>(_host) = _instance->defaultsAndOverrides()->defaultHost;
    }
}

IceInternal::UdpEndpoint::UdpEndpoint(BasicStream* s) :
    _instance(s->instance()),
    _port(0),
    _connect(false)
{
    s->startReadEncaps();
    s->read(const_cast<string&>(_host));
    s->read(const_cast<Int&>(_port));
    // Not transmitted.
    //s->read(const_cast<bool&>(_connect));
    s->endReadEncaps();
}

void
IceInternal::UdpEndpoint::streamWrite(BasicStream* s) const
{
    s->write(UdpEndpointType);
    s->startWriteEncaps();
    s->write(_host);
    s->write(_port);
    // Not transmitted.
    //s->write(_connect);
    s->endWriteEncaps();
}

string
IceInternal::UdpEndpoint::toString() const
{
    ostringstream s;
    s << "udp -h " << _host << " -p " << _port;
    if (_connect)
    {
	s << " -c";
    }
    return s.str();
}

Short
IceInternal::UdpEndpoint::type() const
{
    return UdpEndpointType;
}

Int
IceInternal::UdpEndpoint::timeout() const
{
    return -1;
}

EndpointPtr
IceInternal::UdpEndpoint::timeout(Int) const
{
    return const_cast<UdpEndpoint*>(this);
}

bool
IceInternal::UdpEndpoint::datagram() const
{
    return true;
}

bool
IceInternal::UdpEndpoint::secure() const
{
    return false;
}

bool
IceInternal::UdpEndpoint::unknown() const
{
    return false;
}

TransceiverPtr
IceInternal::UdpEndpoint::clientTransceiver() const
{
    return new UdpTransceiver(_instance, _host, _port);
}

TransceiverPtr
IceInternal::UdpEndpoint::serverTransceiver(EndpointPtr& endp) const
{
    UdpTransceiver* p = new UdpTransceiver(_instance, _host, _port, _connect);
    endp = new UdpEndpoint(_instance, _host, p->effectivePort());
    return p;
}

ConnectorPtr
IceInternal::UdpEndpoint::connector() const
{
    return 0;
}

AcceptorPtr
IceInternal::UdpEndpoint::acceptor(EndpointPtr& endp) const
{
    endp = const_cast<UdpEndpoint*>(this);
    return 0;
}

bool
IceInternal::UdpEndpoint::equivalent(const TransceiverPtr& transceiver) const
{
    const UdpTransceiver* udpTransceiver = dynamic_cast<const UdpTransceiver*>(transceiver.get());
    if (!udpTransceiver)
    {
	return false;
    }
    return udpTransceiver->equivalent(_host, _port);
}

bool
IceInternal::UdpEndpoint::equivalent(const AcceptorPtr&) const
{
    return false;
}

bool
IceInternal::UdpEndpoint::operator==(const Endpoint& r) const
{
    const UdpEndpoint* p = dynamic_cast<const UdpEndpoint*>(&r);
    if (!p)
    {
	return false;
    }

    if (this == p)
    {
	return true;
    }

    if (_port != p->_port)
    {
	return false;
    }

    if (_connect != p->_connect)
    {
	return false;
    }

    if (_host != p->_host)
    {
	//
	// We do the most time-consuming part of the comparison last.
	//
	struct sockaddr_in laddr;
	struct sockaddr_in raddr;
	getAddress(_host, _port, laddr);
	getAddress(p->_host, p->_port, raddr);
        return compareAddress(laddr, raddr);
    }

    return true;
}

bool
IceInternal::UdpEndpoint::operator!=(const Endpoint& r) const
{
    return !operator==(r);
}

bool
IceInternal::UdpEndpoint::operator<(const Endpoint& r) const
{
    const UdpEndpoint* p = dynamic_cast<const UdpEndpoint*>(&r);
    if (!p)
    {
        return type() < r.type();
    }

    if (this == p)
    {
	return false;
    }

    if (_port < p->_port)
    {
	return true;
    }
    else if (p->_port < _port)
    {
	return false;
    }

    if (!_connect && p->_connect)
    {
	return true;
    }
    else if (!p->_connect && _connect)
    {
	return false;
    }

    if (_host != p->_host)
    {
	//
	// We do the most time-consuming part of the comparison last.
	//
	struct sockaddr_in laddr;
	struct sockaddr_in raddr;
	getAddress(_host, _port, laddr);
	getAddress(p->_host, p->_port, raddr);
	if (laddr.sin_addr.s_addr < raddr.sin_addr.s_addr)
	{
	    return true;
	}
	else if (raddr.sin_addr.s_addr < laddr.sin_addr.s_addr)
	{
	    return false;
	}
    }

    return false;
}

IceInternal::UdpEndpointFactory::UdpEndpointFactory(const InstancePtr& instance)
    : _instance(instance)
{
}

IceInternal::UdpEndpointFactory::~UdpEndpointFactory()
{
}

Short
IceInternal::UdpEndpointFactory::type() const
{
    return UdpEndpointType;
}

const string&
IceInternal::UdpEndpointFactory::protocol() const
{
    const static string result = "udp";
    return result;
}

EndpointPtr
IceInternal::UdpEndpointFactory::create(const std::string& str) const
{
    return new UdpEndpoint(_instance, str);
}

EndpointPtr
IceInternal::UdpEndpointFactory::read(BasicStream* s) const
{
    return new UdpEndpoint(s);
}

void
IceInternal::UdpEndpointFactory::destroy()
{
    _instance = 0;
}
