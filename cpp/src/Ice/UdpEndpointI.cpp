// Copyright (c) ZeroC, Inc.

#include "UdpEndpointI.h"
#include "HashUtil.h"
#include "Ice/InputStream.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Logger.h"
#include "Ice/OutputStream.h"
#include "Network.h"
#include "ProtocolInstance.h"
#include "UdpConnector.h"
#include "UdpTransceiver.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    const char* const udpPluginName = "IceUDP";

    Plugin* createIceUDP(const CommunicatorPtr& c, const string& name, const StringSeq&)
    {
        string pluginName{udpPluginName};

        if (name != pluginName)
        {
            throw PluginInitializationException{
                __FILE__,
                __LINE__,
                "the UDP plug-in must be named '" + pluginName + "'"};
        }

        return new EndpointFactoryPlugin(
            c,
            make_shared<UdpEndpointFactory>(make_shared<ProtocolInstance>(c, UDPEndpointType, "udp", false)));
    }
}

Ice::PluginFactory
Ice::udpPluginFactory()
{
    return {udpPluginName, createIceUDP};
}

IceInternal::UdpEndpointI::UdpEndpointI(
    const ProtocolInstancePtr& instance,
    const string& host,
    int32_t port,
    const Address& sourceAddr,
    string mcastInterface,
    int32_t mttl,
    const string& connectionId,
    bool compress)
    : IPEndpointI(instance, host, port, sourceAddr, connectionId),
      _mcastTtl(mttl),
      _mcastInterface(std::move(mcastInterface)),
      _compress(compress)
{
}

IceInternal::UdpEndpointI::UdpEndpointI(const ProtocolInstancePtr& instance)
    : IPEndpointI(instance),
      _mcastTtl(-1),
      _compress(false)
{
}

IceInternal::UdpEndpointI::UdpEndpointI(const ProtocolInstancePtr& instance, InputStream* s)
    : IPEndpointI(instance, s),
      _mcastTtl(-1),
      _compress(false)
{
    if (s->getEncoding() == Ice::Encoding_1_0)
    {
        uint8_t b;
        s->read(b);
        s->read(b);
        s->read(b);
        s->read(b);
    }
    s->read(const_cast<bool&>(_compress));
}

void
IceInternal::UdpEndpointI::streamWriteImpl(OutputStream* s) const
{
    IPEndpointI::streamWriteImpl(s);
    if (s->getEncoding() == Ice::Encoding_1_0)
    {
        s->write(Protocol_1_0);
        s->write(Ice::Encoding_1_0);
    }
    s->write(_compress);
}

EndpointInfoPtr
IceInternal::UdpEndpointI::getInfo() const noexcept
{
    return make_shared<UDPEndpointInfo>(
        _compress,
        _host,
        _port,
        inetAddrToString(_sourceAddr),
        _mcastInterface,
        _mcastTtl);
}

int32_t
IceInternal::UdpEndpointI::timeout() const
{
    return -1;
}

EndpointIPtr
IceInternal::UdpEndpointI::timeout(int32_t) const
{
    return const_cast<UdpEndpointI*>(this)->shared_from_this();
}

bool
IceInternal::UdpEndpointI::compress() const
{
    return _compress;
}

EndpointIPtr
IceInternal::UdpEndpointI::compress(bool compress) const
{
    if (compress == _compress)
    {
        return const_cast<UdpEndpointI*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<
            UdpEndpointI>(_instance, _host, _port, _sourceAddr, _mcastInterface, _mcastTtl, _connectionId, compress);
    }
}

bool
IceInternal::UdpEndpointI::datagram() const
{
    return true;
}

shared_ptr<EndpointI>
IceInternal::UdpEndpointI::toPublishedEndpoint(string publishedHost) const
{
    return make_shared<UdpEndpointI>(
        _instance,
        publishedHost.empty() ? _host : publishedHost,
        _port,
        Address{},
        "",
        -1,
        "",
        _compress);
}

TransceiverPtr
IceInternal::UdpEndpointI::transceiver() const
{
    return make_shared<UdpTransceiver>(
        dynamic_pointer_cast<UdpEndpointI>(const_cast<UdpEndpointI*>(this)->shared_from_this()),
        _instance,
        _host,
        _port,
        _mcastInterface);
}

AcceptorPtr
IceInternal::UdpEndpointI::acceptor(const string&, const optional<Ice::SSL::ServerAuthenticationOptions>&) const
{
    return nullptr;
}

UdpEndpointIPtr
IceInternal::UdpEndpointI::endpoint(const UdpTransceiverPtr& transceiver) const
{
    int port = transceiver->effectivePort();
    if (port == _port)
    {
        return dynamic_pointer_cast<UdpEndpointI>(const_cast<UdpEndpointI*>(this)->shared_from_this());
    }
    else
    {
        return make_shared<
            UdpEndpointI>(_instance, _host, port, _sourceAddr, _mcastInterface, _mcastTtl, _connectionId, _compress);
    }
}

void
IceInternal::UdpEndpointI::initWithOptions(vector<string>& args, bool oaEndpoint)
{
    IPEndpointI::initWithOptions(args, oaEndpoint);

    if (_mcastInterface == "*")
    {
        if (oaEndpoint)
        {
            const_cast<string&>(_mcastInterface) = string();
        }
        else
        {
            throw ParseException(
                __FILE__,
                __LINE__,
                "'--interface *' not valid for proxy endpoint '" + toString() + "'");
        }
    }
}

