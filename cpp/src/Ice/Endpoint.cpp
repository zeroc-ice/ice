// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Endpoint.h>
#include <Ice/Network.h>
#include <Ice/TcpAcceptor.h>
#include <Ice/TcpConnector.h>
#include <Ice/TcpTransceiver.h>
#include <Ice/SslAcceptor.h>
#include <Ice/SslConnector.h>
#include <Ice/SslTransceiver.h>
#include <Ice/UdpTransceiver.h>
#include <Ice/Stream.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(Endpoint* p) { p->__incRef(); }
void IceInternal::decRef(Endpoint* p) { p->__decRef(); }

EndpointPtr
IceInternal::Endpoint::endpointFromString(const string& str)
{
    const string delim = " \t\n\r";

    string s(str);
    transform(s.begin(), s.end(), s.begin(), tolower);

    string::size_type beg = s.find_first_not_of(delim);
    if (beg == string::npos)
    {
	throw EndpointParseException(__FILE__, __LINE__);
    }

    string::size_type end = s.find_first_of(delim, beg);
    if (end == string::npos)
    {
	end = s.length();
    }

    if (s.compare(beg, end - beg, "tcp") == 0)
    {
	return new TcpEndpoint(s.substr(end));
    }

    if (s.compare(beg, end - beg, "ssl") == 0)
    {
	return new SslEndpoint(s.substr(end));
    }

    if (s.compare(beg, end - beg, "udp") == 0)
    {
	return new UdpEndpoint(s.substr(end));
    }

    throw EndpointParseException(__FILE__, __LINE__);
}

void
IceInternal::Endpoint::streamRead(Stream* s, EndpointPtr& v)
{
    Short type;
    s->read(type);
    
    switch (type)
    {
	case TcpEndpointType:
	{
	    v = new TcpEndpoint(s);
	    break;
	}
		
	case SslEndpointType:
	{
	    v = new SslEndpoint(s);
	    break;
	}
	
	case UdpEndpointType:
	{
	    v = new UdpEndpoint(s);
	    break;
	}

	default:
	{
	    v = new UnknownEndpoint(s);
	    break;
	}
    }
}

bool
IceInternal::Endpoint::regular() const
{
    return !secure() && !datagram();
}

IceInternal::UnknownEndpoint::UnknownEndpoint(Stream* s)
{
    s->read(const_cast<vector<Byte>&>(_rawBytes));
}

void
IceInternal::UnknownEndpoint::streamWrite(Stream* s) const
{
    s->write(UnknownEndpointType);
    s->write(_rawBytes);
}

Short
IceInternal::UnknownEndpoint::type() const
{
    return UnknownEndpointType;
}

bool
IceInternal::UnknownEndpoint::oneway() const
{
    return false;
}

Int
IceInternal::UnknownEndpoint::timeout() const
{
    return -1;
}

EndpointPtr
IceInternal::UnknownEndpoint::timeout(Int) const
{
    return const_cast<UnknownEndpoint*>(this);
}

bool
IceInternal::UnknownEndpoint::datagram() const
{
    return false;
}

bool
IceInternal::UnknownEndpoint::secure() const
{
    return false;
}

TransceiverPtr
IceInternal::UnknownEndpoint::clientTransceiver(const InstancePtr&) const
{
    return 0;
}

TransceiverPtr
IceInternal::UnknownEndpoint::serverTransceiver(const InstancePtr&) const
{
    return 0;
}

ConnectorPtr
IceInternal::UnknownEndpoint::connector(const InstancePtr& instance) const
{
    return 0;
}

AcceptorPtr
IceInternal::UnknownEndpoint::acceptor(const InstancePtr& instance) const
{
    return 0;
}

bool
IceInternal::UnknownEndpoint::equivalent(const TransceiverPtr&) const
{
    return false;
}

bool
IceInternal::UnknownEndpoint::equivalent(const AcceptorPtr&) const
{
    return false;
}

bool
IceInternal::UnknownEndpoint::operator==(const Endpoint& r) const
{
    return !operator!=(r);
}

bool
IceInternal::UnknownEndpoint::operator!=(const Endpoint& r) const
{
    const UnknownEndpoint* p = dynamic_cast<const UnknownEndpoint*>(&r);
    if (!p)
    {
	return true;
    }

    if (this == p)
    {
	return false;
    }

    if (_rawBytes != p->_rawBytes)
    {
	return true;
    }

    return false;
}

