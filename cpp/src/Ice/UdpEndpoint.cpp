// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/UdpEndpoint.h>
#include <Ice/Network.h>
#include <Ice/UdpTransceiver.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Protocol.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::UdpEndpoint::UdpEndpoint(const InstancePtr& instance, const string& ho, Int po, bool co) :
    _instance(instance),
    _host(ho),
    _port(po),
    _protocolMajor(protocolMajor),
    _protocolMinor(protocolMinor),
    _encodingMajor(encodingMajor),
    _encodingMinor(encodingMinor),
    _connect(false),
    _compress(co)
{
}

IceInternal::UdpEndpoint::UdpEndpoint(const InstancePtr& instance, const string& str) :
    _instance(instance),
    _port(0),
    _protocolMajor(protocolMajor),
    _protocolMinor(protocolMinor),
    _encodingMajor(encodingMajor),
    _encodingMinor(encodingMinor),
    _connect(false),
    _compress(false)
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
	    ex.str = "udp " + str;
	    throw ex;
	}

	string argument;
	string::size_type argumentBeg = str.find_first_not_of(delim, end);
	if(argumentBeg != string::npos && str[argumentBeg] != '-')
	{
	    beg = argumentBeg;
	    end = str.find_first_of(delim + ":", beg);
	    if(end == string::npos)
	    {
		end = str.length();
	    }
	    argument = str.substr(beg, end - beg);
	}

	switch(option[1])
	{
	    case 'v':
	    {
		if(argument.empty())
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "udp " + str;
		    throw ex;
		}

		string::size_type pos = argument.find('.');
		if(pos == string::npos)
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "udp " + str;
		    throw ex;
		}
		string majorStr = argument.substr(0, pos);
		string minorStr = argument.substr(pos + 1, string::npos);

		istringstream majStr(majorStr);
		Int majVersion;
		if(!(majStr >> majVersion) || !majStr.eof())
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "udp " + str;
		    throw ex;
		}

		istringstream minStr(minorStr);
		Int minVersion;
		if(!(minStr >> minVersion) || !minStr.eof())
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "udp " + str;
		    throw ex;
		}

		if(majVersion < 1 || majVersion > 255 || minVersion < 0 || minVersion > 255)
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "udp " + str;
		    throw ex;
		}

		if(majVersion != protocolMajor)
		{
		    UnsupportedProtocolException ex(__FILE__, __LINE__);
		    ex.badMajor = majVersion;
		    ex.badMinor = minVersion;
		    ex.major = static_cast<unsigned char>(protocolMajor);
		    ex.minor = static_cast<unsigned char>(protocolMinor);
		    throw ex;
		}

		const_cast<Byte&>(_protocolMajor) = majVersion;
		const_cast<Byte&>(_protocolMinor) = minVersion;

		break;
	    }

	    case 'e':
	    {
		if(argument.empty())
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "udp " + str;
		    throw ex;
		}
		string::size_type pos = argument.find('.');
		string majorStr = argument.substr(0, pos);
		string minorStr = argument.substr(pos + 1, string::npos);

		istringstream majStr(majorStr);
		Int majVersion;
		if(!(majStr >> majVersion) || !majStr.eof())
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "udp " + str;
		    throw ex;
		}

		istringstream minStr(minorStr);
		Int minVersion;
		if(!(minStr >> minVersion) || !minStr.eof())
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "udp " + str;
		    throw ex;
		}

		if(majVersion < 1 || majVersion > 255 || minVersion < 0 || minVersion > 255)
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "udp " + str;
		    throw ex;
		}

		if(majVersion != static_cast<unsigned char>(encodingMajor))
		{
		    UnsupportedEncodingException ex(__FILE__, __LINE__);
		    ex.badMajor = majVersion;
		    ex.badMinor = minVersion;
		    ex.major = static_cast<unsigned char>(encodingMajor);
		    ex.minor = static_cast<unsigned char>(encodingMinor);
		    throw ex;
		}

		const_cast<Byte&>(_encodingMajor) = majVersion;
		const_cast<Byte&>(_encodingMinor) = minVersion;

		break;
	    }

	    case 'h':
	    {
		if(argument.empty())
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "udp " + str;
		    throw ex;
		}
		const_cast<string&>(_host) = argument;
		break;
	    }

	    case 'p':
	    {
		istringstream p(argument);
		if(!(p >> const_cast<Int&>(_port)) || !p.eof())
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "udp " + str;
		    throw ex;
		}
		break;
	    }

	    case 'c':
	    {
		if(!argument.empty())
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "udp " + str;
		    throw ex;
		}
		const_cast<bool&>(_connect) = true;
		break;
	    }

	    case 'z':
	    {
		if(!argument.empty())
		{
		    EndpointParseException ex(__FILE__, __LINE__);
		    ex.str = "udp " + str;
		    throw ex;
		}
		const_cast<bool&>(_compress) = true;
		break;
	    }

	    default:
	    {
		EndpointParseException ex(__FILE__, __LINE__);
		ex.str = "udp " + str;
		throw ex;
	    }
	}
    }

    if(_host.empty())
    {
	const_cast<string&>(_host) = _instance->defaultsAndOverrides()->defaultHost;
    }
}

