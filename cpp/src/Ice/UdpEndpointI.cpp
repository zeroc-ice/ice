// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/UdpEndpointI.h>
#include <Ice/Network.h>
#include <Ice/UdpConnector.h>
#include <Ice/UdpTransceiver.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Protocol.h>
#include <Ice/HashUtil.h>
#include <Ice/Logger.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::UdpEndpointI::UdpEndpointI(const InstancePtr& instance, const string& ho, Int po, const string& mif, 
                                        Int mttl, bool conn, const string& conId, bool co) :
    EndpointI(conId),
    _instance(instance),
    _host(ho),
    _port(po),
    _mcastInterface(mif),
    _mcastTtl(mttl),
    _connect(conn),
    _compress(co)
{
}

IceInternal::UdpEndpointI::UdpEndpointI(const InstancePtr& instance, const string& str, bool oaEndpoint) :
    EndpointI(""),
    _instance(instance),
    _port(0),
    _mcastTtl(-1),
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
        if(option[0] != '-')
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "expected an endpoint option but found `" + option + "' in endpoint `udp " + str + "'";
            throw ex;
        }

        string argument;
        string::size_type argumentBeg = str.find_first_not_of(delim, end);
        if(argumentBeg != string::npos && str[argumentBeg] != '-')
        {
            beg = argumentBeg;
            if(str[beg] == '\"')
            {
                end = str.find_first_of('\"', beg + 1);
                if(end == string::npos)
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "mismatched quotes around `" + argument + "' in endpoint `udp " + str + "'";
                    throw ex;
                }
                else
                {
                    ++end;
                }
            }
            else
            {
                end = str.find_first_of(delim, beg);
                if(end == string::npos)
                {
                    end = str.length();
                }
            }
            argument = str.substr(beg, end - beg);
            if(argument[0] == '\"' && argument[argument.size() - 1] == '\"')
            {
                argument = argument.substr(1, argument.size() - 2);
            }
        }

        if(option == "-h")
        {
            if(argument.empty())
            {
                EndpointParseException ex(__FILE__, __LINE__);
                ex.str = "no argument provided for -h option in endpoint `udp " + str + "'";
                throw ex;
            }
            const_cast<string&>(_host) = argument;
        }
        else if(option == "-p")
        {
            if(argument.empty())
            {
                EndpointParseException ex(__FILE__, __LINE__);
                ex.str = "no argument provided for -p option in endpoint `udp " + str + "'";
                throw ex;
            }
            istringstream p(argument);
            if(!(p >> const_cast<Int&>(_port)) || !p.eof())
            {
                EndpointParseException ex(__FILE__, __LINE__);
                ex.str = "invalid port value `" + argument + "' in endpoint `udp " + str + "'";
                throw ex;
            }
            else if(_port < 0 || _port > 65535)
            {
                EndpointParseException ex(__FILE__, __LINE__);
                ex.str = "port value `" + argument + "' out of range in endpoint `udp " + str + "'";
                throw ex;
            }
        }
        else if(option == "-c")
        {
            if(!argument.empty())
            {
                EndpointParseException ex(__FILE__, __LINE__);
                ex.str = "unexpected argument `" + argument + "' provided for -c option in `udp " + str + "'";
                throw ex;
            }
            const_cast<bool&>(_connect) = true;
        }
        else if(option == "-z")
        {
            if(!argument.empty())
            {
                EndpointParseException ex(__FILE__, __LINE__);
                ex.str = "unexpected argument `" + argument + "' provided for -z option in `udp " + str + "'";
                throw ex;
            }
            const_cast<bool&>(_compress) = true;
        }
        else if(option == "-v" || option == "-e")
        {
            if(argument.empty())
            {
                EndpointParseException ex(__FILE__, __LINE__);
                ex.str = "no argument provided for " + option + " option in endpoint `udp " + str + "'";
                throw ex;
            }
            try
            {
                Ice::Byte major, minor;
                IceInternal::stringToMajorMinor(argument, major, minor);
                if(major != 1 || minor != 0)
                {
                    _instance->initializationData().logger->warning("deprecated udp endpoint option: " + option);
                }
            }
            catch(const VersionParseException& e)
            {
                EndpointParseException ex(__FILE__, __LINE__);
                ex.str = "invalid version `" + argument + "' in endpoint `udp " + str + "':\n" + e.str;
                throw ex;
            }
        }
        else if(option == "--interface")
        {
            if(argument.empty())
            {
                EndpointParseException ex(__FILE__, __LINE__);
                ex.str = "no argument provided for --interface option in endpoint `udp " + str + "'";
                throw ex;
            }
            const_cast<string&>(_mcastInterface) = argument;
        }
        else if(option == "--ttl")
        {
            if(argument.empty())
            {
                EndpointParseException ex(__FILE__, __LINE__);
                ex.str = "no argument provided for --ttl option in endpoint `udp " + str + "'";
                throw ex;
            }
            istringstream p(argument);
            if(!(p >> const_cast<Int&>(_mcastTtl)) || !p.eof())
            {
                EndpointParseException ex(__FILE__, __LINE__);
                ex.str = "invalid TTL value `" + argument + "' in endpoint `udp " + str + "'";
                throw ex;
            }
        }
        else
        {
            Ice::EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "unknown option `" + option + "' in endpoint `udp " + str + "'";
            throw ex;
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
            ex.str = "`-h *' not valid for proxy endpoint `udp " + str + "'";
            throw ex;
        }
    }
}

