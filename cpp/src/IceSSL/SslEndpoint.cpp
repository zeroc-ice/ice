// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Network.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <Ice/ProtocolPluginFacade.h>
#include <IceSSL/SslEndpoint.h>
#include <IceSSL/SslAcceptor.h>
#include <IceSSL/SslConnector.h>
#include <IceSSL/SslTransceiver.h>
#include <IceSSL/OpenSSLPluginI.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceSSL::SslEndpoint::SslEndpoint(const OpenSSLPluginIPtr& plugin, const string& ho, Int po, Int ti) :
    _plugin(plugin),
    _host(ho),
    _port(po),
    _timeout(ti)
{
}

IceSSL::SslEndpoint::SslEndpoint(const OpenSSLPluginIPtr& plugin, const string& str) :
    _plugin(plugin),
    _port(0),
    _timeout(-1)
{
    const string delim = " \t\n\r";

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
	    EndpointParseException ex(__FILE__, __LINE__);
	    ex.str = "ssl " + str;
	    throw ex;
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
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "ssl " + str;
		    throw ex;
		}
		const_cast<string&>(_host) = argument;
		break;
	    }

	    case 'p':
	    {
		if(argument.empty())
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "ssl " + str;
		    throw ex;
		}
		const_cast<Int&>(_port) = atoi(argument.c_str());
		break;
	    }

	    case 't':
	    {
		if(argument.empty())
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "ssl " + str;
		    throw ex;
		}
		const_cast<Int&>(_timeout) = atoi(argument.c_str());
		break;
	    }

	    default:
	    {
		EndpointParseException ex(__FILE__, __LINE__);
		ex.str = "ssl " + str;
		throw ex;
	    }
	}
    }

    if(_host.empty())
    {
	const_cast<string&>(_host) = _plugin->getProtocolPluginFacade()->getDefaultHost();
    }
}

IceSSL::SslEndpoint::SslEndpoint(const OpenSSLPluginIPtr& plugin, BasicStream* s) :
    _plugin(plugin),
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
IceSSL::SslEndpoint::streamWrite(BasicStream* s) const
{
    s->write(SslEndpointType);
    s->startWriteEncaps();
    s->write(_host);
    s->write(_port);
    s->write(_timeout);
    s->endWriteEncaps();
}

string
IceSSL::SslEndpoint::toString() const
{
    ostringstream s;
    s << "ssl -h " << _host << " -p " << _port;
    if(_timeout != -1)
    {
	s << " -t " << _timeout;
    }
    return s.str();
}

Short
IceSSL::SslEndpoint::type() const
{
    return SslEndpointType;
}

Int
IceSSL::SslEndpoint::timeout() const
{
    return _timeout;
}

EndpointPtr
IceSSL::SslEndpoint::timeout(Int timeout) const
{
    if(timeout == _timeout)
    {
	return const_cast<SslEndpoint*>(this);
    }
    else
    {
	return new SslEndpoint(_plugin, _host, _port, timeout);
    }
}

bool
IceSSL::SslEndpoint::datagram() const
{
    return false;
}

bool
IceSSL::SslEndpoint::secure() const
{
    return true;
}

bool
IceSSL::SslEndpoint::unknown() const
{
    return false;
}

TransceiverPtr
IceSSL::SslEndpoint::clientTransceiver() const
{
    return 0;
}

TransceiverPtr
IceSSL::SslEndpoint::serverTransceiver(EndpointPtr& endp) const
{
    endp = const_cast<SslEndpoint*>(this);
    return 0;
}

ConnectorPtr
IceSSL::SslEndpoint::connector() const
{
    return new SslConnector(_plugin, _host, _port);
}

AcceptorPtr
IceSSL::SslEndpoint::acceptor(EndpointPtr& endp) const
{
    SslAcceptor* p = new SslAcceptor(_plugin, _host, _port);
    endp = new SslEndpoint(_plugin, _host, p->effectivePort(), _timeout);
    return p;
}

bool
IceSSL::SslEndpoint::equivalent(const TransceiverPtr&) const
{
    return false;
}

bool
IceSSL::SslEndpoint::equivalent(const AcceptorPtr& acceptor) const
{
    const SslAcceptor* sslAcceptor = dynamic_cast<const SslAcceptor*>(acceptor.get());
    if(!sslAcceptor)
    {
	return false;
    }
    return sslAcceptor->equivalent(_host, _port);
}

bool
IceSSL::SslEndpoint::operator==(const Endpoint& r) const
{
    const SslEndpoint* p = dynamic_cast<const SslEndpoint*>(&r);
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
IceSSL::SslEndpoint::operator!=(const Endpoint& r) const
{
    return !operator==(r);
}

bool
IceSSL::SslEndpoint::operator<(const Endpoint& r) const
{
    const SslEndpoint* p = dynamic_cast<const SslEndpoint*>(&r);
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

IceSSL::SslEndpointFactory::SslEndpointFactory(const OpenSSLPluginIPtr& plugin)
    : _plugin(plugin)
{
}

IceSSL::SslEndpointFactory::~SslEndpointFactory()
{
}

Short
IceSSL::SslEndpointFactory::type() const
{
    return SslEndpointType;
}

string
IceSSL::SslEndpointFactory::protocol() const
{
    return "ssl";
}

EndpointPtr
IceSSL::SslEndpointFactory::create(const std::string& str) const
{
    return new SslEndpoint(_plugin, str);
}

EndpointPtr
IceSSL::SslEndpointFactory::read(BasicStream* s) const
{
    return new SslEndpoint(_plugin, s);
}

void
IceSSL::SslEndpointFactory::destroy()
{
    _plugin = 0;
}
