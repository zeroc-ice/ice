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
#include <Ice/SUdpTransceiver.h>
#include <Ice/BasicStream.h>
#include <Ice/Exception.h>
#include <Ice/Instance.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(Endpoint* p) { p->__incRef(); }
void IceInternal::decRef(Endpoint* p) { p->__decRef(); }

EndpointPtr
IceInternal::Endpoint::endpointFromString(const InstancePtr& instance, const string& str)
{
    static const string delim = " \t\n\r";

    string::size_type beg = str.find_first_not_of(delim);
    if (beg == string::npos)
    {
	throw EndpointParseException(__FILE__, __LINE__);
    }

    string::size_type end = str.find_first_of(delim, beg);
    if (end == string::npos)
    {
	end = str.length();
    }

    string protocol = str.substr(beg, end - beg);

    if (protocol == "default")
    {
	protocol = instance->defaultProtocol();
    }

    if (protocol == "tcp")
    {
	return new TcpEndpoint(instance, str.substr(end));
    }

    if (protocol == "ssl")
    {
	return new SslEndpoint(instance, str.substr(end));
    }

    if (protocol == "udp")
    {
	return new UdpEndpoint(instance, str.substr(end));
    }

    if (protocol == "sudp")
    {
	return new SUdpEndpoint(instance, str.substr(end));
    }

    throw EndpointParseException(__FILE__, __LINE__);
}

void
IceInternal::Endpoint::streamRead(BasicStream* s, EndpointPtr& v)
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

	case SUdpEndpointType:
	{
	    v = new SUdpEndpoint(s);
	    break;
	}

	default:
	{
	    v = new UnknownEndpoint(type, s);
	    break;
	}
    }
}

IceInternal::UnknownEndpoint::UnknownEndpoint(Short type, BasicStream* s) :
    _instance(s->instance()),
    _type(type)
{
    s->startReadEncaps();
    Int sz = s->getReadEncapsSize();
    s->readBlob(const_cast<vector<Byte>&>(_rawBytes), sz);
    s->endReadEncaps();
}

void
IceInternal::UnknownEndpoint::streamWrite(BasicStream* s) const
{
    s->write(_type);
    s->startWriteEncaps();
    s->writeBlob(_rawBytes);
    s->endWriteEncaps();
}

string
IceInternal::UnknownEndpoint::toString() const
{
    return string();
}