IceInternal::UdpEndpoint::UdpEndpoint(BasicStream* s) :
    _instance(s->instance()),
    _port(0),
    _protocolMajor(protocolMajor),
    _protocolMinor(protocolMinor),
    _encodingMajor(encodingMajor),
    _encodingMinor(encodingMinor),
    _connect(false),
    _compress(false)    
{
    s->startReadEncaps();
    s->read(const_cast<string&>(_host));
    s->read(const_cast<Int&>(_port));
    s->read(const_cast<Byte&>(_protocolMajor));
    s->read(const_cast<Byte&>(_protocolMinor));
    s->read(const_cast<Byte&>(_encodingMajor));
    s->read(const_cast<Byte&>(_encodingMinor));
    if(_protocolMajor != protocolMajor)
    {
	UnsupportedProtocolException ex(__FILE__, __LINE__);
	ex.badMajor = _protocolMajor;
	ex.badMinor = _protocolMinor;
	ex.major = static_cast<unsigned char>(protocolMajor);
	ex.minor = static_cast<unsigned char>(protocolMinor);
	throw ex;
    }
    if(_encodingMajor != encodingMajor)
    {
	UnsupportedEncodingException ex(__FILE__, __LINE__);
	ex.badMajor = _encodingMajor;
	ex.badMinor = _encodingMinor;
	ex.major = static_cast<unsigned char>(encodingMajor);
	ex.minor = static_cast<unsigned char>(encodingMinor);
	throw ex;
    }
    // Not transmitted.
    //s->read(const_cast<bool&>(_connect));
    s->read(const_cast<bool&>(_compress));
    s->endReadEncaps();
}

void
IceInternal::UdpEndpoint::streamWrite(BasicStream* s) const
{
    s->write(UdpEndpointType);
    s->startWriteEncaps();
    s->write(_host);
    s->write(_port);
    s->write(_protocolMajor);
    s->write(_protocolMinor);
    s->write(_encodingMajor);
    s->write(_encodingMinor);
    // Not transmitted.
    //s->write(_connect);
    s->write(_compress);
    s->endWriteEncaps();
}

string
IceInternal::UdpEndpoint::toString() const
{
    ostringstream s;

    s << "udp";

    if(_protocolMajor != Byte(1) || _protocolMinor != Byte(0))
    {
	s << " -v "
	  << static_cast<unsigned>(static_cast<unsigned char>(_protocolMajor)) << "."
	  << static_cast<unsigned>(static_cast<unsigned char>(_protocolMinor));
    }

    if(_encodingMajor != Byte(1) || _encodingMinor != Byte(0))
    {
	s << " -e "
	  << static_cast<unsigned>(static_cast<unsigned char>(_encodingMajor)) << "."
	  << static_cast<unsigned>(static_cast<unsigned char>(_encodingMinor));
    }

    s << " -h " << _host << " -p " << _port;

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
IceInternal::UdpEndpoint::compress() const
{
    return _compress;
}

EndpointPtr
IceInternal::UdpEndpoint::compress(bool compress) const
{
    if(compress == _compress)
    {
	return const_cast<UdpEndpoint*>(this);
    }
    else
    {
	return new UdpEndpoint(_instance, _host, _port, compress);
    }
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
    endp = new UdpEndpoint(_instance, _host, p->effectivePort(), _compress);
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
    if(!udpTransceiver)
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

    if(_compress != p->_compress)
    {
	return false;
    }

    if(_connect != p->_connect)
    {
	return false;
    }

    if(_protocolMajor != p->_protocolMajor)
    {
	return false;
    }

    if(_protocolMinor != p->_protocolMinor)
    {
	return false;
    }

    if(_encodingMajor != p->_encodingMajor)
    {
	return false;
    }

    if(_encodingMinor != p->_encodingMinor)
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
IceInternal::UdpEndpoint::operator!=(const Endpoint& r) const
{
    return !operator==(r);
}

bool
IceInternal::UdpEndpoint::operator<(const Endpoint& r) const
{
    const UdpEndpoint* p = dynamic_cast<const UdpEndpoint*>(&r);
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

    if(_protocolMajor < p->_protocolMajor)
    {
	return true;
    }
    else if(p->_protocolMajor < _protocolMajor)
    {
	return false;
    }

    if(_protocolMinor < p->_protocolMinor)
    {
	return true;
    }
    else if(p->_protocolMinor < _protocolMinor)
    {
	return false;
    }

    if(_encodingMajor < p->_encodingMajor)
    {
	return true;
    }
    else if(p->_encodingMajor < _encodingMajor)
    {
	return false;
    }

    if(_encodingMinor < p->_encodingMinor)
    {
	return true;
    }
    else if(p->_encodingMinor < _encodingMinor)
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

string
IceInternal::UdpEndpointFactory::protocol() const
{
    return "udp";
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
