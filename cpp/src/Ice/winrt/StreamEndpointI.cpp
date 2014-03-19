// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/winrt/StreamEndpointI.h>
#include <Ice/winrt/StreamAcceptor.h>
#include <Ice/winrt/StreamConnector.h>
#include <Ice/winrt/StreamTransceiver.h>

#include <Ice/Network.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/HashUtil.h>

#include <IceSSL/EndpointInfo.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

template<class T> class InfoI : public T
{
public:
    
    InfoI(Ice::Short type, Ice::Int to, bool comp, const string& host, Ice::Int port) :
        T(to, comp, host, port), _type(type)
    {
    }
    
    virtual Ice::Short
    type() const
    {
        return _type;
    }
    
    virtual bool
    datagram() const
    {
        return false;
    }
    
    virtual bool
    secure() const
    {
        return _type == IceSSL::EndpointType;
    }

private:
    
    Ice::Short _type;
};

}

IceInternal::StreamEndpointI::StreamEndpointI(const InstancePtr& instance, Ice::Short type, const string& ho, Int po, 
                                              Int ti, const string& conId, bool co) :
    EndpointI(conId),
    _instance(instance),
    _type(type),
    _host(ho),
    _port(po),
    _timeout(ti),
    _compress(co)
{
}

IceInternal::StreamEndpointI::StreamEndpointI(const InstancePtr& instance, Ice::Short type, const string& str, 
                                              bool oaEndpoint) :
    EndpointI(""),
    _instance(instance),
    _type(type),
    _port(0),
    _timeout(-1),
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
            ex.str = "expected an endpoint option but found `" + option + "' in endpoint `" + typeToString(_type) +
                " " + str + "'";
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
            if(argument[0] == '\"' && argument[argument.size() - 1] == '\"')
            {
                argument = argument.substr(1, argument.size() - 2);
            }
        }

        switch(option[1])
        {
            case 'h':
            {
                if(argument.empty())
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "no argument provided for -h option in endpoint `" + typeToString(_type) + " " +
                        str + "'";
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
                    ex.str = "no argument provided for -p option in endpoint `" + typeToString(_type) + " " + str + "'";
                    throw ex;
                }
                istringstream p(argument);
                if(!(p >> const_cast<Int&>(_port)) || !p.eof())
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "invalid port value `" + argument + "' in endpoint `" + typeToString(_type) + " " + 
                        str + "'";
                    throw ex;
                }
                else if(_port < 0 || _port > 65535)
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "port value `" + argument + "' out of range in endpoint `" + typeToString(_type) + " " + 
                        str + "'";
                    throw ex;
                }
                break;
            }

            case 't':
            {
                if(argument.empty())
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "no argument provided for -t option in endpoint `" + typeToString(_type) + " " + str + "'";
                    throw ex;
                }
                istringstream t(argument);
                if(!(t >> const_cast<Int&>(_timeout)) || !t.eof())
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "invalid timeout value `" + argument + "' in endpoint `" + typeToString(_type) + " " +
                        str + "'";
                    throw ex;
                }
                break;
            }

            case 'z':
            {
                if(!argument.empty())
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "unexpected argument `" + argument + "' provided for -z option in `" + 
                        typeToString(_type) + " " + str + "'";
                    throw ex;
                }
                const_cast<bool&>(_compress) = true;
                break;
            }

            default:
            {
                Ice::EndpointParseException ex(__FILE__, __LINE__);
                ex.str = "unknown option `" + option + "' in endpoint `"+ typeToString(_type) + " " + str + "'";
                throw ex;
            }
        }
    }

    if(_host.empty())
    {
        const_cast<string&>(_host) = _instance->defaultsAndOverrides()->defaultHost;
    }
    else if(_host == "*")
    {
        if(oaEndpoint)
        {
            const_cast<string&>(_host) = string();
        }
        else
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "`-h *' not valid for proxy endpoint `" + typeToString(_type) + " " + str + "'";
            throw ex;
        }
    }
}

