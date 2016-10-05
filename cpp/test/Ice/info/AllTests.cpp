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

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing proxy endpoint information... " << flush;
    {
        Ice::ObjectPrx p1 =
            communicator->stringToProxy("test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:"
                                        "udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:"
                                        "opaque -e 1.8 -t 100 -v ABCD");

        Ice::EndpointSeq endps = p1->ice_getEndpoints();

        Ice::IPEndpointInfoPtr ipEndpoint = Ice::IPEndpointInfoPtr::dynamicCast(endps[0]->getInfo());
        test(ipEndpoint);
        test(ipEndpoint->host == "tcphost");
        test(ipEndpoint->port == 10000);
        test(ipEndpoint->timeout == 1200);
#if !defined(ICE_OS_WINRT)
        test(ipEndpoint->sourceAddress == "10.10.10.10");
#endif
        test(ipEndpoint->compress);
        test(!ipEndpoint->datagram());
        test((ipEndpoint->type() == Ice::TCPEndpointType && !ipEndpoint->secure()) ||
             (ipEndpoint->type() == IceSSL::EndpointType && ipEndpoint->secure()) ||
             (ipEndpoint->type() == Ice::WSEndpointType && !ipEndpoint->secure()) ||
             (ipEndpoint->type() == Ice::WSSEndpointType && ipEndpoint->secure()));
        test((ipEndpoint->type() == Ice::TCPEndpointType && Ice::TCPEndpointInfoPtr::dynamicCast(ipEndpoint)) ||
             (ipEndpoint->type() == IceSSL::EndpointType && IceSSL::EndpointInfoPtr::dynamicCast(ipEndpoint)) ||
             (ipEndpoint->type() == Ice::WSEndpointType && Ice::WSEndpointInfoPtr::dynamicCast(ipEndpoint)) ||
             (ipEndpoint->type() == Ice::WSSEndpointType && IceSSL::WSSEndpointInfoPtr::dynamicCast(ipEndpoint)));

        Ice::UDPEndpointInfoPtr udpEndpoint = Ice::UDPEndpointInfoPtr::dynamicCast(endps[1]->getInfo());
        test(udpEndpoint);
        test(udpEndpoint->host == "udphost");
        test(udpEndpoint->port == 10001);
#if !defined(ICE_OS_WINRT)
        test(udpEndpoint->sourceAddress == "10.10.10.10");
#endif
        test(udpEndpoint->mcastInterface == "eth0");
        test(udpEndpoint->mcastTtl == 5);
        test(udpEndpoint->timeout == -1);
        test(!udpEndpoint->compress);
        test(!udpEndpoint->secure());
        test(udpEndpoint->datagram());
        test(udpEndpoint->type() == Ice::UDPEndpointType);

        Ice::OpaqueEndpointInfoPtr opaqueEndpoint = Ice::OpaqueEndpointInfoPtr::dynamicCast(endps[2]->getInfo());
        test(opaqueEndpoint);
        Ice::EncodingVersion rev;
        rev.major = 1;
        rev.minor = 8;
        test(opaqueEndpoint->rawEncoding == rev);
    }
    cout << "ok" << endl;

#ifdef ICE_OS_WINRT
    bool winrt = true;
#else
    bool winrt = false;
