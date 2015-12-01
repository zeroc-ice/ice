// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceSSL/IceSSL.h>
#include <TestCommon.h>
#include <TestI.h>

using namespace std;
using namespace Ice;
using namespace Test;

void
TestI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

Ice::Context
TestI::getEndpointInfoAsContext(const Ice::Current& c)
{
    ostringstream os;

    Ice::Context ctx;
    Ice::EndpointInfoPtr info = c.con->getEndpoint()->getInfo();
    os << info->timeout;
    ctx["timeout"] = os.str();
    ctx["compress"] = info->compress ? "true" : "false";
    ctx["datagram"] = info->datagram() ? "true" : "false";
    ctx["secure"] = info->datagram() ? "true" : "false";
    os.str("");
    os << info->type();
    ctx["type"] = os.str();

    Ice::IPEndpointInfoPtr ipinfo = ICE_DYNAMIC_CAST(Ice::IPEndpointInfo, info);
    test(info);
    ctx["host"] = ipinfo->host;
    os.str("");
    os << ipinfo->port;
    ctx["port"] = os.str();

    if(ICE_DYNAMIC_CAST(Ice::UDPEndpointInfo, ipinfo))
    {
        Ice::UDPEndpointInfoPtr udp = ICE_DYNAMIC_CAST(Ice::UDPEndpointInfo, ipinfo);
        ctx["mcastInterface"] = udp->mcastInterface;
        ctx["mcastTtl"] = udp->mcastTtl;
    }

    return ctx;
}

Ice::Context
TestI::getConnectionInfoAsContext(const Ice::Current& c)
{
    Ice::Context ctx;
    Ice::ConnectionInfoPtr info = c.con->getInfo();
    ctx["adapterName"] = info->adapterName;
    ctx["incoming"] = info->incoming ? "true" : "false";
    ostringstream os;

    Ice::IPConnectionInfoPtr ipinfo = ICE_DYNAMIC_CAST(Ice::IPConnectionInfo, info);
    test(ipinfo);
    ctx["localAddress"] = ipinfo->localAddress;
    os.str("");
    os << ipinfo->localPort;
    ctx["localPort"] = os.str();
    ctx["remoteAddress"] = ipinfo->remoteAddress;
    os.str("");
    os << ipinfo->remotePort;
    ctx["remotePort"] = os.str();

    Ice::WSConnectionInfoPtr wsinfo = ICE_DYNAMIC_CAST(Ice::WSConnectionInfo, info);
    if(wsinfo)
    {
        for(Ice::HeaderDict::const_iterator p = wsinfo->headers.begin(); p != wsinfo->headers.end(); ++p)
        {
            ctx["ws." + p->first] = p->second;
        }
    }

    IceSSL::WSSConnectionInfoPtr wssinfo = ICE_DYNAMIC_CAST(IceSSL::WSSConnectionInfo, info);
    if(wssinfo)
    {
        for(Ice::HeaderDict::const_iterator p = wssinfo->headers.begin(); p != wssinfo->headers.end(); ++p)
        {
            ctx["ws." + p->first] = p->second;
        }
    }

    return ctx;
}
