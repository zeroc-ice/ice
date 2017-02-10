// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
using namespace Test;

namespace
{

Ice::TCPEndpointInfoPtr
getTCPEndpointInfo(const Ice::EndpointInfoPtr& info)
{
    for(Ice::EndpointInfoPtr p = info; p; p = p->underlying)
    {
        Ice::TCPEndpointInfoPtr tcpInfo = ICE_DYNAMIC_CAST(Ice::TCPEndpointInfo, p);
        if(tcpInfo)
        {
            return tcpInfo;
        }
    }
    return ICE_NULLPTR;
}

Ice::TCPConnectionInfoPtr
getTCPConnectionInfo(const Ice::ConnectionInfoPtr& info)
{
    for(Ice::ConnectionInfoPtr p = info; p; p = p->underlying)
    {
        Ice::TCPConnectionInfoPtr tcpInfo = ICE_DYNAMIC_CAST(Ice::TCPConnectionInfo, p);
        if(tcpInfo)
        {
            return tcpInfo;
        }
    }
    return ICE_NULLPTR;
}

}

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing proxy endpoint information... " << flush;
    {
        Ice::ObjectPrxPtr p1 =
            communicator->stringToProxy("test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:"
                                        "udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:"
                                        "opaque -e 1.8 -t 100 -v ABCD");

        Ice::EndpointSeq endps = p1->ice_getEndpoints();

        Ice::EndpointInfoPtr info = endps[0]->getInfo();
        Ice::TCPEndpointInfoPtr ipEndpoint = getTCPEndpointInfo(info);
        test(ipEndpoint);
        test(ipEndpoint->host == "tcphost");
        test(ipEndpoint->port == 10000);
        test(ipEndpoint->timeout == 1200);
#if !defined(ICE_OS_UWP)
        test(ipEndpoint->sourceAddress == "10.10.10.10");
#endif
        test(ipEndpoint->compress);
        test(!ipEndpoint->datagram());
        test((ipEndpoint->type() == Ice::TCPEndpointType && !ipEndpoint->secure()) ||
             (ipEndpoint->type() == Ice::SSLEndpointType && ipEndpoint->secure()) ||
             (ipEndpoint->type() == Ice::WSEndpointType && !ipEndpoint->secure()) ||
             (ipEndpoint->type() == Ice::WSSEndpointType && ipEndpoint->secure()));

        test((ipEndpoint->type() == Ice::TCPEndpointType && ICE_DYNAMIC_CAST(Ice::TCPEndpointInfo, info)) ||
             (ipEndpoint->type() == Ice::SSLEndpointType && ICE_DYNAMIC_CAST(IceSSL::EndpointInfo, info)) ||
             (ipEndpoint->type() == Ice::WSEndpointType && ICE_DYNAMIC_CAST(Ice::WSEndpointInfo, info)) ||
             (ipEndpoint->type() == Ice::WSSEndpointType && ICE_DYNAMIC_CAST(Ice::WSEndpointInfo, info)));

        Ice::UDPEndpointInfoPtr udpEndpoint = ICE_DYNAMIC_CAST(Ice::UDPEndpointInfo, endps[1]->getInfo());
        test(udpEndpoint);
        test(udpEndpoint->host == "udphost");
        test(udpEndpoint->port == 10001);
#if !defined(ICE_OS_UWP)
        test(udpEndpoint->sourceAddress == "10.10.10.10");
#endif
        test(udpEndpoint->mcastInterface == "eth0");
        test(udpEndpoint->mcastTtl == 5);
        test(udpEndpoint->timeout == -1);
        test(!udpEndpoint->compress);
        test(!udpEndpoint->secure());
        test(udpEndpoint->datagram());
        test(udpEndpoint->type() == Ice::UDPEndpointType);

        Ice::OpaqueEndpointInfoPtr opaqueEndpoint = ICE_DYNAMIC_CAST(Ice::OpaqueEndpointInfo, endps[2]->getInfo());
        test(opaqueEndpoint);
        Ice::EncodingVersion rev;
        rev.major = 1;
        rev.minor = 8;
        test(opaqueEndpoint->rawEncoding == rev);
    }
    cout << "ok" << endl;

    string defaultHost = communicator->getProperties()->getProperty("Ice.Default.Host");
