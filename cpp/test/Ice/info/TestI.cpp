// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;
using namespace Test;

namespace
{
    IPEndpointInfoPtr getIPEndpointInfo(const EndpointInfoPtr& info)
    {
        for (EndpointInfoPtr p = info; p; p = p->underlying)
        {
            IPEndpointInfoPtr ipInfo = dynamic_pointer_cast<IPEndpointInfo>(p);
            if (ipInfo)
            {
                return ipInfo;
            }
        }
        return nullptr;
    }

    IPConnectionInfoPtr getIPConnectionInfo(const ConnectionInfoPtr& info)
    {
        for (ConnectionInfoPtr p = info; p; p = p->underlying)
        {
            IPConnectionInfoPtr ipInfo = dynamic_pointer_cast<IPConnectionInfo>(p);
            if (ipInfo)
            {
                return ipInfo;
            }
        }
        return nullptr;
    }
}

void
TestI::shutdown(const Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

Context
TestI::getEndpointInfoAsContext(const Current& c)
{
    ostringstream os;

    Context ctx;
    EndpointInfoPtr info = c.con->getEndpoint()->getInfo();
    os << info->timeout;
    ctx["timeout"] = os.str();
    ctx["compress"] = info->compress ? "true" : "false";
    ctx["datagram"] = info->datagram() ? "true" : "false";
    ctx["secure"] = info->datagram() ? "true" : "false";
    os.str("");
    os << info->type();
    ctx["type"] = os.str();

    IPEndpointInfoPtr ipinfo = getIPEndpointInfo(info);
    test(ipinfo);
    ctx["host"] = ipinfo->host;
    os.str("");
    os << ipinfo->port;
    ctx["port"] = os.str();

    if (dynamic_pointer_cast<UDPEndpointInfo>(ipinfo))
    {
        UDPEndpointInfoPtr udp = dynamic_pointer_cast<UDPEndpointInfo>(ipinfo);
        ctx["mcastInterface"] = udp->mcastInterface;
        ctx["mcastTtl"] = static_cast<char>(udp->mcastTtl);
    }

    return ctx;
}

Context
TestI::getConnectionInfoAsContext(const Current& c)
{
    Context ctx;
    ConnectionInfoPtr info = c.con->getInfo();
    ctx["adapterName"] = info->adapterName;
    ctx["incoming"] = info->incoming ? "true" : "false";
    ostringstream os;

    IPConnectionInfoPtr ipinfo = getIPConnectionInfo(info);
    test(ipinfo);
    ctx["localAddress"] = ipinfo->localAddress;
    os.str("");
    os << ipinfo->localPort;
    ctx["localPort"] = os.str();
    ctx["remoteAddress"] = ipinfo->remoteAddress;
    os.str("");
    os << ipinfo->remotePort;
    ctx["remotePort"] = os.str();

    WSConnectionInfoPtr wsinfo = dynamic_pointer_cast<WSConnectionInfo>(info);
    if (wsinfo)
    {
        for (const auto& header : wsinfo->headers)
        {
            ctx["ws." + header.first] = header.second;
        }
    }

    return ctx;
}
