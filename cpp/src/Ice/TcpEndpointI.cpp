// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/TcpEndpointI.h>
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

IceInternal::TcpEndpointI::TcpEndpointI(const InstancePtr& instance, const string& ho, Int po, Int ti,
                                        const string& conId, bool co, bool oae) :
    _instance(instance),
    _host(ho),
    _port(po),
    _timeout(ti),
    _connectionId(conId),
    _compress(co),
    _oaEndpoint(oae)
{
}

IceInternal::TcpEndpointI::TcpEndpointI(const InstancePtr& instance, const string& str, bool oaEndpoint) :
    _instance(instance),
    _port(0),
    _timeout(-1),
    _compress(false),
    _oaEndpoint(oaEndpoint)
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
                istringstream p(argument);
                if(!(p >> const_cast<Int&>(_port)) || !p.eof() || _port < 0 || _port > 65535)
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "tcp " + str;
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
                    ex.str = "tcp " + str;
                    throw ex;
                }
                break;
            }

            case 'z':
            {
                if(!argument.empty())
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "tcp " + str;
                    throw ex;
                }
                const_cast<bool&>(_compress) = true;
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

    if(_host.empty())
    {
        const_cast<string&>(_host) = _instance->defaultsAndOverrides()->defaultHost;
        if(_host.empty())
        {
            if(oaEndpoint)
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
}

IceInternal::TcpEndpointI::TcpEndpointI(BasicStream* s) :
    _instance(s->instance()),
    _port(0),
    _timeout(-1),
    _compress(false),
    _oaEndpoint(false)
{
    s->startReadEncaps();
    s->read(const_cast<string&>(_host), false);
    s->read(const_cast<Int&>(_port));
    s->read(const_cast<Int&>(_timeout));
    s->read(const_cast<bool&>(_compress));
    s->endReadEncaps();
}

void
IceInternal::TcpEndpointI::streamWrite(BasicStream* s) const
{
    s->write(TcpEndpointType);
    s->startWriteEncaps();
    s->write(_host, false);
    s->write(_port);
    s->write(_timeout);
    s->write(_compress);
    s->endWriteEncaps();
}

string
IceInternal::TcpEndpointI::toString() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    ostringstream s;
    s << "tcp -h " << _host << " -p " << _port;
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
IceInternal::TcpEndpointI::type() const
{
    return TcpEndpointType;
}

Int
IceInternal::TcpEndpointI::timeout() const
{
    return _timeout;
}

EndpointIPtr
IceInternal::TcpEndpointI::timeout(Int timeout) const
{
    if(timeout == _timeout)
    {
        return const_cast<TcpEndpointI*>(this);
    }
    else
    {
        return new TcpEndpointI(_instance, _host, _port, timeout, _connectionId, _compress, _oaEndpoint);
    }
}

EndpointIPtr
IceInternal::TcpEndpointI::connectionId(const string& connectionId) const
{
    if(connectionId == _connectionId)
    {
        return const_cast<TcpEndpointI*>(this);
    }
    else
    {
        return new TcpEndpointI(_instance, _host, _port, _timeout, connectionId, _compress, _oaEndpoint);
    }
}

bool
IceInternal::TcpEndpointI::compress() const
{
    return _compress;
}

EndpointIPtr
IceInternal::TcpEndpointI::compress(bool compress) const
{
    if(compress == _compress)
    {
        return const_cast<TcpEndpointI*>(this);
    }
    else
    {
        return new TcpEndpointI(_instance, _host, _port, _timeout, _connectionId, compress, _oaEndpoint);
    }
}

bool
IceInternal::TcpEndpointI::datagram() const
{
    return false;
}

bool
IceInternal::TcpEndpointI::secure() const
{
    return false;
}

bool
IceInternal::TcpEndpointI::unknown() const
{
    return false;
}

TransceiverPtr
IceInternal::TcpEndpointI::transceiver(EndpointIPtr& endp) const
{
    endp = const_cast<TcpEndpointI*>(this);
    return 0;
}

vector<ConnectorPtr>
IceInternal::TcpEndpointI::connectors() const
{
    vector<ConnectorPtr> connectors;
    vector<struct sockaddr_in> addresses = getAddresses(_host, _port);
    for(unsigned int i = 0; i < addresses.size(); ++i)
    {
        connectors.push_back(new TcpConnector(_instance, addresses[i], _timeout, _connectionId));
    }
    return connectors;
}

AcceptorPtr
IceInternal::TcpEndpointI::acceptor(EndpointIPtr& endp, const string&) const
{
    TcpAcceptor* p = new TcpAcceptor(_instance, _host, _port);
    endp = new TcpEndpointI(_instance, _host, p->effectivePort(), _timeout, _connectionId, _compress, _oaEndpoint);
    return p;
}


vector<EndpointIPtr>
IceInternal::TcpEndpointI::expand() const
{
    vector<EndpointIPtr> endps;
    if(_host == "0.0.0.0")
    {
        vector<string> hosts = getLocalHosts();
        for(unsigned int i = 0; i < hosts.size(); ++i)
        {
            if(!_oaEndpoint || hosts.size() == 1 || hosts[i] != "127.0.0.1")
            {
                endps.push_back(new TcpEndpointI(_instance, hosts[i], _port, _timeout, _connectionId, _compress, 
                                                 _oaEndpoint));
            }
        }
    }
    else
    {
        endps.push_back(const_cast<TcpEndpointI*>(this));
    }   
    return endps;
}

bool
IceInternal::TcpEndpointI::equivalent(const TransceiverPtr&) const
{
    return false;
}

bool
IceInternal::TcpEndpointI::equivalent(const AcceptorPtr& acceptor) const
{
    const TcpAcceptor* tcpAcceptor = dynamic_cast<const TcpAcceptor*>(acceptor.get());
    if(!tcpAcceptor)
    {
        return false;
    }
    return tcpAcceptor->equivalent(_host, _port);
}

bool
IceInternal::TcpEndpointI::operator==(const EndpointI& r) const
{
    const TcpEndpointI* p = dynamic_cast<const TcpEndpointI*>(&r);
    if(!p)
    {
        return false;
    }

    if(this == p)
    {
        return true;
    }

    if(_host != p->_host)
    {
        return false;
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

    return true;
}

bool
IceInternal::TcpEndpointI::operator!=(const EndpointI& r) const
{
    return !operator==(r);
}

bool
IceInternal::TcpEndpointI::operator<(const EndpointI& r) const
{
    const TcpEndpointI* p = dynamic_cast<const TcpEndpointI*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    if(this == p)
    {
        return false;
    }

    if(_host < p->_host)
    {
        return true;
    }
    else if (p->_host < _host)
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

string
IceInternal::TcpEndpointFactory::protocol() const
{
    return "tcp";
}

EndpointIPtr
IceInternal::TcpEndpointFactory::create(const std::string& str, bool oaEndpoint) const
{
    return new TcpEndpointI(_instance, str, oaEndpoint);
}

EndpointIPtr
IceInternal::TcpEndpointFactory::read(BasicStream* s) const
{
    return new TcpEndpointI(s);
}

void
IceInternal::TcpEndpointFactory::destroy()
{
    _instance = 0;
}