bool
IceInternal::UnknownEndpoint::operator<(const Endpoint& r) const
{
    const UnknownEndpoint* p = dynamic_cast<const UnknownEndpoint*>(&r);
    if (!p)
    {
	return true; // unknown is "less than" every other protocol
    }

    if (this == p)
    {
	return false;
    }

    if (_rawBytes < p->_rawBytes)
    {
	return true;
    }
    else if (_rawBytes != p->_rawBytes)
    {
	return false;
    }

    return false;
}

IceInternal::TcpEndpoint::TcpEndpoint(const string& ho, Int po, Int ti) :
    _host(ho),
    _port(po),
    _timeout(ti)
{
}

IceInternal::TcpEndpoint::TcpEndpoint(const string& str) :
    _port(10000),
    _timeout(-1)
{
    const string delim = " \t\n\r";

    string s(str);
    transform(s.begin(), s.end(), s.begin(), tolower);

    string::size_type beg;
    string::size_type end = 0;

    while (true)
    {
	beg = s.find_first_not_of(delim, end);
	if (beg == string::npos)
	{
	    break;
	}
	
	end = s.find_first_of(delim, beg);
	if (end == string::npos)
	{
	    end = s.length();
	}

	string option = s.substr(beg, end - beg);
	if (option.length() != 2 || option[0] != '-')
	{
	    throw EndpointParseException(__FILE__, __LINE__);
	}

	beg = s.find_first_not_of(delim, end);
	if (beg == string::npos)
	{
	    throw EndpointParseException(__FILE__, __LINE__);
	}
	
	end = s.find_first_of(delim, beg);
	if (end == string::npos)
	{
	    end = s.length();
	}
	
	string argument = s.substr(beg, end - beg);

	switch (option[1])
	{
	    case 'h':
	    {
		const_cast<string&>(_host) = argument;
		break;
	    }

	    case 'p':
	    {
		const_cast<Int&>(_port) = atoi(argument.c_str());
		break;
	    }

	    case 't':
	    {
		const_cast<Int&>(_timeout) = atoi(argument.c_str());
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
	// TODO: Whether numeric or not should be configurable
	const_cast<string&>(_host) = getLocalHost(true);
    }
}

IceInternal::TcpEndpoint::TcpEndpoint(Stream* s) :
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
IceInternal::TcpEndpoint::streamWrite(Stream* s) const
{
    s->write(TcpEndpointType);
    s->startWriteEncaps();
    s->write(_host);
    s->write(_port);
    s->write(_timeout);
    s->endWriteEncaps();
}

Short
IceInternal::TcpEndpoint::type() const
{
    return TcpEndpointType;
}

bool
IceInternal::TcpEndpoint::oneway() const
{
    return false;
}

Int
IceInternal::TcpEndpoint::timeout() const
{
    return _timeout;
}

EndpointPtr
IceInternal::TcpEndpoint::timeout(Int timeout) const
{
    if (timeout == _timeout)
    {
	return const_cast<TcpEndpoint*>(this);
    }
    else
    {
	return new TcpEndpoint(_host, _port, timeout);
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

TransceiverPtr
IceInternal::TcpEndpoint::clientTransceiver(const InstancePtr&) const
{
    return 0;
}

TransceiverPtr
IceInternal::TcpEndpoint::serverTransceiver(const InstancePtr&) const
{
    return 0;
}

ConnectorPtr
IceInternal::TcpEndpoint::connector(const InstancePtr& instance) const
{
    return new TcpConnector(instance, _host, _port);
}

AcceptorPtr
IceInternal::TcpEndpoint::acceptor(const InstancePtr& instance) const
{
    return new TcpAcceptor(instance, _port);
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
    if (!tcpAcceptor)
    {
	return false;
    }
    return tcpAcceptor->equivalent(_host, _port);
}

bool
IceInternal::TcpEndpoint::operator==(const Endpoint& r) const
{
    return !operator!=(r);
}

bool
IceInternal::TcpEndpoint::operator!=(const Endpoint& r) const
{
    const TcpEndpoint* p = dynamic_cast<const TcpEndpoint*>(&r);
    if (!p)
    {
	return true;
    }

    if (this == p)
    {
	return false;
    }

    if (_port != p->_port)
    {
	return true;
    }
    
    struct sockaddr_in laddr;
    struct sockaddr_in raddr;
    getAddress(_host.c_str(), _port, laddr);
    getAddress(p->_host.c_str(), p->_port, raddr);
    
    if (memcmp(&laddr, &raddr, sizeof(struct sockaddr_in)) != 0)
    {
	return true;
    }

    if (_timeout != p->_timeout)
    {
	return true;
    }

    return false;
}

bool
IceInternal::TcpEndpoint::operator<(const Endpoint& r) const
{
    const TcpEndpoint* p = dynamic_cast<const TcpEndpoint*>(&r);
    if (!p)
    {
	if (dynamic_cast<const SslEndpoint*>(&r))
	{
	    return false; // tcp is not "less than" ssl
	}

	if (dynamic_cast<const UdpEndpoint*>(&r))
	{
	    return false; // tcp is not "less than" udp
	}

	if (dynamic_cast<const UnknownEndpoint*>(&r))
	{
	    return false; // tcp is not "less than" unknown
	}

	assert(false);
    }

    if (this == p)
    {
	return false;
    }

    struct sockaddr_in laddr;
    struct sockaddr_in raddr;
    getAddress(_host.c_str(), _port, laddr);
    getAddress(p->_host.c_str(), p->_port, raddr);

    if (laddr.sin_addr.s_addr < raddr.sin_addr.s_addr)
    {
	return true;
    }
    else if (laddr.sin_addr.s_addr != raddr.sin_addr.s_addr)
    {
	return false;
    }

    if (_port < p->_port)
    {
	return true;
    }
    else if (_port != p->_port)
    {
	return false;
    }

    if (_timeout < p->_timeout)
    {
	return true;
    }
    else if (_timeout != p->_timeout)
    {
	return false;
    }

    return false;
}

IceInternal::SslEndpoint::SslEndpoint(const string& ho, Int po, Int ti) :
    _host(ho),
    _port(po),
    _timeout(ti)
{
}

IceInternal::SslEndpoint::SslEndpoint(const string& str) :
    _port(10000),
    _timeout(-1)
{
    const string delim = " \t\n\r";

    string s(str);
    transform(s.begin(), s.end(), s.begin(), tolower);

    string::size_type beg;
    string::size_type end = 0;

    while (true)
    {
	beg = s.find_first_not_of(delim, end);
	if (beg == string::npos)
	{
	    break;
	}
	
	end = s.find_first_of(delim, beg);
	if (end == string::npos)
	{
	    end = s.length();
	}

	string option = s.substr(beg, end - beg);
	if (option.length() != 2 || option[0] != '-')
	{
	    throw EndpointParseException(__FILE__, __LINE__);
	}

	beg = s.find_first_not_of(delim, end);
	if (beg == string::npos)
	{
	    throw EndpointParseException(__FILE__, __LINE__);
	}
	
	end = s.find_first_of(delim, beg);
	if (end == string::npos)
	{
	    end = s.length();
	}
	
	string argument = s.substr(beg, end - beg);

	switch (option[1])
	{
	    case 'h':
	    {
		const_cast<string&>(_host) = argument;
		break;
	    }

	    case 'p':
	    {
		const_cast<Int&>(_port) = atoi(argument.c_str());
		break;
	    }

	    case 't':
	    {
		const_cast<Int&>(_timeout) = atoi(argument.c_str());
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
	// TODO: Whether numeric or not should be configurable
	const_cast<string&>(_host) = getLocalHost(true);
    }
}

IceInternal::SslEndpoint::SslEndpoint(Stream* s) :
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
IceInternal::SslEndpoint::streamWrite(Stream* s) const
{
    s->write(SslEndpointType);
    s->startWriteEncaps();
    s->write(_host);
    s->write(_port);
    s->write(_timeout);
    s->endWriteEncaps();
}

Short
IceInternal::SslEndpoint::type() const
{
    return SslEndpointType;
}

bool
IceInternal::SslEndpoint::oneway() const
{
    return false;
}

Int
IceInternal::SslEndpoint::timeout() const
{
    return _timeout;
}

EndpointPtr
IceInternal::SslEndpoint::timeout(Int timeout) const
{
    if (timeout == _timeout)
    {
	return const_cast<SslEndpoint*>(this);
    }
    else
    {
	return new SslEndpoint(_host, _port, timeout);
    }
}

bool
IceInternal::SslEndpoint::datagram() const
{
    return false;
}

bool
IceInternal::SslEndpoint::secure() const
{
    return true;
}

TransceiverPtr
IceInternal::SslEndpoint::clientTransceiver(const InstancePtr&) const
{
    return 0;
}

TransceiverPtr
IceInternal::SslEndpoint::serverTransceiver(const InstancePtr&) const
{
    return 0;
}

ConnectorPtr
IceInternal::SslEndpoint::connector(const InstancePtr& instance) const
{
    return new SslConnector(instance, _host, _port);
}

AcceptorPtr
IceInternal::SslEndpoint::acceptor(const InstancePtr& instance) const
{
    return new SslAcceptor(instance, _port);
}

bool
IceInternal::SslEndpoint::equivalent(const TransceiverPtr&) const
{
    return false;
}

bool
IceInternal::SslEndpoint::equivalent(const AcceptorPtr& acceptor) const
{
    const SslAcceptor* sslAcceptor = dynamic_cast<const SslAcceptor*>(acceptor.get());
    if (!sslAcceptor)
    {
	return false;
    }
    return sslAcceptor->equivalent(_host, _port);
}

bool
IceInternal::SslEndpoint::operator==(const Endpoint& r) const
{
    return !operator!=(r);
}

bool
IceInternal::SslEndpoint::operator!=(const Endpoint& r) const
{
    const SslEndpoint* p = dynamic_cast<const SslEndpoint*>(&r);
    if (!p)
    {
	return true;
    }

    if (this == p)
    {
	return false;
    }

    if (_port != p->_port)
    {
	return true;
    }
    
    struct sockaddr_in laddr;
    struct sockaddr_in raddr;
    getAddress(_host.c_str(), _port, laddr);
    getAddress(p->_host.c_str(), p->_port, raddr);
    
    if (memcmp(&laddr, &raddr, sizeof(struct sockaddr_in)) != 0)
    {
	return true;
    }

    if (_timeout != p->_timeout)
    {
	return true;
    }

    return false;
}

bool
IceInternal::SslEndpoint::operator<(const Endpoint& r) const
{
    const SslEndpoint* p = dynamic_cast<const SslEndpoint*>(&r);
    if (!p)
    {
	if (dynamic_cast<const TcpEndpoint*>(&r))
	{
	    return true; // ssl is "less than" tcp
	}

	if (dynamic_cast<const UdpEndpoint*>(&r))
	{
	    return false; // ssl is not "less than" udp
	}

	if (dynamic_cast<const UnknownEndpoint*>(&r))
	{
	    return false; // ssl is not "less than" unknown
	}

	assert(false);
    }

    if (this == p)
    {
	return false;
    }

    struct sockaddr_in laddr;
    struct sockaddr_in raddr;
    getAddress(_host.c_str(), _port, laddr);
    getAddress(p->_host.c_str(), p->_port, raddr);

    if (laddr.sin_addr.s_addr < raddr.sin_addr.s_addr)
    {
	return true;
    }
    else if (laddr.sin_addr.s_addr != raddr.sin_addr.s_addr)
    {
	return false;
    }

    if (_port < p->_port)
    {
	return true;
    }
    else if (_port != p->_port)
    {
	return false;
    }

    if (_timeout < p->_timeout)
    {
	return true;
    }
    else if (_timeout != p->_timeout)
    {
	return false;
    }

    return false;
}

IceInternal::UdpEndpoint::UdpEndpoint(const string& ho, Int po) :
    _host(ho),
    _port(po)
{
}

IceInternal::UdpEndpoint::UdpEndpoint(const string& str) :
    _port(10000)
{
    const string delim = " \t\n\r";

    string s(str);
    transform(s.begin(), s.end(), s.begin(), tolower);

    string::size_type beg;
    string::size_type end = 0;

    while (true)
    {
	beg = s.find_first_not_of(delim, end);
	if (beg == string::npos)
	{
	    break;
	}
	
	end = s.find_first_of(delim, beg);
	if (end == string::npos)
	{
	    end = s.length();
	}

	string option = s.substr(beg, end - beg);
	if (option.length() != 2 || option[0] != '-')
	{
	    throw EndpointParseException(__FILE__, __LINE__);
	}

	beg = s.find_first_not_of(delim, end);
	if (beg == string::npos)
	{
	    throw EndpointParseException(__FILE__, __LINE__);
	}
	
	end = s.find_first_of(delim, beg);
	if (end == string::npos)
	{
	    end = s.length();
	}
	
	string argument = s.substr(beg, end - beg);

	switch (option[1])
	{
	    case 'h':
	    {
		const_cast<string&>(_host) = argument;
		break;
	    }

	    case 'p':
	    {
		const_cast<Int&>(_port) = atoi(argument.c_str());
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
	// TODO: Whether numeric or not should be configurable
	const_cast<string&>(_host) = getLocalHost(true);
    }
}

IceInternal::UdpEndpoint::UdpEndpoint(Stream* s) :
    _port(0)
{
    s->startReadEncaps();
    s->read(const_cast<string&>(_host));
    s->read(const_cast<Int&>(_port));
    s->endReadEncaps();
}

void
IceInternal::UdpEndpoint::streamWrite(Stream* s) const
{
    s->write(UdpEndpointType);
    s->startWriteEncaps();
    s->write(_host);
    s->write(_port);
    s->endWriteEncaps();
}

Short
IceInternal::UdpEndpoint::type() const
{
    return UdpEndpointType;
}

bool
IceInternal::UdpEndpoint::oneway() const
{
    return true;
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

TransceiverPtr
IceInternal::UdpEndpoint::clientTransceiver(const InstancePtr& instance) const
{
    return new UdpTransceiver(instance, _host, _port);
}

TransceiverPtr
IceInternal::UdpEndpoint::serverTransceiver(const InstancePtr& instance) const
{
    return new UdpTransceiver(instance, _port);
}

ConnectorPtr
IceInternal::UdpEndpoint::connector(const InstancePtr&) const
{
    return 0;
}

AcceptorPtr
IceInternal::UdpEndpoint::acceptor(const InstancePtr&) const
{
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
    return !operator!=(r);
}

bool
IceInternal::UdpEndpoint::operator!=(const Endpoint& r) const
{
    const UdpEndpoint* p = dynamic_cast<const UdpEndpoint*>(&r);
    if (!p)
    {
	return true;
    }

    if (this == p)
    {
	return false;
    }

    if (_port != p->_port)
    {
	return true;
    }
    
    struct sockaddr_in laddr;
    struct sockaddr_in raddr;
    getAddress(_host.c_str(), _port, laddr);
    getAddress(p->_host.c_str(), p->_port, raddr);
    
    if (memcmp(&laddr, &raddr, sizeof(struct sockaddr_in)) != 0)
    {
	return true;
    }

    return false;
}

bool
IceInternal::UdpEndpoint::operator<(const Endpoint& r) const
{
    const UdpEndpoint* p = dynamic_cast<const UdpEndpoint*>(&r);
    if (!p)
    {
	if (dynamic_cast<const SslEndpoint*>(&r))
	{
	    return true; // udp is "less than" ssl
	}

	if (dynamic_cast<const TcpEndpoint*>(&r))
	{
	    return true; // udp is "less than" tcp
	}

	if (dynamic_cast<const UnknownEndpoint*>(&r))
	{
	    return false; // udp is not "less than" unknown
	}

	assert(false);
    }

    if (this == p)
    {
	return false;
    }

    struct sockaddr_in laddr;
    struct sockaddr_in raddr;
    getAddress(_host.c_str(), _port, laddr);
    getAddress(p->_host.c_str(), p->_port, raddr);

    if (laddr.sin_addr.s_addr < raddr.sin_addr.s_addr)
    {
	return true;
    }
    else if (laddr.sin_addr.s_addr != raddr.sin_addr.s_addr)
    {
	return false;
    }

    if (_port < p->_port)
    {
	return true;
    }
    else if (_port != p->_port)
    {
	return false;
    }

    return false;
}
