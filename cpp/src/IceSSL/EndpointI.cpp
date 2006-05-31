// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/EndpointI.h>
#include <IceSSL/AcceptorI.h>
#include <IceSSL/ConnectorI.h>
#include <IceSSL/TransceiverI.h>
#include <IceSSL/Instance.h>
#include <Ice/Network.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <Ice/DefaultsAndOverrides.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

IceSSL::EndpointI::EndpointI(const InstancePtr& instance, const string& ho, Int po, Int ti, const string& conId,
			     bool co, bool pub) :
    _instance(instance),
    _host(ho),
    _port(po),
    _timeout(ti),
    _connectionId(conId),
    _compress(co),
    _publish(pub)
{
}

IceSSL::EndpointI::EndpointI(const InstancePtr& instance, const string& str) :
    _instance(instance),
    _port(0),
    _timeout(-1),
    _compress(false),
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
		istringstream p(argument);
		if(!(p >> const_cast<Int&>(_port)) || !p.eof() || _port < 0 || _port > 65535)
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "ssl " + str;
		    throw ex;
		}
		break;
	    }

	    case 't':
	    {
		istringstream t(argument);
		if(!(t >> const_cast<Int&>(_timeout)) || !t.eof())
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "ssl " + str;
		    throw ex;
		}
		break;
	    }

	    case 'z':
	    {
		if(!argument.empty())
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "ssl " + str;
		    throw ex;
		}
		const_cast<bool&>(_compress) = true;
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
	const_cast<string&>(_host) = _instance->defaultHost();
	if(_host.empty())
	{
	    const_cast<string&>(_host) = "0.0.0.0";
	}
    }
    else if(_host == "*")
    {
        const_cast<string&>(_host) = "0.0.0.0";
    }
}

IceSSL::EndpointI::EndpointI(const InstancePtr& instance, IceInternal::BasicStream* s) :
    _instance(instance),
    _port(0),
    _timeout(-1),
    _compress(false),
    _publish(true)
{
    s->startReadEncaps();
    s->read(const_cast<string&>(_host), false);
    s->read(const_cast<Int&>(_port));
    s->read(const_cast<Int&>(_timeout));
    s->read(const_cast<bool&>(_compress));
    s->endReadEncaps();
}

void
IceSSL::EndpointI::streamWrite(IceInternal::BasicStream* s) const
{
    s->write(EndpointType);
    s->startWriteEncaps();
    s->write(_host, false);
    s->write(_port);
    s->write(_timeout);
    s->write(_compress);
    s->endWriteEncaps();
}

string
IceSSL::EndpointI::toString() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    ostringstream s;
    s << "ssl -h " << _host << " -p " << _port;
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

Short
IceSSL::EndpointI::type() const
{
    return EndpointType;
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
	return new EndpointI(_instance, _host, _port, timeout, _connectionId, _compress, _publish);
    }
}

IceInternal::EndpointIPtr
IceSSL::EndpointI::connectionId(const string& connectionId) const
{
    if(connectionId == _connectionId)
    {
	return const_cast<EndpointI*>(this);
    }
    else
    {
	return new EndpointI(_instance, _host, _port, _timeout, connectionId, _compress, _publish);
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
	return new EndpointI(_instance, _host, _port, _timeout, _connectionId, compress, _publish);
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

bool
IceSSL::EndpointI::unknown() const
{
    return false;
}

IceInternal::TransceiverPtr
IceSSL::EndpointI::clientTransceiver() const
{
    return 0;
}

IceInternal::TransceiverPtr
IceSSL::EndpointI::serverTransceiver(IceInternal::EndpointIPtr& endp) const
{
    endp = const_cast<EndpointI*>(this);
    return 0;
}

IceInternal::ConnectorPtr
IceSSL::EndpointI::connector() const
{
    return new ConnectorI(_instance, _host, _port);
}

IceInternal::AcceptorPtr
IceSSL::EndpointI::acceptor(IceInternal::EndpointIPtr& endp, const string& adapterName) const
{
    AcceptorI* p = new AcceptorI(_instance, adapterName, _host, _port);
    endp = new EndpointI(_instance, _host, p->effectivePort(), _timeout, _connectionId, _compress, _publish);
    return p;
}

vector<IceInternal::EndpointIPtr>
IceSSL::EndpointI::expand(bool includeLoopback) const
{
    vector<IceInternal::EndpointIPtr> endps;
    if(_host == "0.0.0.0")
    {
        vector<string> hosts = IceInternal::getLocalHosts();
	for(unsigned int i = 0; i < hosts.size(); ++i)
	{
	    if(includeLoopback || hosts.size() == 1 || hosts[i] != "127.0.0.1")
	    {
	        endps.push_back(new EndpointI(_instance, hosts[i], _port, _timeout, _connectionId, _compress,
	    				      hosts.size() == 1 || hosts[i] != "127.0.0.1"));
	    }
	}
    }
    else
    {
        endps.push_back(const_cast<EndpointI*>(this));
    }
    return endps;
}

bool
IceSSL::EndpointI::publish() const
{
    return _publish;
}

bool
IceSSL::EndpointI::equivalent(const IceInternal::TransceiverPtr&) const
{
    return false;
}

bool
IceSSL::EndpointI::equivalent(const IceInternal::AcceptorPtr& acceptor) const
{
    const AcceptorI* sslAcceptor = dynamic_cast<const AcceptorI*>(acceptor.get());
    if(!sslAcceptor)
    {
	return false;
    }
    return sslAcceptor->equivalent(_host, _port);
}

bool
IceSSL::EndpointI::operator==(const IceInternal::EndpointI& r) const
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

    if(_port != p->_port)
    {
	return false;
    }

    if(_timeout != p->_timeout)
    {
	return false;
    }

    if(_connectionId != p->_connectionId)
    {
	return false;
    }

    if(_compress != p->_compress)
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
	    IceInternal::getAddress(_host, _port, laddr);
	    IceInternal::getAddress(p->_host, p->_port, raddr);
	}
	catch(const DNSException&)
	{
	    return false;
	}

	return IceInternal::compareAddress(laddr, raddr);
    }

    return true;
}

bool
IceSSL::EndpointI::operator!=(const IceInternal::EndpointI& r) const
{
    return !operator==(r);
}

bool
IceSSL::EndpointI::operator<(const IceInternal::EndpointI& r) const
{
    const EndpointI* p = dynamic_cast<const EndpointI*>(&r);
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

    if(_connectionId < p->_connectionId)
    {
	return true;
    }
    else if(p->_connectionId < _connectionId)
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

    if(_host != p->_host)
    {
	//
	// We do the most time-consuming part of the comparison last.
	//
	struct sockaddr_in laddr;
	try
	{
	    IceInternal::getAddress(_host, _port, laddr);
	}
	catch(const DNSException&)
	{
	}

	struct sockaddr_in raddr;
	try
	{
	    IceInternal::getAddress(p->_host, p->_port, raddr);
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

IceSSL::EndpointFactoryI::EndpointFactoryI(const InstancePtr& instance)
    : _instance(instance)
{
}

IceSSL::EndpointFactoryI::~EndpointFactoryI()
{
}

Short
IceSSL::EndpointFactoryI::type() const
{
    return EndpointType;
}

string
IceSSL::EndpointFactoryI::protocol() const
{
    return "ssl";
}

IceInternal::EndpointIPtr
IceSSL::EndpointFactoryI::create(const std::string& str) const
{
    return new EndpointI(_instance, str);
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