#endif

    string defaultHost = communicator->getProperties()->getProperty("Ice.Default.Host");
    if(!winrt || (communicator->getProperties()->getProperty("Ice.Default.Protocol") != "ssl" &&
                  communicator->getProperties()->getProperty("Ice.Default.Protocol") != "wss"))
    {
        cout << "test object adapter endpoint information... " << flush;
        {
            communicator->getProperties()->setProperty("TestAdapter.Endpoints", "default -h 127.0.0.1 -t 15000:udp -h 127.0.0.1");
            Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");

            Ice::EndpointSeq endpoints = adapter->getEndpoints();
            test(endpoints.size() == 2);
            Ice::EndpointSeq publishedEndpoints = adapter->getPublishedEndpoints();
            test(endpoints == publishedEndpoints);

            Ice::IPEndpointInfoPtr ipEndpoint = Ice::IPEndpointInfoPtr::dynamicCast(endpoints[0]->getInfo());
            test(ipEndpoint);
            test(ipEndpoint->type() == Ice::TCPEndpointType || ipEndpoint->type() == IceSSL::EndpointType ||
                 ipEndpoint->type() == Ice::WSEndpointType || ipEndpoint->type() == Ice::WSSEndpointType);
            test(ipEndpoint->host == "127.0.0.1");
            test(ipEndpoint->port > 0);
            test(ipEndpoint->timeout == 15000);

            Ice::UDPEndpointInfoPtr udpEndpoint = Ice::UDPEndpointInfoPtr::dynamicCast(endpoints[1]->getInfo());
            test(udpEndpoint);
            test(udpEndpoint->host == "127.0.0.1");
            test(udpEndpoint->datagram());
            test(udpEndpoint->port > 0);

            adapter->destroy();

            communicator->getProperties()->setProperty("TestAdapter.Endpoints", "default -h * -p 12020");
            communicator->getProperties()->setProperty("TestAdapter.PublishedEndpoints", "default -h 127.0.0.1 -p 12020");
            adapter = communicator->createObjectAdapter("TestAdapter");

            endpoints = adapter->getEndpoints();
            test(endpoints.size() >= 1);
            publishedEndpoints = adapter->getPublishedEndpoints();
            test(publishedEndpoints.size() == 1);

            for(Ice::EndpointSeq::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
            {
                ipEndpoint = Ice::IPEndpointInfoPtr::dynamicCast((*p)->getInfo());
                test(ipEndpoint->port == 12020);
            }

            ipEndpoint = Ice::IPEndpointInfoPtr::dynamicCast(publishedEndpoints[0]->getInfo());
            test(ipEndpoint->host == "127.0.0.1");
            test(ipEndpoint->port == 12020);

            adapter->destroy();
        }
        cout << "ok" << endl;
    }

    Ice::ObjectPrx base = communicator->stringToProxy("test:default -p 12010:udp -p 12010 -c");
    TestIntfPrx testIntf = TestIntfPrx::checkedCast(base);

    cout << "test connection endpoint information... " << flush;
    {
        Ice::EndpointInfoPtr info = base->ice_getConnection()->getEndpoint()->getInfo();
        Ice::IPEndpointInfoPtr ipinfo = Ice::IPEndpointInfoPtr::dynamicCast(info);
        test(ipinfo->port == 12010);
        test(!ipinfo->compress);
        test(ipinfo->host == defaultHost);

        ostringstream os;

        Ice::Context ctx = testIntf->getEndpointInfoAsContext();
        test(ctx["host"] == ipinfo->host);
        test(ctx["compress"] == "false");
        istringstream is(ctx["port"]);
        int port;
        is >> port;
        test(port > 0);

        info = base->ice_datagram()->ice_getConnection()->getEndpoint()->getInfo();
        Ice::UDPEndpointInfoPtr udp = Ice::UDPEndpointInfoPtr::dynamicCast(info);
        test(udp);
        test(udp->port == 12010);
        test(udp->host == defaultHost);
    }
    cout << "ok" << endl;

    cout << "testing connection information... " << flush;
    {
        Ice::ConnectionPtr connection = base->ice_getConnection();
        connection->setBufferSize(1024, 2048);

        Ice::IPConnectionInfoPtr info = Ice::IPConnectionInfoPtr::dynamicCast(connection->getInfo());
        test(info);
        test(!info->incoming);
        test(info->adapterName.empty());
        test(info->localPort > 0);
        test(info->remotePort == 12010);
        if(defaultHost == "127.0.0.1")
        {
            test(info->remoteAddress == defaultHost);
            test(info->localAddress == defaultHost);
        }
#if !defined(ICE_OS_WINRT)
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

            Ice::WSConnectionInfoPtr wsinfo = Ice::WSConnectionInfoPtr::dynamicCast(info);
            if(wsinfo)
            {
                headers = wsinfo->headers;
            }

            IceSSL::WSSConnectionInfoPtr wssinfo = IceSSL::WSSConnectionInfoPtr::dynamicCast(info);
            if(wssinfo)
            {
                headers = wssinfo->headers;
                test(wssinfo->verified);
#if !defined(ICE_OS_WINRT) && TARGET_OS_IPHONE==0
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

        info = Ice::IPConnectionInfoPtr::dynamicCast(connection->getInfo());
        test(!info->incoming);
        test(info->adapterName.empty());
        test(info->localPort > 0);
        test(info->remotePort == 12010);
        if(defaultHost == "127.0.0.1")
        {
            test(info->remoteAddress == defaultHost);
            test(info->localAddress == defaultHost);
        }

#if !defined(ICE_OS_WINRT)
        test(info->rcvSize >= 2048);
        test(info->sndSize >= 1024);
#endif
    }
    cout << "ok" << endl;

    testIntf->shutdown();

    communicator->shutdown();
    communicator->waitForShutdown();
}
