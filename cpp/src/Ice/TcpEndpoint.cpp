// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/TcpEndpoint.h>
#include <Ice/Network.h>
#include <Ice/TcpAcceptor.h>
#include <Ice/TcpConnector.h>
#include <Ice/TcpTransceiver.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/DefaultsAndOverrides.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::TcpEndpoint::TcpEndpoint(const InstancePtr& instance, const string& ho, Int po, Int ti) :
    _instance(instance),
    _host(ho),
    _port(po),
    _timeout(ti)
{
}

IceInternal::TcpEndpoint::TcpEndpoint(const InstancePtr& instance, const string& str) :
    _instance(instance),
    _port(0),
    _timeout(-1)
{
    static const string delim = " \t\n\r";

    string::size_type beg;
    string::size_type end = 0;

    while(true)
    {
	beg = str.find_first_not_of(delim, end);
	if(beg == string::npos)
	{
	    break;
	}
	
	end = str.find_first_of(delim, beg);
	if(end == string::npos)
	{
	    end = str.length();
	}

	string option = str.substr(beg, end - beg);
	if(option.length() != 2 || option[0] != '-')
	{
	    throw EndpointParseException(__FILE__, __LINE__);
	}

	string argument;
	string::size_type argumentBeg = str.find_first_not_of(delim, end);
	if(argumentBeg != string::npos && str[argumentBeg] != '-')
	{
	    beg = argumentBeg;
	    end = str.find_first_of(delim, beg);
	    if(end == string::npos)
	    {
		end = str.length();
	    }
	    argument = str.substr(beg, end - beg);
	}

	switch(option[1])
	{
	    case 'h':
	    {
		if(argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}
		const_cast<string&>(_host) = argument;
		break;
	    }

	    case 'p':
	    {
		if(argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}
		const_cast<Int&>(_port) = atoi(argument.c_str());
		break;
	    }

	    case 't':
	    {
		if(argument.empty())
		{
		    throw EndpointParseException(__FILE__, __LINE__);
		}
		const_cast<Int&>(_timeout) = atoi(argument.c_str());
		break;
	    }

	    default:
	    {
		throw EndpointParseException(__FILE__, __LINE__);
	    }
	}
    }

    if(_host.empty())
    {
	const_cast<string&>(_host) = _instance->defaultsAndOverrides()->defaultHost;
    }
}

IceInternal::TcpEndpoint::TcpEndpoint(BasicStream* s) :
    _instance(s->instance()),
    _port(0),
    _timeout(-1)
{
    s->startReadEncaps();
    s->read(const_cast<string&>(_host));
    s->read(const_cast<Int&>(_port));
    s->read(const_cast<Int&>(_timeout));
    s->endReadEncaps();
}

void
IceInternal::TcpEndpoint::streamWrite(BasicStream* s) const
{
    s->write(TcpEndpointType);
    s->startWriteEncaps();
    s->write(_host);
    s->write(_port);
    s->write(_timeout);
    s->endWriteEncaps();
}

string
IceInternal::TcpEndpoint::toString() const
{
    ostringstream s;
    s << "tcp -h " << _host << " -p " << _port;
    if(_timeout != -1)
    {
	s << " -t " << _timeout;
    }
    return s.str();
}

Short
IceInternal::TcpEndpoint::type() const
{
    return TcpEndpointType;
}

Int
IceInternal::TcpEndpoint::timeout() const
{
    return _timeout;
}

EndpointPtr
IceInternal::TcpEndpoint::timeout(Int timeout) const
{
    if(timeout == _timeout)
    {
	return const_cast<TcpEndpoint*>(this);
    }
    else
    {
	return new TcpEndpoint(_instance, _host, _port, timeout);
    }
}

bool
IceInternal::TcpEndpoint::datagram() const
{
    return false;
}

bool
IceInternal::TcpEndpoint::secure() const
{
    return false;
}

bool
IceInternal::TcpEndpoint::unknown() const
{
    return false;
}

TransceiverPtr
IceInternal::TcpEndpoint::clientTransceiver() const
{
    return 0;
}

TransceiverPtr
IceInternal::TcpEndpoint::serverTransceiver(EndpointPtr& endp) const
{
    endp = const_cast<TcpEndpoint*>(this);
    return 0;
}

ConnectorPtr
IceInternal::TcpEndpoint::connector() const
{
    return new TcpConnector(_instance, _host, _port);
}

AcceptorPtr
IceInternal::TcpEndpoint::acceptor(EndpointPtr& endp) const
{
    TcpAcceptor* p = new TcpAcceptor(_instance, _host, _port);
    endp = new TcpEndpoint(_instance, _host, p->effectivePort(), _timeout);
    return p;
}

bool
IceInternal::TcpEndpoint::equivalent(const TransceiverPtr&) const
{
    return false;
}

bool
IceInternal::TcpEndpoint::equivalent(const AcceptorPtr& acceptor) const
{
    const TcpAcceptor* tcpAcceptor = dynamic_cast<const TcpAcceptor*>(acceptor.get());
    if(!tcpAcceptor)
    {
	return false;
    }
    return tcpAcceptor->equivalent(_host, _port);
}

bool
IceInternal::TcpEndpoint::operator==(const Endpoint& r) const
{
    const TcpEndpoint* p = dynamic_cast<const TcpEndpoint*>(&r);
    if(!p)
    {
	return false;
    }

    if(this == p)
    {
	return true;
    }

    if(_port != p->_port)
    {
	return false;
    }

    if(_timeout != p->_timeout)
    {
	return false;
    }

    if(_host != p->_host)
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
IceInternal::TcpEndpoint::operator!=(const Endpoint& r) const
{
    return !operator==(r);
}

bool
IceInternal::TcpEndpoint::operator<(const Endpoint& r) const
{
    const TcpEndpoint* p = dynamic_cast<const TcpEndpoint*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    if(this == p)
    {
	return false;
    }

    if(_port < p->_port)
    {
	return true;
    }
    else if(p->_port < _port)
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

    if(_host != p->_host)
    {
	//
	// We do the most time-consuming part of the comparison last.
	//
	struct sockaddr_in laddr;
	struct sockaddr_in raddr;
	getAddress(_host, _port, laddr);
	getAddress(p->_host, p->_port, raddr);
	if(laddr.sin_addr.s_addr < raddr.sin_addr.s_addr)
	{
	    return true;
	}
	else if(raddr.sin_addr.s_addr < laddr.sin_addr.s_addr)
	{
	    return false;
	}
    }

    return false;
}

IceInternal::TcpEndpointFactory::TcpEndpointFactory(const InstancePtr& instance)
    : _instance(instance)
{
}

IceInternal::TcpEndpointFactory::~TcpEndpointFactory()
{
}

Short
IceInternal::TcpEndpointFactory::type() const
{
    return TcpEndpointType;
}

const string&
IceInternal::TcpEndpointFactory::protocol() const
{
    const static string result = "tcp";
    return result;
}

EndpointPtr
IceInternal::TcpEndpointFactory::create(const std::string& str) const
{
    return new TcpEndpoint(_instance, str);
}

EndpointPtr
IceInternal::TcpEndpointFactory::read(BasicStream* s) const
{
    return new TcpEndpoint(s);
}

void
IceInternal::TcpEndpointFactory::destroy()
{
    _instance = 0;
}