IceInternal::StreamEndpointI::StreamEndpointI(BasicStream* s, Ice::Short type) :
    _instance(s->instance()),
    _type(type),
    _port(0),
    _timeout(-1),
    _compress(false)
{
    s->startReadEncaps();
    s->read(const_cast<string&>(_host), false);
    s->read(const_cast<Int&>(_port));
    s->read(const_cast<Int&>(_timeout));
    s->read(const_cast<bool&>(_compress));
    s->endReadEncaps();
}

void
IceInternal::StreamEndpointI::streamWrite(BasicStream* s) const
{
    s->write(_type);
    s->startWriteEncaps();
    s->write(_host, false);
    s->write(_port);
    s->write(_timeout);
    s->write(_compress);
    s->endWriteEncaps();
}

string
IceInternal::StreamEndpointI::toString() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    ostringstream s;
    s << "" + typeToString(_type) + "";
    
    if(!_host.empty())
    {
        s << " -h ";
        bool addQuote = _host.find(':') != string::npos;
        if(addQuote)
        {
            s << "\"";
        }
        s << _host;
        if(addQuote)
        {
            s << "\"";
        }
    }

    s << " -p " << _port;
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

EndpointInfoPtr
IceInternal::StreamEndpointI::getInfo() const
{
    switch(_type)
    {
    case TCPEndpointType:
        return new InfoI<Ice::TCPEndpointInfo>(_type, _timeout, _compress, _host, _port);
    case IceSSL::EndpointType:
        return new InfoI<IceSSL::EndpointInfo>(_type, _timeout, _compress, _host, _port);
    default:
        assert(false);
        return 0;
    }
}

Short
IceInternal::StreamEndpointI::type() const
{
    return _type;
}

string
IceInternal::StreamEndpointI::protocol() const
{
    return _type == TCPEndpointType ? "tcp" : "ssl";
}

Int
IceInternal::StreamEndpointI::timeout() const
{
    return _timeout;
}

EndpointIPtr
IceInternal::StreamEndpointI::timeout(Int timeout) const
{
    if(timeout == _timeout)
    {
        return const_cast<StreamEndpointI*>(this);
    }
    else
    {
        return new StreamEndpointI(_instance, _type, _host, _port, timeout, _connectionId, _compress);
    }
}

EndpointIPtr
IceInternal::StreamEndpointI::connectionId(const string& connectionId) const
{
    if(connectionId == _connectionId)
    {
        return const_cast<StreamEndpointI*>(this);
    }
    else
    {
      return new StreamEndpointI(_instance, _type, _host, _port, _timeout, connectionId, _compress);
    }
}

bool
IceInternal::StreamEndpointI::compress() const
{
    return _compress;
}

EndpointIPtr
IceInternal::StreamEndpointI::compress(bool compress) const
{
    if(compress == _compress)
    {
        return const_cast<StreamEndpointI*>(this);
    }
    else
    {
        return new StreamEndpointI(_instance, _type, _host, _port, _timeout, _connectionId, compress);
    }
}

bool
IceInternal::StreamEndpointI::datagram() const
{
    return false;
}

bool
IceInternal::StreamEndpointI::secure() const
{
    return _type == IceSSL::EndpointType;
}

TransceiverPtr
IceInternal::StreamEndpointI::transceiver(EndpointIPtr& endp) const
{
    endp = const_cast<StreamEndpointI*>(this);
    return 0;
}

vector<ConnectorPtr>
IceInternal::StreamEndpointI::connectors(Ice::EndpointSelectionType selType) const
{
    return connectors(getAddresses(_host, _port, _instance->protocolSupport(), selType, _instance->preferIPv6(), true));
}

void
IceInternal::StreamEndpointI::connectors_async(Ice::EndpointSelectionType selType,
                                               const EndpointI_connectorsPtr& callback) const
{
    callback->connectors(connectors(selType));
}

