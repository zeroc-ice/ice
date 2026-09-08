// Copyright (c) ZeroC, Inc.

#include "../Network.h"

#if defined(ICE_USE_NETWORK_FRAMEWORK)

#    include "Ice/LocalExceptions.h"
#    include "Ice/LoggerUtil.h"
#    include "../Network.h"
#    include "../NetworkProxy.h"
#    include "../ProtocolInstance.h"
#    include "NetworkFrameworkConnector.h"
#    include "NetworkFrameworkTransceiver.h"

#    include <Network/Network.h>
#    include <utility>

using namespace std;
using namespace Ice;
using namespace IceInternal;

TransceiverPtr
IceInternal::NetworkFrameworkConnector::connect()
{
    //
    // Create the Network.framework endpoint. With a network proxy, connect to the proxy: the transceiver performs
    // the proxy handshake (SOCKS4 or HTTP CONNECT) with the destination address before Ice uses the connection.
    //
    string host = _host;
    int port = _port;
    if (_proxy)
    {
        addrToAddressAndPort(_proxy->getAddress(), host, port);
    }
    nw_endpoint_t endpoint = nw_endpoint_create_host(host.c_str(), to_string(port).c_str());
    if (!endpoint)
    {
        throw ConnectFailedException(__FILE__, __LINE__, 0);
    }

    //
    // Create TCP parameters. TLS is disabled (plaintext TCP).
    // For SSL/TLS, a separate connector will configure tls_options.
    //
    nw_parameters_t parameters = nw_parameters_create_secure_tcp(
        NW_PARAMETERS_DISABLE_PROTOCOL, // No TLS for plain TCP.
        NW_PARAMETERS_DEFAULT_CONFIGURATION);

    if (!parameters)
    {
        nw_release(endpoint);
        throw ConnectFailedException(__FILE__, __LINE__, 0);
    }

    //
    // Bind the connection to the source address configured on the endpoint (--sourceAddress), if any.
    //
    if (isAddressValid(_sourceAddr))
    {
        nw_endpoint_t localEndpoint = nw_endpoint_create_address(&_sourceAddr.sa);
        if (!localEndpoint)
        {
            nw_release(parameters);
            nw_release(endpoint);
            throw ConnectFailedException(__FILE__, __LINE__, 0);
        }
        nw_parameters_set_local_endpoint(parameters, localEndpoint);
        nw_release(localEndpoint);
    }

    //
    // Create the Network.framework connection.
    //
    nw_connection_t connection = nw_connection_create(endpoint, parameters);
    nw_release(endpoint);
    nw_release(parameters);

    if (!connection)
    {
        throw ConnectFailedException(__FILE__, __LINE__, 0);
    }

    try
    {
        auto transceiver = make_shared<NetworkFrameworkTransceiver>(_instance, connection, false, _proxy, _addr);
        nw_release(connection); // The transceiver retains its own reference.
        return transceiver;
    }
    catch (...)
    {
        nw_release(connection);
        throw;
    }
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