IceInternal::UdpEndpointI::UdpEndpointI(BasicStream* s) :
    _instance(s->instance()),
    _port(0),
    _mcastTtl(-1),
    _connect(false),
    _compress(false)
{
    s->startReadEncaps();
    s->read(const_cast<string&>(_host), false);
    s->read(const_cast<Int&>(_port));
    if(s->getReadEncoding() == Ice::Encoding_1_0)
    {
        Ice::Byte b;
        s->read(b);
        s->read(b);
        s->read(b);
        s->read(b);
    }
    // Not transmitted.
    //s->read(const_cast<bool&>(_connect));
    s->read(const_cast<bool&>(_compress));
    s->endReadEncaps();
}

void
IceInternal::UdpEndpointI::streamWrite(BasicStream* s) const
{
    s->write(UDPEndpointType);
    s->startWriteEncaps();
    s->write(_host, false);
    s->write(_port);
    if(s->getWriteEncoding() == Ice::Encoding_1_0)
    {
        s->write(Ice::Protocol_1_0);
        s->write(Ice::Encoding_1_0);
    }
    // Not transmitted.
    //s->write(_connect);
    s->write(_compress);
    s->endWriteEncaps();
}

string
IceInternal::UdpEndpointI::toString() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    ostringstream s;

    s << "udp";

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

    if(_mcastInterface.length() > 0)
    {
        s << " --interface " << _mcastInterface;
    }

    if(_mcastTtl != -1)
    {
        s << " --ttl " << _mcastTtl;
    }

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

EndpointInfoPtr
IceInternal::UdpEndpointI::getInfo() const
{
    class InfoI : public Ice::UDPEndpointInfo
    {
    public:

        InfoI(bool comp, const string& host, Ice::Int port, const std::string& mcastInterface, Ice::Int mcastTtl) :
            UDPEndpointInfo(-1, comp, host, port, mcastInterface, mcastTtl)
        {
        }

        virtual Ice::Short
        type() const
        {
            return UDPEndpointType;
        }
        
        virtual bool
        datagram() const
        {
            return true;
        }

        virtual bool
        secure() const
        {
            return false;
        }
    };

    return new InfoI(_compress, _host, _port, _mcastInterface, _mcastTtl);
}

Short
IceInternal::UdpEndpointI::type() const
{
    return UDPEndpointType;
}

std::string
IceInternal::UdpEndpointI::protocol() const
{
    return "udp";
}

Int
IceInternal::UdpEndpointI::timeout() const
{
    return -1;
}

EndpointIPtr
IceInternal::UdpEndpointI::timeout(Int) const
{
    return const_cast<UdpEndpointI*>(this);
}

EndpointIPtr
IceInternal::UdpEndpointI::connectionId(const string& connectionId) const
{
    if(connectionId == _connectionId)
    {
        return const_cast<UdpEndpointI*>(this);
    }
    else
    {
        return new UdpEndpointI(_instance, _host, _port, _mcastInterface, _mcastTtl, _connect, connectionId, _compress);
    }
}

bool
IceInternal::UdpEndpointI::compress() const
{
    return _compress;
}

EndpointIPtr
IceInternal::UdpEndpointI::compress(bool compress) const
{
    if(compress == _compress)
    {
        return const_cast<UdpEndpointI*>(this);
    }
    else
    {
        return new UdpEndpointI(_instance, _host, _port, _mcastInterface, _mcastTtl, _connect, _connectionId, compress);
    }
}

bool
IceInternal::UdpEndpointI::datagram() const
{
    return true;
}

bool
IceInternal::UdpEndpointI::secure() const
{
    return false;
}

TransceiverPtr
IceInternal::UdpEndpointI::transceiver(EndpointIPtr& endp) const
{
    UdpTransceiver* p = new UdpTransceiver(_instance, _host, _port, _mcastInterface, _connect);
    endp = new UdpEndpointI(_instance, _host, p->effectivePort(), _mcastInterface, _mcastTtl, _connect, _connectionId, 
                            _compress);
    return p;
}

