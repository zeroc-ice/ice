// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Network.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <Ice/ProtocolPluginFacade.h>
#include <IceSSL/SslEndpointI.h>
#include <IceSSL/SslAcceptor.h>
#include <IceSSL/SslConnector.h>
#include <IceSSL/SslTransceiver.h>
#include <IceSSL/OpenSSLPluginI.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceSSL::SslEndpointI::SslEndpointI(const OpenSSLPluginIPtr& plugin, const string& ho, Int po, Int ti,
				   const string& conId, bool co, bool pub) :
    _plugin(plugin),
    _host(ho),
    _port(po),
    _timeout(ti),
    _connectionId(conId),
    _compress(co),
    _publish(pub)
{
}

IceSSL::SslEndpointI::SslEndpointI(const OpenSSLPluginIPtr& plugin, const string& str, bool adapterEndp) :
    _plugin(plugin),
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
        const_cast<string&>(_host) = _plugin->getProtocolPluginFacade()->getDefaultHost();
        if(_host.empty())
        {
	    if(adapterEndp)
	    {
	        const_cast<string&>(_host) = "0.0.0.0";
	    }
	    else
	    {
                 const_cast<string&>(_host) = getLocalHost(true);
	    }
        }
    }
    else if(_host == "*" && adapterEndp)
    {
        const_cast<string&>(_host) = "0.0.0.0";
    }
}

IceSSL::SslEndpointI::SslEndpointI(const OpenSSLPluginIPtr& plugin, BasicStream* s) :
    _plugin(plugin),
    _port(0),
    _timeout(-1),
    _compress(false),
    _publish(true)
{
    s->startReadEncaps();
    s->read(const_cast<string&>(_host));
    s->read(const_cast<Int&>(_port));
    s->read(const_cast<Int&>(_timeout));
    s->read(const_cast<bool&>(_compress));
    s->endReadEncaps();
}

void
IceSSL::SslEndpointI::streamWrite(BasicStream* s) const
{
    s->write(SslEndpointType);
    s->startWriteEncaps();
    s->write(_host);
    s->write(_port);
    s->write(_timeout);
    s->write(_compress);
    s->endWriteEncaps();
}

string
IceSSL::SslEndpointI::toString() const
{
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
IceSSL::SslEndpointI::type() const
{
    return SslEndpointType;
}

Int
IceSSL::SslEndpointI::timeout() const
{
    return _timeout;
}

EndpointIPtr
IceSSL::SslEndpointI::timeout(Int timeout) const
{
    if(timeout == _timeout)
    {
	return const_cast<SslEndpointI*>(this);
    }
    else
    {
	return new SslEndpointI(_plugin, _host, _port, timeout, _connectionId, _compress, _publish);
    }
}

EndpointIPtr
IceSSL::SslEndpointI::connectionId(const string& connectionId) const
{
    if(connectionId == _connectionId)
    {
	return const_cast<SslEndpointI*>(this);
    }
    else
    {
	return new SslEndpointI(_plugin, _host, _port, _timeout, connectionId, _compress, _publish);
    }
}

bool
IceSSL::SslEndpointI::compress() const
{
    return _compress;
}

EndpointIPtr
IceSSL::SslEndpointI::compress(bool compress) const
{
    if(compress == _compress)
    {
	return const_cast<SslEndpointI*>(this);
    }
    else
    {
	return new SslEndpointI(_plugin, _host, _port, _timeout, _connectionId, compress, _publish);
    }
}

bool
IceSSL::SslEndpointI::datagram() const
{
    return false;
}

bool
IceSSL::SslEndpointI::secure() const
{
    return true;
}

bool
IceSSL::SslEndpointI::unknown() const
{
    return false;
}

TransceiverPtr
IceSSL::SslEndpointI::clientTransceiver() const
{
    return 0;
}

TransceiverPtr
IceSSL::SslEndpointI::serverTransceiver(EndpointIPtr& endp) const
{
    endp = const_cast<SslEndpointI*>(this);
    return 0;
}

ConnectorPtr
IceSSL::SslEndpointI::connector() const
{
    return new SslConnector(_plugin, _host, _port);
}

AcceptorPtr
IceSSL::SslEndpointI::acceptor(EndpointIPtr& endp) const
{
    SslAcceptor* p = new SslAcceptor(_plugin, _host, _port);
    endp = new SslEndpointI(_plugin, _host, p->effectivePort(), _timeout, _connectionId, _compress, _publish);
    return p;
}

vector<EndpointIPtr>
IceSSL::SslEndpointI::expand() const
{
    vector<EndpointIPtr> endps;
    if(_host == "0.0.0.0")
    {
        vector<string> hosts = getLocalHosts();
        for(unsigned int i = 0; i < hosts.size(); ++i)
        {
            endps.push_back(new SslEndpointI(_plugin, hosts[i], _port, _timeout, _connectionId, _compress,
					     hosts.size() == 1 || hosts[i] != "127.0.0.1"));
        }
    }
    else
    {
        endps.push_back(const_cast<SslEndpointI*>(this));
    }
    return endps;

}

bool
IceSSL::SslEndpointI::publish() const
{
    return _publish;
}

bool
IceSSL::SslEndpointI::equivalent(const TransceiverPtr&) const
{
    return false;
}

bool
IceSSL::SslEndpointI::equivalent(const AcceptorPtr& acceptor) const
{
    const SslAcceptor* sslAcceptor = dynamic_cast<const SslAcceptor*>(acceptor.get());
    if(!sslAcceptor)
    {
	return false;
    }
    return sslAcceptor->equivalent(_host, _port);
}

bool
IceSSL::SslEndpointI::operator==(const EndpointI& r) const
{
    const SslEndpointI* p = dynamic_cast<const SslEndpointI*>(&r);
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
IceSSL::SslEndpointI::operator!=(const EndpointI& r) const
{
    return !operator==(r);
}

bool
IceSSL::SslEndpointI::operator<(const EndpointI& r) const
{
    const SslEndpointI* p = dynamic_cast<const SslEndpointI*>(&r);
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

EndpointIPtr
IceSSL::SslEndpointFactory::create(const std::string& str, bool adapterEndp) const
{
    return new SslEndpointI(_plugin, str, adapterEndp);
}

EndpointIPtr
IceSSL::SslEndpointFactory::read(BasicStream* s) const
{
    return new SslEndpointI(_plugin, s);
}

void
IceSSL::SslEndpointFactory::destroy()
{
    _plugin = 0;
}