AcceptorPtr
IceInternal::StreamEndpointI::acceptor(EndpointIPtr& endp, const string&) const
{
    StreamAcceptor* p = new StreamAcceptor(_instance, _type, _host, _port);
    endp = new StreamEndpointI(_instance, _type, _host, p->effectivePort(), _timeout, _connectionId, _compress);
    return p;
}


vector<EndpointIPtr>
IceInternal::StreamEndpointI::expand() const
{
    vector<EndpointIPtr> endps;
    vector<string> hosts = getHostsForEndpointExpand(_host, _instance->protocolSupport(), false);
    if(hosts.empty())
    {
        endps.push_back(const_cast<StreamEndpointI*>(this));
    }
    else
    {
        for(vector<string>::const_iterator p = hosts.begin(); p != hosts.end(); ++p)
        {
            endps.push_back(new StreamEndpointI(_instance, _type, *p, _port, _timeout, _connectionId, _compress));
        }
    }
    return endps;
}

bool
IceInternal::StreamEndpointI::equivalent(const EndpointIPtr& endpoint) const
{
    const StreamEndpointI* streamEndpointI = dynamic_cast<const StreamEndpointI*>(endpoint.get());
    if(!streamEndpointI)
    {
        return false;
    }
    return streamEndpointI->_host == _host && streamEndpointI->_port == _port;
}

bool
IceInternal::StreamEndpointI::operator==(const LocalObject& r) const
{
    const StreamEndpointI* p = dynamic_cast<const StreamEndpointI*>(&r);
    if(!p)
    {
        return false;
    }

    if(this == p)
    {
        return true;
    }

    if(_type != p->_type)
    {
        return false;
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
IceInternal::StreamEndpointI::operator<(const LocalObject& r) const
{
    const StreamEndpointI* p = dynamic_cast<const StreamEndpointI*>(&r);
    if(!p)
    {
        const EndpointI* e = dynamic_cast<const EndpointI*>(&r);
        if(!e)
        {
            return false;
        }
        return type() < e->type();
    }

    if(this == p)
    {
        return false;
    }

    if(_type < p->_type)
    {
        return true;
    }
    else if(p->_type < _type)
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

Ice::Int
IceInternal::StreamEndpointI::hashInit() const
{
    Ice::Int h = 5381;
    hashAdd(h, _type);
    hashAdd(h, _host);
    hashAdd(h, _port);
    hashAdd(h, _timeout);
    hashAdd(h, _connectionId);
    hashAdd(h, _compress);
    return h;
}

vector<ConnectorPtr>
IceInternal::StreamEndpointI::connectors(const vector<Address>& addresses) const
{
    vector<ConnectorPtr> connectors;
    for(unsigned int i = 0; i < addresses.size(); ++i)
    {
        connectors.push_back(new StreamConnector(_instance, _type, addresses[i], _timeout, _connectionId));
    }
    return connectors;
}

IceInternal::StreamEndpointFactory::StreamEndpointFactory(const InstancePtr& instance, Ice::Short type)
    : _instance(instance), _type(type)
{
}

IceInternal::StreamEndpointFactory::~StreamEndpointFactory()
{
}

Short
IceInternal::StreamEndpointFactory::type() const
{
    return _type;
}

string
IceInternal::StreamEndpointFactory::protocol() const
{
    return typeToString(_type);
}

EndpointIPtr
IceInternal::StreamEndpointFactory::create(const std::string& str, bool oaEndpoint) const
{
    return new StreamEndpointI(_instance, _type, str, oaEndpoint);
}

EndpointIPtr
IceInternal::StreamEndpointFactory::read(BasicStream* s) const
{
    return new StreamEndpointI(s, _type);
}

void
IceInternal::StreamEndpointFactory::destroy()
{
    _instance = 0;
}
