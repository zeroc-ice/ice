// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TcpTransport/TcpEndpoint.h>
#include <IceE/Network.h>
#include <IceE/Connector.h>
#include <IceE/Transceiver.h>
#include <IceE/BasicStream.h>
#include <IceE/LocalException.h>
#include <IceE/Instance.h>
#include <IceE/DefaultsAndOverrides.h>
#include <IceE/SafeStdio.h>
#ifndef ICEE_PURE_CLIENT
#   include <IceE/Acceptor.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::TcpEndpoint::TcpEndpoint(const InstancePtr& instance, const string& ho, Int po, Int ti, bool pub) :
    _instance(instance),
    _host(ho),
    _port(po),
    _timeout(ti),
    _publish(pub)
{
}

IceInternal::TcpEndpoint::TcpEndpoint(const InstancePtr& instance, const string& str) :
    _instance(instance),
    _port(0),
    _timeout(-1),
    _publish(true)
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
	    ex.str = "tcp " + str;
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
		    ex.str = "tcp " + str;
		    throw ex;
		}
		const_cast<string&>(_host) = argument;
		break;
	    }

	    case 'p':
	    {
	    	const_cast<Int&>(_port) = atoi(argument.c_str());
		if(_port <= 0 || _port > 65535)
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "tcp " + str;
		    throw ex;
		}
		break;
	    }

	    case 't':
	    {
	        const_cast<Int&>(_timeout) = atoi(argument.c_str());
		if(_timeout == 0)
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "tcp " + str;
		    throw ex;
		}
		break;
	    }

	    case 'z':
	    {
	        // Ignore compression flag.
	        break;
	    }

	    default:
	    {
		EndpointParseException ex(__FILE__, __LINE__);
		ex.str = "tcp " + str;
		throw ex;
	    }
	}
    }
}

IceInternal::TcpEndpoint::TcpEndpoint(BasicStream* s) :
    _instance(s->instance()),
    _port(0),
    _timeout(-1),
    _publish(true)
{
    bool dummy;

    s->startReadEncaps();
    s->read(const_cast<string&>(_host), false);
    s->read(const_cast<Int&>(_port));
    s->read(const_cast<Int&>(_timeout));
    s->read(const_cast<bool&>(dummy));
    s->endReadEncaps();
}

void
IceInternal::TcpEndpoint::streamWrite(BasicStream* s) const
{
    s->write(TcpEndpointType);
    s->startWriteEncaps();
    s->write(_host, false);
    s->write(_port);
    s->write(_timeout);
    s->write(false);
    s->endWriteEncaps();
}

string
IceInternal::TcpEndpoint::toString() const
{
    string s;
    s += "tcp -h ";
    s += _host;

    s += Ice::printfToString(" -p %d", _port);

    if(_timeout != -1)
    {
        s += Ice::printfToString(" -t %d", _timeout);
    }
    return s;
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
	return new TcpEndpoint(_instance, _host, _port, timeout, _publish);
    }
}

bool
IceInternal::TcpEndpoint::secure() const
{
    return false;
}

bool
IceInternal::TcpEndpoint::datagram() const
{
    return false;
}

bool
IceInternal::TcpEndpoint::unknown() const
{
    return false;
}

ConnectorPtr
IceInternal::TcpEndpoint::connector() const
{
    return new Connector(_instance, _host, _port);
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
	try
	{
	    getAddress(_host, _port, laddr);
	    getAddress(p->_host, p->_port, raddr);
	}
	catch(const DNSException&)
	{
	    return false;
	}

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
	try
	{
	    getAddress(_host, _port, laddr);
	}
	catch(const DNSException&)
	{
	}

	struct sockaddr_in raddr;
	try
	{
	    getAddress(p->_host, p->_port, raddr);
	}
	catch(const DNSException&)
	{
	}

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

vector<EndpointPtr>
IceInternal::TcpEndpoint::expand(bool server) const
{
    if(_host.empty())
    {
        const_cast<string&>(_host) = _instance->defaultsAndOverrides()->defaultHost;
        if(_host.empty())
        {
	    if(server)
	    {
                const_cast<string&>(_host) = "0.0.0.0";
	    }
	    else
	    {
                const_cast<string&>(_host) = "127.0.0.1";
	    }
        }
    }
    else if(_host == "*")
    {
        const_cast<string&>(_host) = "0.0.0.0";
    }

    vector<EndpointPtr> endps;
    if(_host == "0.0.0.0")
    {
        vector<string> hosts = getLocalHosts();
        for(unsigned int i = 0; i < hosts.size(); ++i)
        {
            endps.push_back(new TcpEndpoint(_instance, hosts[i], _port, _timeout, 
                                            hosts.size() == 1 || hosts[i] != "127.0.0.1"));
        }
    }
    else
    {
        endps.push_back(const_cast<TcpEndpoint*>(this));
    }
    return endps;
}

#ifndef ICEE_PURE_CLIENT

AcceptorPtr
IceInternal::TcpEndpoint::acceptor(EndpointPtr& endp) const
{
    Acceptor* p = new Acceptor(_instance, _host, _port);
    endp = new TcpEndpoint(_instance, _host, p->effectivePort(), _timeout, _publish);
    return p;
}

bool
IceInternal::TcpEndpoint::publish() const
{
    return _publish;
}

#endif