vector<ConnectorPtr>
IceInternal::UdpEndpointI::connectors(Ice::EndpointSelectionType selType) const
{
    return _instance->endpointHostResolver()->resolve(_host, _port, selType, const_cast<UdpEndpointI*>(this));
}

void
IceInternal::UdpEndpointI::connectors_async(Ice::EndpointSelectionType selType, const EndpointI_connectorsPtr& cb) const
{
    _instance->endpointHostResolver()->resolve(_host, _port, selType, const_cast<UdpEndpointI*>(this), cb);
}

AcceptorPtr
IceInternal::UdpEndpointI::acceptor(EndpointIPtr& endp, const string&) const
{
    endp = const_cast<UdpEndpointI*>(this);
    return 0;
}

vector<EndpointIPtr>
IceInternal::UdpEndpointI::expand() const
{
    vector<EndpointIPtr> endps;
    vector<string> hosts = getHostsForEndpointExpand(_host, _instance->protocolSupport(), false);
    if(hosts.empty())
    {
        endps.push_back(const_cast<UdpEndpointI*>(this));
    }
    else
    {
        for(vector<string>::const_iterator p = hosts.begin(); p != hosts.end(); ++p)
        {
            endps.push_back(new UdpEndpointI(_instance, *p, _port, _mcastInterface, _mcastTtl, _connect, _connectionId,
                                             _compress));
        }
    }
    return endps;
}

bool
IceInternal::UdpEndpointI::equivalent(const EndpointIPtr& endpoint) const
{
    const UdpEndpointI* udpEndpointI = dynamic_cast<const UdpEndpointI*>(endpoint.get());
    if(!udpEndpointI)
    {
        return false;
    }
    return udpEndpointI->_host == _host && udpEndpointI->_port == _port;
}

vector<ConnectorPtr>
IceInternal::UdpEndpointI::connectors(const vector<Address>& addresses, const NetworkProxyPtr&) const
{
    vector<ConnectorPtr> connectors;
    for(unsigned int i = 0; i < addresses.size(); ++i)
    {
        connectors.push_back(new UdpConnector(_instance, addresses[i], _mcastInterface, _mcastTtl, _connectionId));
    }
    return connectors;
}

bool
IceInternal::UdpEndpointI::operator==(const LocalObject& r) const
{
    const UdpEndpointI* p = dynamic_cast<const UdpEndpointI*>(&r);
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

    if(_compress != p->_compress)
    {
        return false;
    }

    if(_connectionId != p->_connectionId)
    {
        return false;
    }

    if(_connect != p->_connect)
    {
        return false;
    }

    if(_mcastTtl != p->_mcastTtl)
    {
        return false;
    }

    if(_mcastInterface != p->_mcastInterface)
    {
        return false;
    }

    return true;
}

bool
IceInternal::UdpEndpointI::operator<(const LocalObject& r) const
{
    const UdpEndpointI* p = dynamic_cast<const UdpEndpointI*>(&r);
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

    if(!_connect && p->_connect)
    {
        return true;
    }
    else if(!p->_connect && _connect)
    {
        return false;
    }

    if(_mcastTtl < p->_mcastTtl)
    {
        return true;
    }
    else if(p->_mcastTtl < _mcastTtl)
    {
        return false;
    }

    if(_mcastInterface < p->_mcastInterface)
    {
        return true;
    }
    else if(p->_mcastInterface < _mcastInterface)
    {
        return false;
    }

    return false;
}

Ice::Int
IceInternal::UdpEndpointI::hashInit() const
{
    Ice::Int h = 5381;
    hashAdd(h, UDPEndpointType);
    hashAdd(h, _host);
    hashAdd(h, _port);
    hashAdd(h, _mcastInterface);
    hashAdd(h, _mcastTtl);
    hashAdd(h, _connect);
    hashAdd(h, _connectionId);
    hashAdd(h, _compress);
    return h;
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
    return UDPEndpointType;
}

string
IceInternal::UdpEndpointFactory::protocol() const
{
    return "udp";
}

EndpointIPtr
IceInternal::UdpEndpointFactory::create(const std::string& str, bool oaEndpoint) const
{
    return new UdpEndpointI(_instance, str, oaEndpoint);
}

EndpointIPtr
IceInternal::UdpEndpointFactory::read(BasicStream* s) const
{
    return new UdpEndpointI(s);
}

void
IceInternal::UdpEndpointFactory::destroy()
{
    _instance = 0;
}