string
IceInternal::UdpEndpointI::options() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    ostringstream s;
    s << IPEndpointI::options();

    if (_mcastInterface.length() > 0)
    {
        s << " --interface ";
        bool addQuote = _mcastInterface.find(':') != string::npos;
        if (addQuote)
        {
            s << "\"";
        }
        s << _mcastInterface;
        if (addQuote)
        {
            s << "\"";
        }
    }

    if (_mcastTtl != -1)
    {
        // Use to_string for locale independent formatting.
        s << " --ttl " << to_string(_mcastTtl);
    }

    if (_compress)
    {
        s << " -z";
    }

    return s.str();
}

bool
IceInternal::UdpEndpointI::operator==(const Endpoint& r) const
{
    if (!IPEndpointI::operator==(r))
    {
        return false;
    }

    const auto* p = dynamic_cast<const UdpEndpointI*>(&r);
    if (!p)
    {
        return false;
    }

    if (this == p)
    {
        return true;
    }

    if (_compress != p->_compress)
    {
        return false;
    }

    if (_mcastTtl != p->_mcastTtl)
    {
        return false;
    }

    if (_mcastInterface != p->_mcastInterface)
    {
        return false;
    }

    return true;
}

bool
IceInternal::UdpEndpointI::operator<(const Endpoint& r) const
{
    const auto* p = dynamic_cast<const UdpEndpointI*>(&r);
    if (!p)
    {
        const auto* e = dynamic_cast<const EndpointI*>(&r);
        if (!e)
        {
            return false;
        }
        return type() < e->type();
    }

    if (this == p)
    {
        return false;
    }

    if (!_compress && p->_compress)
    {
        return true;
    }
    else if (p->_compress < _compress)
    {
        return false;
    }

    if (_mcastTtl < p->_mcastTtl)
    {
        return true;
    }
    else if (p->_mcastTtl < _mcastTtl)
    {
        return false;
    }

    if (_mcastInterface < p->_mcastInterface)
    {
        return true;
    }
    else if (p->_mcastInterface < _mcastInterface)
    {
        return false;
    }

    return IPEndpointI::operator<(r);
}

size_t
IceInternal::UdpEndpointI::hash() const noexcept
{
    size_t h = IPEndpointI::hash();
    hashAdd(h, _mcastInterface);
    hashAdd(h, _mcastTtl);
    hashAdd(h, _compress);
    return h;
}

bool
IceInternal::UdpEndpointI::checkOption(const string& option, const string& argument, const string& endpoint)
{
    if (IPEndpointI::checkOption(option, argument, endpoint))
    {
        return true;
    }

    if (option == "-z")
    {
        if (!argument.empty())
        {
            throw ParseException(
                __FILE__,
                __LINE__,
                "unexpected argument '" + argument + "' provided for -z option in endpoint '" + endpoint + "'");
        }
        const_cast<bool&>(_compress) = true;
    }
    else if (option == "-v" || option == "-e")
    {
        if (argument.empty())
        {
            throw ParseException(
                __FILE__,
                __LINE__,
                "no argument provided for " + option + " option in endpoint '" + endpoint + "'");
        }
        try
        {
            uint8_t major, minor;
            IceInternal::stringToMajorMinor(argument, major, minor);
            if (major != 1 || minor != 0)
            {
                _instance->logger()->warning("deprecated udp endpoint option: " + option);
            }
        }
        catch (const ParseException& ex)
        {
            throw ParseException(
                __FILE__,
                __LINE__,
                "invalid version '" + argument + "' in endpoint '" + endpoint + "':\n" + ex.what());
        }
    }
    else if (option == "--interface")
    {
        if (argument.empty())
        {
            throw ParseException(
                __FILE__,
                __LINE__,
                "no argument provided for --interface option in endpoint '" + endpoint + "'");
        }
        const_cast<string&>(_mcastInterface) = argument;
    }
    else if (option == "--ttl")
    {
        if (argument.empty())
        {
            throw ParseException(
                __FILE__,
                __LINE__,
                "no argument provided for --ttl option in endpoint '" + endpoint + "'");
        }
        istringstream p(argument);
        if (!(p >> const_cast<int32_t&>(_mcastTtl)) || !p.eof())
        {
            throw ParseException(
                __FILE__,
                __LINE__,
                "invalid TTL value '" + argument + "' in endpoint '" + endpoint + "'");
        }
    }
    else
    {
        return false;
    }
    return true;
}

ConnectorPtr
IceInternal::UdpEndpointI::createConnector(const Address& address, const NetworkProxyPtr&) const
{
    return make_shared<UdpConnector>(_instance, address, _sourceAddr, _mcastInterface, _mcastTtl, _connectionId);
}

IPEndpointIPtr
IceInternal::UdpEndpointI::createEndpoint(const string& host, int port, const string& connectionId) const
{
    return make_shared<
        UdpEndpointI>(_instance, host, port, _sourceAddr, _mcastInterface, _mcastTtl, connectionId, _compress);
}

IceInternal::UdpEndpointFactory::UdpEndpointFactory(ProtocolInstancePtr instance) : _instance(std::move(instance)) {}

IceInternal::UdpEndpointFactory::~UdpEndpointFactory() = default;

int16_t
IceInternal::UdpEndpointFactory::type() const
{
    return _instance->type();
}

string
IceInternal::UdpEndpointFactory::protocol() const
{
    return _instance->protocol();
}

EndpointIPtr
IceInternal::UdpEndpointFactory::create(vector<string>& args, bool oaEndpoint) const
{
    auto endpt = make_shared<UdpEndpointI>(_instance);
    endpt->initWithOptions(args, oaEndpoint);
    return endpt;
}

EndpointIPtr
IceInternal::UdpEndpointFactory::read(InputStream* s) const
{
    return make_shared<UdpEndpointI>(_instance, s);
}

EndpointFactoryPtr
IceInternal::UdpEndpointFactory::clone(const ProtocolInstancePtr& instance) const
{
    return make_shared<UdpEndpointFactory>(instance);
}
