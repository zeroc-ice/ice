// Copyright (c) ZeroC, Inc.

#include "../Network.h"

#if defined(ICE_USE_NETWORK_FRAMEWORK)

#    include "../Network.h"
#    include "../NetworkProxy.h"
#    include "../ProtocolInstance.h"
#    include "Ice/LocalExceptions.h"
#    include "NetworkFrameworkConnector.h"
#    include "NetworkFrameworkTransceiver.h"

#    include <cassert>
#    include <utility>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    NetworkRef<nw_endpoint_t> createEndpoint(const string& host, int port)
    {
        auto endpoint =
            NetworkRef<nw_endpoint_t>::adopt(nw_endpoint_create_host(host.c_str(), to_string(port).c_str()));
        if (!endpoint)
        {
            throw ConnectFailedException(__FILE__, __LINE__, 0);
        }
        return endpoint;
    }

    // Configures the proxy Network.framework connects through, see NetworkFrameworkConnector::Protocol::TLS. The
    // proxy is explicitly configured: the connection never falls back to a direct connection when it cannot be used.
    void setProxy(nw_parameters_t parameters, const NetworkProxy& proxy)
    {
        if (__builtin_available(macOS 14.0, iOS 17.0, tvOS 17.0, watchOS 10.0, *))
        {
            string host;
            int port;
            addrToAddressAndPort(proxy.getAddress(), host, port);
            NetworkRef<nw_endpoint_t> endpoint = createEndpoint(host, port);
            auto proxyConfig = NetworkRef<nw_proxy_config_t>::adopt(
                proxy.getName() == "HTTP" ? nw_proxy_config_create_http_connect(endpoint.get(), nullptr)
                                          : nw_proxy_config_create_socksv5(endpoint.get()));
            nw_proxy_config_set_failover_allowed(proxyConfig.get(), false);
            auto privacyContext =
                NetworkRef<nw_privacy_context_t>::adopt(nw_privacy_context_create("com.zeroc.ice.ssl-proxy"));
            nw_privacy_context_add_proxy(privacyContext.get(), proxyConfig.get());
            nw_parameters_set_privacy_context(parameters, privacyContext.get());
        }
        else
        {
            throw FeatureNotSupportedException(
                __FILE__,
                __LINE__,
                "SSL connections through a network proxy require macOS 14 or iOS 17");
        }
    }
}

TransceiverPtr
IceInternal::NetworkFrameworkConnector::connect()
{
    auto parameters = NetworkRef<nw_parameters_t>::adopt(
        nw_parameters_create_secure_tcp(NW_PARAMETERS_DISABLE_PROTOCOL, NW_PARAMETERS_DEFAULT_CONFIGURATION));
    if (!parameters)
    {
        throw ConnectFailedException(__FILE__, __LINE__, 0);
    }
    return connect(Protocol::TCP, std::move(parameters));
}

shared_ptr<NetworkFrameworkTransceiver>
IceInternal::NetworkFrameworkConnector::connect(Protocol protocol, NetworkRef<nw_parameters_t> parameters) const
{
    assert(parameters);

    // The destination of the connection, and how it reaches it through the proxy: see Protocol.
    string host = _host;
    int port = _port;
    if (_proxy)
    {
        switch (protocol)
        {
            case Protocol::TCP:
                addrToAddressAndPort(_proxy->getAddress(), host, port); // The transceiver performs the handshake.
                break;
            case Protocol::TLS:
                setProxy(parameters.get(), *_proxy); // Network.framework performs the handshake.
                break;
        }
    }
    NetworkRef<nw_endpoint_t> endpoint = createEndpoint(host, port);

    // The source address configured on the endpoint (--sourceAddress), if any.
    if (isAddressValid(_sourceAddr))
    {
        auto localEndpoint = NetworkRef<nw_endpoint_t>::adopt(nw_endpoint_create_address(&_sourceAddr.sa));
        if (!localEndpoint)
        {
            throw ConnectFailedException(__FILE__, __LINE__, 0);
        }
        nw_parameters_set_local_endpoint(parameters.get(), localEndpoint.get());
    }

    auto connection = NetworkRef<nw_connection_t>::adopt(nw_connection_create(endpoint.get(), parameters.get()));
    if (!connection)
    {
        throw ConnectFailedException(__FILE__, __LINE__, 0);
    }

    // The transceiver owns the connection. With a proxy, it describes the connection with the destination address.
    return make_shared<NetworkFrameworkTransceiver>(
        _instance,
        std::move(connection),
        protocol == Protocol::TLS,
        _proxy,
        _addr);
}

int16_t
IceInternal::NetworkFrameworkConnector::type() const
{
    return _instance->type();
}

string
IceInternal::NetworkFrameworkConnector::toString() const
{
    return _host + ":" + to_string(_port);
}

bool
IceInternal::NetworkFrameworkConnector::operator==(const Connector& r) const
{
    const auto* p = dynamic_cast<const NetworkFrameworkConnector*>(&r);
    if (!p)
    {
        return false;
    }

    if (_host != p->_host)
    {
        return false;
    }

    if (_port != p->_port)
    {
        return false;
    }

    if (_timeout != p->_timeout)
    {
        return false;
    }

    if (compareAddress(_sourceAddr, p->_sourceAddr) != 0)
    {
        return false;
    }

    if (_connectionId != p->_connectionId)
    {
        return false;
    }

    return true;
}

bool
IceInternal::NetworkFrameworkConnector::operator<(const Connector& r) const
{
    const auto* p = dynamic_cast<const NetworkFrameworkConnector*>(&r);
    if (!p)
    {
        return type() < r.type();
    }

    if (_host < p->_host)
    {
        return true;
    }
    else if (p->_host < _host)
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

    int rc = compareAddress(_sourceAddr, p->_sourceAddr);
    if (rc < 0)
    {
        return true;
    }
    else if (rc > 0)
    {
        return false;
    }

    return _connectionId < p->_connectionId;
}

IceInternal::NetworkFrameworkConnector::NetworkFrameworkConnector(
    ProtocolInstancePtr instance,
    const Address& addr,
    NetworkProxyPtr proxy,
    const Address& sourceAddr,
    int32_t timeout,
    string connectionId)
    : _instance(std::move(instance)),
      _addr(addr),
      _proxy(std::move(proxy)),
      _port(0),
      _sourceAddr(sourceAddr),
      _timeout(timeout),
      _connectionId(std::move(connectionId))
{
    addrToAddressAndPort(_addr, _host, _port);
}

IceInternal::NetworkFrameworkConnector::~NetworkFrameworkConnector() = default;

#endif