#ifdef ICE_OS_UWP
    bool uwp = true;
#else
    bool uwp = false;
#endif
    if(!uwp || (communicator->getProperties()->getProperty("Ice.Default.Protocol") != "ssl" &&
                  communicator->getProperties()->getProperty("Ice.Default.Protocol") != "wss"))
    {
        cout << "test object adapter endpoint information... " << flush;
        {
            communicator->getProperties()->setProperty("TestAdapter.Endpoints",
                                                       "default -h 127.0.0.1 -t 15000:udp -h 127.0.0.1");
            Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");

            Ice::EndpointSeq endpoints = adapter->getEndpoints();
            test(endpoints.size() == 2);
            Ice::EndpointSeq publishedEndpoints = adapter->getPublishedEndpoints();
            test(endpoints == publishedEndpoints);

            Ice::TCPEndpointInfoPtr ipEndpoint = getTCPEndpointInfo(endpoints[0]->getInfo());
            test(ipEndpoint);
            test(ipEndpoint->type() == Ice::TCPEndpointType || ipEndpoint->type() == Ice::SSLEndpointType ||
                ipEndpoint->type() == Ice::WSEndpointType || ipEndpoint->type() == Ice::WSSEndpointType);
            test(ipEndpoint->host == "127.0.0.1");
            test(ipEndpoint->port > 0);
            test(ipEndpoint->timeout == 15000);

            Ice::UDPEndpointInfoPtr udpEndpoint = ICE_DYNAMIC_CAST(Ice::UDPEndpointInfo, endpoints[1]->getInfo());
            test(udpEndpoint);
            test(udpEndpoint->host == "127.0.0.1");
            test(udpEndpoint->datagram());
            test(udpEndpoint->port > 0);

            endpoints.pop_back();
            test(endpoints.size() == 1);
            adapter->setPublishedEndpoints(endpoints);
            publishedEndpoints = adapter->getPublishedEndpoints();
            test(endpoints == publishedEndpoints);

            adapter->destroy();

            int port = getTestPort(communicator->getProperties(), 1);
            ostringstream portStr;
            portStr << port;
            communicator->getProperties()->setProperty("TestAdapter.Endpoints", "default -h * -p " + portStr.str());
            communicator->getProperties()->setProperty("TestAdapter.PublishedEndpoints", getTestEndpoint(communicator, 1));
            adapter = communicator->createObjectAdapter("TestAdapter");

            endpoints = adapter->getEndpoints();
            test(endpoints.size() >= 1);
            publishedEndpoints = adapter->getPublishedEndpoints();
            test(publishedEndpoints.size() == 1);

            for(Ice::EndpointSeq::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
            {
                ipEndpoint = getTCPEndpointInfo((*p)->getInfo());
                test(ipEndpoint->port == port);
            }

            ipEndpoint = getTCPEndpointInfo(publishedEndpoints[0]->getInfo());
            test(ipEndpoint->host == "127.0.0.1");
            test(ipEndpoint->port == port);

            adapter->destroy();
        }
        cout << "ok" << endl;
    }

    string endpoints = getTestEndpoint(communicator, 0) + ":" + getTestEndpoint(communicator, 0, "udp") + " -c";
    int port = getTestPort(communicator->getProperties(), 0);
    Ice::ObjectPrxPtr base = communicator->stringToProxy("test:" + endpoints);
    TestIntfPrxPtr testIntf = ICE_CHECKED_CAST(TestIntfPrx, base);

    cout << "test connection endpoint information... " << flush;
    {
        Ice::EndpointInfoPtr info = base->ice_getConnection()->getEndpoint()->getInfo();
        Ice::TCPEndpointInfoPtr tcpinfo = getTCPEndpointInfo(info);
        test(tcpinfo->port == port);
        test(!tcpinfo->compress);
        test(tcpinfo->host == defaultHost);

        ostringstream os;

        Ice::Context ctx = testIntf->getEndpointInfoAsContext();
        test(ctx["host"] == tcpinfo->host);
        test(ctx["compress"] == "false");
        istringstream is(ctx["port"]);
        int port;
        is >> port;
        test(port > 0);

        info = base->ice_datagram()->ice_getConnection()->getEndpoint()->getInfo();
        Ice::UDPEndpointInfoPtr udp = ICE_DYNAMIC_CAST(Ice::UDPEndpointInfo, info);
        test(udp);
        test(udp->port == port);
        test(udp->host == defaultHost);
    }
    cout << "ok" << endl;

    cout << "testing connection information... " << flush;
    {
        Ice::ConnectionPtr connection = base->ice_getConnection();
        connection->setBufferSize(1024, 2048);

        Ice::TCPConnectionInfoPtr info = getTCPConnectionInfo(connection->getInfo());
        test(info);
        test(!info->incoming);
        test(info->adapterName.empty());
        test(info->localPort > 0);
        test(info->remotePort == port);
        if(defaultHost == "127.0.0.1")
        {
            test(info->remoteAddress == defaultHost);
            test(info->localAddress == defaultHost);
        }
#if !defined(ICE_OS_UWP)
        test(info->rcvSize >= 1024);
        test(info->sndSize >= 2048);
#endif

        ostringstream os;

        Ice::Context ctx = testIntf->getConnectionInfoAsContext();
        test(ctx["incoming"] == "true");
        test(ctx["adapterName"] == "TestAdapter");
        test(ctx["remoteAddress"] == info->localAddress);
        test(ctx["localAddress"] == info->remoteAddress);
        os.str("");
        os << info->localPort;
        test(ctx["remotePort"] == os.str());
        os.str("");
        os << info->remotePort;
        test(ctx["localPort"] == os.str());

        if(base->ice_getConnection()->type() == "ws" || base->ice_getConnection()->type() == "wss")
        {
            Ice::HeaderDict headers;

            Ice::WSConnectionInfoPtr wsinfo = ICE_DYNAMIC_CAST(Ice::WSConnectionInfo, connection->getInfo());
            test(wsinfo);
            headers = wsinfo->headers;

            if(base->ice_getConnection()->type() == "wss")
            {
                IceSSL::ConnectionInfoPtr wssinfo = ICE_DYNAMIC_CAST(IceSSL::ConnectionInfo, wsinfo->underlying);
                test(wssinfo->verified);
#if !defined(ICE_OS_UWP) && TARGET_OS_IPHONE==0
                test(!wssinfo->certs.empty());
#endif
            }

            test(headers["Upgrade"] == "websocket");
            test(headers["Connection"] == "Upgrade");
            test(headers["Sec-WebSocket-Protocol"] == "ice.zeroc.com");
            test(headers.find("Sec-WebSocket-Accept") != headers.end());

            test(ctx["ws.Upgrade"] == "websocket");
            test(ctx["ws.Connection"] == "Upgrade");
            test(ctx["ws.Sec-WebSocket-Protocol"] == "ice.zeroc.com");
            test(ctx["ws.Sec-WebSocket-Version"] == "13");
            test(ctx.find("ws.Sec-WebSocket-Key") != ctx.end());
        }

        connection = base->ice_datagram()->ice_getConnection();
        connection->setBufferSize(2048, 1024);

        Ice::UDPConnectionInfoPtr udpinfo = ICE_DYNAMIC_CAST(Ice::UDPConnectionInfo, connection->getInfo());
        test(!udpinfo->incoming);
        test(udpinfo->adapterName.empty());
        test(udpinfo->localPort > 0);
        test(udpinfo->remotePort == port);
        if(defaultHost == "127.0.0.1")
        {
            test(udpinfo->remoteAddress == defaultHost);
            test(udpinfo->localAddress == defaultHost);
        }

#if !defined(ICE_OS_UWP)
        test(udpinfo->rcvSize >= 2048);
        test(udpinfo->sndSize >= 1024);
#endif
    }
    cout << "ok" << endl;

    testIntf->shutdown();

    communicator->shutdown();
    communicator->waitForShutdown();
}
