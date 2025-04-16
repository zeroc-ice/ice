// Copyright (c) ZeroC, Inc.

#include "Ice/Config.h"

#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0

#    include "HashUtil.h"
#    include "Ice/InputStream.h"
#    include "Ice/LocalExceptions.h"
#    include "Ice/OutputStream.h"
#    include "Network.h"
#    include "ProtocolInstance.h"
#    include "TcpAcceptor.h"
#    include "TcpConnector.h"
#    include "TcpEndpointI.h"

#    include "TcpTransceiver.h"
#    include <utility>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    const char* const tcpPluginName = "IceTCP";

    Plugin* createIceTCP(const CommunicatorPtr& c, const string& name, const StringSeq&)
    {
        string pluginName{tcpPluginName};

        if (name != pluginName)
        {
            throw PluginInitializationException{
                __FILE__,
                __LINE__,
                "the TCP plug-in must be named '" + pluginName + "'"};
        }

        return new EndpointFactoryPlugin(
            c,
            make_shared<TcpEndpointFactory>(make_shared<ProtocolInstance>(c, TCPEndpointType, "tcp", false)));
    }
}

Ice::PluginFactory
Ice::tcpPluginFactory()
{
    return {tcpPluginName, createIceTCP};
}

namespace
{
    const int32_t defaultTimeout = 60000; // 60,000 ms.
}

IceInternal::TcpEndpointI::TcpEndpointI(
    const ProtocolInstancePtr& instance,
    const string& host,
    int32_t port,
    const Address& sourceAddr,
    int32_t timeout,
    const string& connectionId,
    bool compress)
    : IPEndpointI(instance, host, port, sourceAddr, connectionId),
      _timeout(timeout),
      _compress(compress)
{
}

IceInternal::TcpEndpointI::TcpEndpointI(const ProtocolInstancePtr& instance)
    : IPEndpointI(instance),
      // This timeout is not used in Ice 3.8 and greater.
      _timeout(defaultTimeout),
      _compress(false)
{
}

IceInternal::TcpEndpointI::TcpEndpointI(const ProtocolInstancePtr& instance, InputStream* s)
    : IPEndpointI(instance, s),
      _timeout(-1),
      _compress(false)
{
    s->read(const_cast<int32_t&>(_timeout));
    s->read(const_cast<bool&>(_compress));
}

void
IceInternal::TcpEndpointI::streamWriteImpl(OutputStream* s) const
{
    IPEndpointI::streamWriteImpl(s);
    s->write(_timeout);
    s->write(_compress);
}

EndpointInfoPtr
IceInternal::TcpEndpointI::getInfo() const noexcept
{
    return make_shared<TCPEndpointInfo>(
        _timeout,
        _compress,
        _host,
        _port,
        inetAddrToString(_sourceAddr),
        type(),
        secure());
}

int32_t
IceInternal::TcpEndpointI::timeout() const
{
    return _timeout;
}

EndpointIPtr
IceInternal::TcpEndpointI::timeout(int32_t timeout) const
{
    if (timeout == _timeout)
    {
        return const_cast<TcpEndpointI*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<TcpEndpointI>(_instance, _host, _port, _sourceAddr, timeout, _connectionId, _compress);
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
    if (compress == _compress)
    {
        return const_cast<TcpEndpointI*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<TcpEndpointI>(_instance, _host, _port, _sourceAddr, _timeout, _connectionId, compress);
    }
}

bool
IceInternal::TcpEndpointI::datagram() const
{
    return false;
}

shared_ptr<EndpointI>
IceInternal::TcpEndpointI::toPublishedEndpoint(string publishedHost) const
{
    // A server endpoint can't have a source address or connection ID.
    assert(!isAddressValid(_sourceAddr) && _connectionId.empty());

    if (publishedHost.empty())
    {
        return const_cast<TcpEndpointI*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<TcpEndpointI>(_instance, publishedHost, _port, Address{}, _timeout, "", _compress);
    }
}

TransceiverPtr
IceInternal::TcpEndpointI::transceiver() const
{
    return nullptr;
}

AcceptorPtr
IceInternal::TcpEndpointI::acceptor(const string&, const optional<Ice::SSL::ServerAuthenticationOptions>&) const
{
    return make_shared<TcpAcceptor>(
        dynamic_pointer_cast<TcpEndpointI>(const_cast<TcpEndpointI*>(this)->shared_from_this()),
        _instance,
        _host,
        _port);
}

TcpEndpointIPtr
IceInternal::TcpEndpointI::endpoint(const TcpAcceptorPtr& acceptor) const
{
    int port = acceptor->effectivePort();
    if (_port == port)
    {
        return dynamic_pointer_cast<TcpEndpointI>(const_cast<TcpEndpointI*>(this)->shared_from_this());
    }
    else
    {
        return make_shared<TcpEndpointI>(_instance, _host, port, _sourceAddr, _timeout, _connectionId, _compress);
    }
}

string
IceInternal::TcpEndpointI::options() const
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

    // -t is totally ignored by Ice as of Ice 3.8. To avoid confusion, we don't output it when set to the default value.
    if (_timeout != defaultTimeout)
    {
        if (_timeout == -1)
        {
            s << " -t infinite";
        }
        else
        {
            s << " -t " << to_string(_timeout);
        }
    }

    if (_compress)
    {
        s << " -z";
    }

    return s.str();
}

