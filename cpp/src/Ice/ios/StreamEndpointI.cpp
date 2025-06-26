// Copyright (c) ZeroC, Inc.

#include "Ice/Config.h"

#if TARGET_OS_IPHONE != 0

#    include "../EndpointFactoryManager.h"
#    include "../HashUtil.h"
#    include "../Network.h"
#    include "../NetworkProxy.h"
#    include "Ice/Communicator.h"
#    include "Ice/InputStream.h"
#    include "Ice/LocalExceptions.h"
#    include "Ice/OutputStream.h"
#    include "Ice/Properties.h"
#    include "Ice/StringUtil.h"
#    include "StreamAcceptor.h"
#    include "StreamConnector.h"
#    include "StreamEndpointI.h"

#    include <CoreFoundation/CoreFoundation.h>

#    include <fstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    const char* const tcpPluginName = "IceTCP";

    Plugin* createIceTCP(const CommunicatorPtr& com, const string& name, const StringSeq&)
    {
        string pluginName{tcpPluginName};

        if (name != pluginName)
        {
            throw PluginInitializationException{
                __FILE__,
                __LINE__,
                "the TCP plug-in must be named '" + pluginName + "'"};
        }

        IceObjC::InstancePtr tcpInstance = make_shared<IceObjC::Instance>(com, TCPEndpointType, "tcp", false);
        return new EndpointFactoryPlugin(com, make_shared<IceObjC::StreamEndpointFactory>(tcpInstance));
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

#    if TARGET_IPHONE_SIMULATOR == 0
namespace
{
    inline CFStringRef toCFString(const string& s)
    {
        return CFStringCreateWithCString(nullptr, s.c_str(), kCFStringEncodingUTF8);
    }
}
#    endif

IceObjC::Instance::Instance(const Ice::CommunicatorPtr& communicator, int16_t type, const string& protocol, bool secure)
    : ProtocolInstance(communicator, type, protocol, secure),
      _communicator(communicator),
      _proxySettings(nullptr)
{
    const Ice::PropertiesPtr properties = communicator->getProperties();

    //
    // Proxy settings
    //
    _proxyHost = properties->getIceProperty("Ice.SOCKSProxyHost");
    if (!_proxyHost.empty())
    {
#    if TARGET_IPHONE_SIMULATOR != 0
        throw Ice::FeatureNotSupportedException(__FILE__, __LINE__, "SOCKS proxy not supported");
#    else
        _proxySettings.reset(
            CFDictionaryCreateMutable(0, 3, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));

        _proxyPort = properties->getIcePropertyAsInt("Ice.SOCKSProxyPort");

        UniqueRef<CFStringRef> host(toCFString(_proxyHost));
        CFDictionarySetValue(_proxySettings.get(), kCFStreamPropertySOCKSProxyHost, host.get());

        UniqueRef<CFNumberRef> port(CFNumberCreate(0, kCFNumberSInt32Type, &_proxyPort));
        CFDictionarySetValue(_proxySettings.get(), kCFStreamPropertySOCKSProxyPort, port.get());

        CFDictionarySetValue(_proxySettings.get(), kCFStreamPropertySOCKSVersion, kCFStreamSocketSOCKSVersion4);
#    endif
    }
}

IceObjC::Instance::~Instance() = default;

void
IceObjC::Instance::setupStreams(
    CFReadStreamRef readStream,
    CFWriteStreamRef writeStream,
    bool server,
    const string& /*host*/) const
{
    if (!server && _proxySettings)
    {
        if (!CFReadStreamSetProperty(readStream, kCFStreamPropertySOCKSProxy, _proxySettings.get()) ||
            !CFWriteStreamSetProperty(writeStream, kCFStreamPropertySOCKSProxy, _proxySettings.get()))
        {
            throw Ice::SocketException{
                __FILE__,
                __LINE__,
                "failed to set the SOCKS proxy property on CFNetwork streams"};
        }
    }
}

CommunicatorPtr
IceObjC::Instance::communicator()
{
    Ice::CommunicatorPtr communicator = _communicator.lock();
    if (!communicator)
    {
        throw Ice::CommunicatorDestroyedException{__FILE__, __LINE__};
    }
    return communicator;
}

IceObjC::StreamEndpointI::StreamEndpointI(
    const InstancePtr& instance,
    const string& host,
    int32_t port,
    const Address& sourceAddr,
    int32_t timeout,
    const string& connectionId,
    bool compress)
    : IceInternal::IPEndpointI(instance, host, port, sourceAddr, connectionId),
      _streamInstance(instance),
      _timeout(timeout),
      _compress(compress)
{
}

IceObjC::StreamEndpointI::StreamEndpointI(const InstancePtr& instance)
    : IceInternal::IPEndpointI(instance),
      _streamInstance(instance),
      _timeout(defaultTimeout),
      _compress(false)
{
}

IceObjC::StreamEndpointI::StreamEndpointI(const InstancePtr& instance, Ice::InputStream* s)
    : IPEndpointI(instance, s),
      _streamInstance(instance),
      _timeout(-1),
      _compress(false)
{
    s->read(const_cast<int32_t&>(_timeout));
    s->read(const_cast<bool&>(_compress));
}

EndpointInfoPtr
IceObjC::StreamEndpointI::getInfo() const noexcept
{
    return make_shared<Ice::TCPEndpointInfo>(_compress, _host, _port, inetAddrToString(_sourceAddr), type(), secure());
}

int32_t
IceObjC::StreamEndpointI::timeout() const
{
    return _timeout;
}

EndpointIPtr
IceObjC::StreamEndpointI::timeout(int32_t t) const
{
    if (t == _timeout)
    {
        return const_cast<StreamEndpointI*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<StreamEndpointI>(_streamInstance, _host, _port, _sourceAddr, t, _connectionId, _compress);
    }
}

bool
IceObjC::StreamEndpointI::compress() const
{
    return _compress;
}

EndpointIPtr
IceObjC::StreamEndpointI::compress(bool compress) const
{
    if (compress == _compress)
    {
        return const_cast<StreamEndpointI*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<StreamEndpointI>(
            _streamInstance,
            _host,
            _port,
            _sourceAddr,
            _timeout,
            _connectionId,
            compress);
    }
}

bool
IceObjC::StreamEndpointI::datagram() const
{
    return false;
}

bool
IceObjC::StreamEndpointI::secure() const
{
    return _streamInstance->secure();
}

shared_ptr<IceInternal::EndpointI>
IceObjC::StreamEndpointI::toPublishedEndpoint(string publishedHost) const
{
    // A server endpoint can't have a source address or connection ID.
    assert(!isAddressValid(_sourceAddr) && _connectionId.empty());

    if (publishedHost.empty())
    {
        return const_cast<StreamEndpointI*>(this)->shared_from_this();
    }
    else
    {
        return make_shared<StreamEndpointI>(
            _streamInstance,
            publishedHost,
            _port,
            _sourceAddr,
            _timeout,
            _connectionId,
            _compress);
    }
}

void
IceObjC::StreamEndpointI::connectorsAsync(
    function<void(vector<IceInternal::ConnectorPtr>)> response,
    function<void(exception_ptr)>) const
{
    vector<ConnectorPtr> connectors;
    connectors.emplace_back(make_shared<StreamConnector>(_streamInstance, _host, _port, _timeout, _connectionId));
    response(std::move(connectors));
}

TransceiverPtr
IceObjC::StreamEndpointI::transceiver() const
{
    return 0;
}

AcceptorPtr
IceObjC::StreamEndpointI::acceptor(const string&, const optional<SSL::ServerAuthenticationOptions>&) const
{
    return make_shared<StreamAcceptor>(
        const_cast<StreamEndpointI*>(this)->shared_from_this(),
        _streamInstance,
        _host,
        _port);
}

IceObjC::StreamEndpointIPtr
IceObjC::StreamEndpointI::endpoint(const StreamAcceptorPtr& a) const
{
    int port = a->effectivePort();
    if (port == _port)
    {
        return dynamic_pointer_cast<StreamEndpointI>(const_cast<StreamEndpointI*>(this)->shared_from_this());
    }
    else
    {
        return make_shared<StreamEndpointI>(
            _streamInstance,
            _host,
            port,
            _sourceAddr,
            _timeout,
            _connectionId,
            _compress);
    }
}

string
IceObjC::StreamEndpointI::options() const
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
IceObjC::StreamEndpointI::operator==(const Endpoint& r) const
{
    if (!IPEndpointI::operator==(r))
    {
        return false;
    }

    const StreamEndpointI* p = dynamic_cast<const StreamEndpointI*>(&r);
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
IceObjC::StreamEndpointI::operator<(const Endpoint& r) const
{
    const StreamEndpointI* p = dynamic_cast<const StreamEndpointI*>(&r);
    if (!p)
    {
        const IceInternal::EndpointI* e = dynamic_cast<const IceInternal::EndpointI*>(&r);
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

void
IceObjC::StreamEndpointI::streamWriteImpl(Ice::OutputStream* s) const
{
    IPEndpointI::streamWriteImpl(s);
    s->write(_timeout);
    s->write(_compress);
}

size_t
IceObjC::StreamEndpointI::hash() const noexcept
{
    size_t h = IPEndpointI::hash();
    hashAdd(h, _timeout);
    hashAdd(h, _compress);
    return h;
}

bool
IceObjC::StreamEndpointI::checkOption(const string& option, const string& argument, const string& endpoint)
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
IceObjC::StreamEndpointI::createConnector(const Address& /*address*/, const NetworkProxyPtr& /*proxy*/) const
{
    assert(false);
    return nullptr;
}

IPEndpointIPtr
IceObjC::StreamEndpointI::createEndpoint(const string& host, int port, const string& connectionId) const
{
    return make_shared<StreamEndpointI>(_streamInstance, host, port, _sourceAddr, _timeout, connectionId, _compress);
}

IceObjC::StreamEndpointFactory::StreamEndpointFactory(const InstancePtr& instance) : _instance(instance) {}

int16_t
IceObjC::StreamEndpointFactory::type() const
{
    return _instance->type();
}

string
IceObjC::StreamEndpointFactory::protocol() const
{
    return _instance->protocol();
}

EndpointIPtr
IceObjC::StreamEndpointFactory::create(vector<string>& args, bool oaEndpoint) const
{
    IPEndpointIPtr endpt = make_shared<StreamEndpointI>(_instance);
    endpt->initWithOptions(args, oaEndpoint);
    return endpt;
}

EndpointIPtr
IceObjC::StreamEndpointFactory::read(Ice::InputStream* s) const
{
    return make_shared<StreamEndpointI>(_instance, s);
}

EndpointFactoryPtr
IceObjC::StreamEndpointFactory::clone(const ProtocolInstancePtr& instance) const
{
    return make_shared<StreamEndpointFactory>(make_shared<IceObjC::Instance>(
        _instance->communicator(),
        instance->type(),
        instance->protocol(),
        instance->secure()));
}
#endif
