// Copyright (c) ZeroC, Inc.

#include "../Network.h"

#if defined(ICE_USE_NETWORK_FRAMEWORK)

#    include "Ice/LocalExceptions.h"
#    include "../ProtocolInstance.h"
#    include "NetworkFrameworkUdpConnector.h"
#    include "NetworkFrameworkUdpTransceiver.h"

#    include <Network/Network.h>
#    include <utility>

using namespace std;
using namespace Ice;
using namespace IceInternal;

TransceiverPtr
IceInternal::NetworkFrameworkUdpConnector::connect()
{
    //
    // Create the Network.framework endpoint from host and port.
    //
    nw_endpoint_t endpoint = nw_endpoint_create_host(_host.c_str(), to_string(_port).c_str());
    if (!endpoint)
    {
        throw ConnectFailedException(__FILE__, __LINE__, 0);
    }

    //
    // Create UDP parameters (no security).
    //
    nw_parameters_t parameters =
        nw_parameters_create_secure_udp(NW_PARAMETERS_DISABLE_PROTOCOL, NW_PARAMETERS_DEFAULT_CONFIGURATION);
    if (!parameters)
    {
        nw_release(endpoint);
        throw ConnectFailedException(__FILE__, __LINE__, 0);
    }

    // Enable IP fragmentation for large UDP datagrams. Network.framework disables fragmentation by default,
    // but BSD sockets allow it, so we need to match that behavior.
    nw_protocol_stack_t stack = nw_parameters_copy_default_protocol_stack(parameters);
    nw_protocol_options_t ipOptions = nw_protocol_stack_copy_internet_protocol(stack);
    nw_ip_options_set_disable_fragmentation(ipOptions, false);
    nw_release(ipOptions);
    nw_release(stack);

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
        auto transceiver = make_shared<NetworkFrameworkUdpTransceiver>(_instance, connection);
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
IceInternal::NetworkFrameworkUdpConnector::type() const
{
    return _instance->type();
}

string
IceInternal::NetworkFrameworkUdpConnector::toString() const
{
    return _host + ":" + to_string(_port);
}

bool
IceInternal::NetworkFrameworkUdpConnector::operator==(const Connector& r) const
{
    const auto* p = dynamic_cast<const NetworkFrameworkUdpConnector*>(&r);
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

    if (_mcastTtl != p->_mcastTtl)
    {
        return false;
    }

    if (_mcastInterface != p->_mcastInterface)
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
IceInternal::NetworkFrameworkUdpConnector::operator<(const Connector& r) const
{
    const auto* p = dynamic_cast<const NetworkFrameworkUdpConnector*>(&r);
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

    if (_connectionId < p->_connectionId)
    {
        return true;
    }
    else if (p->_connectionId < _connectionId)
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

    return compareAddress(_sourceAddr, p->_sourceAddr) < 0;
}

IceInternal::NetworkFrameworkUdpConnector::NetworkFrameworkUdpConnector(
    ProtocolInstancePtr instance,
    const string& host,
    int32_t port,
    const Address& sourceAddr,
    const string& mcastInterface,
    int32_t mcastTtl,
    string connectionId)
    : _instance(std::move(instance)),
      _host(host),
      _port(port),
      _sourceAddr(sourceAddr),
      _mcastInterface(mcastInterface),
      _mcastTtl(mcastTtl),
      _connectionId(std::move(connectionId))
{
}

IceInternal::NetworkFrameworkUdpConnector::~NetworkFrameworkUdpConnector() = default;

#endif