bool
IceInternal::TcpEndpointI::operator==(const Endpoint& r) const
{
    if (!IPEndpointI::operator==(r))
    {
        return false;
    }

    const auto* p = dynamic_cast<const TcpEndpointI*>(&r);
    if (!p)
    {
        return false;
    }

    if (this == p)
    {
        return true;
    }

    if (_timeout != p->_timeout)
    {
        return false;
    }

    if (_compress != p->_compress)
    {
        return false;
    }
    return true;
}

bool
IceInternal::TcpEndpointI::operator<(const Endpoint& r) const
{
    const auto* p = dynamic_cast<const TcpEndpointI*>(&r);
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

    if (_timeout < p->_timeout)
    {
        return true;
    }
    else if (p->_timeout < _timeout)
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

    return IPEndpointI::operator<(r);
}

size_t
IceInternal::TcpEndpointI::hash() const noexcept
{
    size_t h = IPEndpointI::hash();
    hashAdd(h, _timeout);
    hashAdd(h, _compress);
    return h;
}

bool
IceInternal::TcpEndpointI::checkOption(const string& option, const string& argument, const string& endpoint)
{
    if (IPEndpointI::checkOption(option, argument, endpoint))
    {
        return true;
    }

    switch (option[1])
    {
        case 't':
        {
            if (argument.empty())
            {
                throw ParseException(
                    __FILE__,
                    __LINE__,
                    "no argument provided for -t option in endpoint '" + endpoint + "'");
            }

            if (argument == "infinite")
            {
                const_cast<int32_t&>(_timeout) = -1;
            }
            else
            {
                istringstream t(argument);
                if (!(t >> const_cast<int32_t&>(_timeout)) || !t.eof() || _timeout < 1)
                {
                    throw ParseException(
                        __FILE__,
                        __LINE__,
                        "invalid timeout value '" + argument + "' in endpoint '" + endpoint + "'");
                }
            }
            return true;
        }

        case 'z':
        {
            if (!argument.empty())
            {
                throw ParseException(
                    __FILE__,
                    __LINE__,
                    "unexpected argument '" + argument + "' provided for -z option in endpoint '" + endpoint + "'");
            }
            const_cast<bool&>(_compress) = true;
            return true;
        }

        default:
        {
            return false;
        }
    }
}

ConnectorPtr
IceInternal::TcpEndpointI::createConnector(const Address& address, const NetworkProxyPtr& proxy) const
{
    return make_shared<TcpConnector>(_instance, address, proxy, _sourceAddr, _timeout, _connectionId);
}

IPEndpointIPtr
IceInternal::TcpEndpointI::createEndpoint(const string& host, int port, const string& connectionId) const
{
    return make_shared<TcpEndpointI>(_instance, host, port, _sourceAddr, _timeout, connectionId, _compress);
}

IceInternal::TcpEndpointFactory::TcpEndpointFactory(ProtocolInstancePtr instance) : _instance(std::move(instance)) {}

IceInternal::TcpEndpointFactory::~TcpEndpointFactory() = default;

int16_t
IceInternal::TcpEndpointFactory::type() const
{
    return _instance->type();
}

string
IceInternal::TcpEndpointFactory::protocol() const
{
    return _instance->protocol();
}

EndpointIPtr
IceInternal::TcpEndpointFactory::create(vector<string>& args, bool oaEndpoint) const
{
    IPEndpointIPtr endpt = make_shared<TcpEndpointI>(_instance);
    endpt->initWithOptions(args, oaEndpoint);
    return endpt;
}

EndpointIPtr
IceInternal::TcpEndpointFactory::read(InputStream* s) const
{
    return make_shared<TcpEndpointI>(_instance, s);
}

EndpointFactoryPtr
IceInternal::TcpEndpointFactory::clone(const ProtocolInstancePtr& instance) const
{
    return make_shared<TcpEndpointFactory>(instance);
}
#endif