Short
IceInternal::UnknownEndpoint::type() const
{
    return _type;
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

bool
IceInternal::UnknownEndpoint::unknown() const
{
    return true;
}

TransceiverPtr
IceInternal::UnknownEndpoint::clientTransceiver() const
{
    return 0;
}

TransceiverPtr
IceInternal::UnknownEndpoint::serverTransceiver(EndpointPtr& endp) const
{
    endp = const_cast<UnknownEndpoint*>(this);
    return 0;
}

ConnectorPtr
IceInternal::UnknownEndpoint::connector() const
{
    return 0;
}

AcceptorPtr
IceInternal::UnknownEndpoint::acceptor(EndpointPtr& endp) const
{
    endp = const_cast<UnknownEndpoint*>(this);
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
    const UnknownEndpoint* p = dynamic_cast<const UnknownEndpoint*>(&r);
    if (!p)
    {
	return false;
    }

    if (this == p)
    {
	return true;
    }

    if (_type != p->_type)
    {
        return false;
    }

    if (_rawBytes != p->_rawBytes)
    {
	return false;
    }

    return true;
}

bool
IceInternal::UnknownEndpoint::operator!=(const Endpoint& r) const
{
    return !operator==(r);
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

    if (_type < p->_type)
    {
	return true;
    }
    else if (p->_type < _type)
    {
	return false;
    }

    if (_rawBytes < p->_rawBytes)
    {
	return true;
    }
    else if (p->_rawBytes < _rawBytes)
    {
	return false;
    }

    return false;
}

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
	    end = str.find_first_of(delim, beg);
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

	    case 't':
	    {
		if (argument.empty())
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

    if (_host.empty())
    {
	const_cast<string&>(_host) = _instance->defaultHost();
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
    if (_timeout != -1)
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
    if (timeout == _timeout)
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
    TcpAcceptor* p = new TcpAcceptor(_instance, _port);
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
    if (!tcpAcceptor)
    {
	return false;
    }
    return tcpAcceptor->equivalent(_host, _port);
}

bool
IceInternal::TcpEndpoint::operator==(const Endpoint& r) const
{
    const TcpEndpoint* p = dynamic_cast<const TcpEndpoint*>(&r);
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

    if (_timeout != p->_timeout)
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
	getAddress(_host.c_str(), _port, laddr);
	getAddress(p->_host.c_str(), p->_port, raddr);
	if (memcmp(&laddr, &raddr, sizeof(struct sockaddr_in)) != 0)
	{
	    return false;
	}
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

	if (dynamic_cast<const SUdpEndpoint*>(&r))
	{
	    return false; // tcp is not "less than" sudp
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

    if (_port < p->_port)
    {
	return true;
    }
    else if (p->_port < _port)
    {
	return false;
    }

    if (_timeout < p->_timeout)
    {
	return true;
    }
    else if (p->_timeout < _timeout)
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
	getAddress(_host.c_str(), _port, laddr);
	getAddress(p->_host.c_str(), p->_port, raddr);
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

IceInternal::SslEndpoint::SslEndpoint(const InstancePtr& instance, const string& ho, Int po, Int ti) :
    _instance(instance),
    _host(ho),
    _port(po),
    _timeout(ti)
{
}

IceInternal::SslEndpoint::SslEndpoint(const InstancePtr& instance, const string& str) :
    _instance(instance),
    _port(0),
    _timeout(-1)
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
	    end = str.find_first_of(delim, beg);
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

	    case 't':
	    {
		if (argument.empty())
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

    if (_host.empty())
    {
	const_cast<string&>(_host) = _instance->defaultHost();
    }
}

IceInternal::SslEndpoint::SslEndpoint(BasicStream* s) :
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
IceInternal::SslEndpoint::streamWrite(BasicStream* s) const
{
    s->write(SslEndpointType);
    s->startWriteEncaps();
    s->write(_host);
    s->write(_port);
    s->write(_timeout);
    s->endWriteEncaps();
}

string
IceInternal::SslEndpoint::toString() const
{
    ostringstream s;
    s << "ssl -h " << _host << " -p " << _port;
    if (_timeout != -1)
    {
	s << " -t " << _timeout;
    }
    return s.str();
}

Short
IceInternal::SslEndpoint::type() const
{
    return SslEndpointType;
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
	return new SslEndpoint(_instance, _host, _port, timeout);
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

bool
IceInternal::SslEndpoint::unknown() const
{
    return false;
}

TransceiverPtr
IceInternal::SslEndpoint::clientTransceiver() const
{
    return 0;
}

TransceiverPtr
IceInternal::SslEndpoint::serverTransceiver(EndpointPtr& endp) const
{
    endp = const_cast<SslEndpoint*>(this);
    return 0;
}

ConnectorPtr
IceInternal::SslEndpoint::connector() const
{
    return new SslConnector(_instance, _host, _port);
}

AcceptorPtr
IceInternal::SslEndpoint::acceptor(EndpointPtr& endp) const
{
    SslAcceptor* p = new SslAcceptor(_instance, _port);
    endp = new SslEndpoint(_instance, _host, p->effectivePort(), _timeout);
    return p;
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
    const SslEndpoint* p = dynamic_cast<const SslEndpoint*>(&r);
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

    if (_timeout != p->_timeout)
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
	getAddress(_host.c_str(), _port, laddr);
	getAddress(p->_host.c_str(), p->_port, raddr);
	if (memcmp(&laddr, &raddr, sizeof(struct sockaddr_in)) != 0)
	{
	    return false;
	}
    }

    return true;
}

bool
IceInternal::SslEndpoint::operator!=(const Endpoint& r) const
{
    return !operator==(r);
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

	if (dynamic_cast<const SUdpEndpoint*>(&r))
	{
	    return false; // ssl is not "less than" sudp
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

    if (_port < p->_port)
    {
	return true;
    }
    else if (p->_port < _port)
    {
	return false;
    }

    if (_timeout < p->_timeout)
    {
	return true;
    }
    else if (p->_timeout < _timeout)
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
	getAddress(_host.c_str(), _port, laddr);
	getAddress(p->_host.c_str(), p->_port, raddr);
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
	const_cast<string&>(_host) = _instance->defaultHost();
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
    UdpTransceiver* p = new UdpTransceiver(_instance, _port, _connect);
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
	getAddress(_host.c_str(), _port, laddr);
	getAddress(p->_host.c_str(), p->_port, raddr);
	if (memcmp(&laddr, &raddr, sizeof(struct sockaddr_in)) != 0)
	{
	    return false;
	}
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
	if (dynamic_cast<const SslEndpoint*>(&r))
	{
	    return true; // udp is "less than" ssl
	}

	if (dynamic_cast<const TcpEndpoint*>(&r))
	{
	    return true; // udp is "less than" tcp
	}

	if (dynamic_cast<const SUdpEndpoint*>(&r))
	{
	    return false; // udp is not "less than" sudp
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
	getAddress(_host.c_str(), _port, laddr);
	getAddress(p->_host.c_str(), p->_port, raddr);
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

IceInternal::SUdpEndpoint::SUdpEndpoint(const InstancePtr& instance, const string& ho, Int po) :
    _instance(instance),
    _host(ho),
    _port(po),
    _connect(false)
{
}

IceInternal::SUdpEndpoint::SUdpEndpoint(const InstancePtr& instance, const string& str) :
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
	const_cast<string&>(_host) = _instance->defaultHost();
    }
}

IceInternal::SUdpEndpoint::SUdpEndpoint(BasicStream* s) :
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
IceInternal::SUdpEndpoint::streamWrite(BasicStream* s) const
{
    s->write(SUdpEndpointType);
    s->startWriteEncaps();
    s->write(_host);
    s->write(_port);
    // Not transmitted.
    //s->write(_connect);
    s->endWriteEncaps();
}

string
IceInternal::SUdpEndpoint::toString() const
{
    ostringstream s;
    s << "sudp -h " << _host << " -p " << _port;
    if (_connect)
    {
	s << " -c";
    }
    return s.str();
}

Short
IceInternal::SUdpEndpoint::type() const
{
    return SUdpEndpointType;
}

Int
IceInternal::SUdpEndpoint::timeout() const
{
    return -1;
}

EndpointPtr
IceInternal::SUdpEndpoint::timeout(Int) const
{
    return const_cast<SUdpEndpoint*>(this);
}

bool
IceInternal::SUdpEndpoint::datagram() const
{
    return true;
}

bool
IceInternal::SUdpEndpoint::secure() const
{
    return true;
}

bool
IceInternal::SUdpEndpoint::unknown() const
{
    return false;
}

TransceiverPtr
IceInternal::SUdpEndpoint::clientTransceiver() const
{
    return new SUdpTransceiver(_instance, _host, _port);
}

TransceiverPtr
IceInternal::SUdpEndpoint::serverTransceiver(EndpointPtr& endp) const
{
    SUdpTransceiver* p = new SUdpTransceiver(_instance, _port, _connect);
    endp = new SUdpEndpoint(_instance, _host, p->effectivePort());
    return p;
}

ConnectorPtr
IceInternal::SUdpEndpoint::connector() const
{
    return 0;
}

AcceptorPtr
IceInternal::SUdpEndpoint::acceptor(EndpointPtr& endp) const
{
    endp = const_cast<SUdpEndpoint*>(this);
    return 0;
}

bool
IceInternal::SUdpEndpoint::equivalent(const TransceiverPtr& transceiver) const
{
    const SUdpTransceiver* sudpTransceiver = dynamic_cast<const SUdpTransceiver*>(transceiver.get());
    if (!sudpTransceiver)
    {
	return false;
    }
    return sudpTransceiver->equivalent(_host, _port);
}

bool
IceInternal::SUdpEndpoint::equivalent(const AcceptorPtr&) const
{
    return false;
}

bool
IceInternal::SUdpEndpoint::operator==(const Endpoint& r) const
{
    const SUdpEndpoint* p = dynamic_cast<const SUdpEndpoint*>(&r);
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
	getAddress(_host.c_str(), _port, laddr);
	getAddress(p->_host.c_str(), p->_port, raddr);
	if (memcmp(&laddr, &raddr, sizeof(struct sockaddr_in)) != 0)
	{
	    return false;
	}
    }

    return true;
}

bool
IceInternal::SUdpEndpoint::operator!=(const Endpoint& r) const
{
    return !operator==(r);
}

bool
IceInternal::SUdpEndpoint::operator<(const Endpoint& r) const
{
    const SUdpEndpoint* p = dynamic_cast<const SUdpEndpoint*>(&r);
    if (!p)
    {
	if (dynamic_cast<const SslEndpoint*>(&r))
	{
	    return true; // sudp is "less than" ssl
	}

	if (dynamic_cast<const TcpEndpoint*>(&r))
	{
	    return true; // sudp is "less than" tcp
	}

	if (dynamic_cast<const UdpEndpoint*>(&r))
	{
	    return true; // sudp is "less than" udp
	}

	if (dynamic_cast<const UnknownEndpoint*>(&r))
	{
	    return false; // sudp is not "less than" unknown
	}

	assert(false);
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
	getAddress(_host.c_str(), _port, laddr);
	getAddress(p->_host.c_str(), p->_port, raddr);
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
