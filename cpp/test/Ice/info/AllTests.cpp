// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/SSL/SSLUtil.h"
#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

using namespace std;
using namespace Test;

namespace
{
    Ice::TCPEndpointInfoPtr getTCPEndpointInfo(const Ice::EndpointInfoPtr& info)
    {
        for (Ice::EndpointInfoPtr p = info; p; p = p->underlying)
        {
            Ice::TCPEndpointInfoPtr tcpInfo = dynamic_pointer_cast<Ice::TCPEndpointInfo>(p);
            if (tcpInfo)
            {
                return tcpInfo;
            }
        }
        return nullptr;
    }

    Ice::TCPConnectionInfoPtr getTCPConnectionInfo(const Ice::ConnectionInfoPtr& info)
    {
        for (Ice::ConnectionInfoPtr p = info; p; p = p->underlying)
        {
            Ice::TCPConnectionInfoPtr tcpInfo = dynamic_pointer_cast<Ice::TCPConnectionInfo>(p);
            if (tcpInfo)
            {
                return tcpInfo;
            }
        }
        return nullptr;
    }

#if TARGET_OS_IPHONE == 0
    // IceSSL on iOS does not support getSubjectName
    void checkPeerCertificateSubjectName(const string& subjectName)
    {
        test(subjectName.find("CN=ca.server") != string::npos);
        test(subjectName.find("OU=Ice test infrastructure") != string::npos);
        test(subjectName.find("O=ZeroC") != string::npos);
        test(subjectName.find("L=Jupiter") != string::npos);
        test(subjectName.find("ST=Florida") != string::npos);
        test(subjectName.find("C=US") != string::npos);
        test(subjectName.find("emailAddress=info@zeroc.com") != string::npos);
    }
#endif
}

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    cout << "testing proxy endpoint information... " << flush;
    {
        Ice::ObjectPrx p1(
            communicator,
            "test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:"
            "udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:"
            "opaque -e 1.8 -t 100 -v ABCD");

        Ice::EndpointSeq endps = p1->ice_getEndpoints();

        Ice::EndpointInfoPtr info = endps[0]->getInfo();
        Ice::TCPEndpointInfoPtr ipEndpoint = getTCPEndpointInfo(info);
        test(ipEndpoint);
        test(ipEndpoint->host == "tcphost");
        test(ipEndpoint->port == 10000);
        test(ipEndpoint->sourceAddress == "10.10.10.10");
        test(ipEndpoint->compress);
        test(!ipEndpoint->datagram());
        test(
            (ipEndpoint->type() == Ice::TCPEndpointType && !ipEndpoint->secure()) ||
            (ipEndpoint->type() == Ice::SSLEndpointType && ipEndpoint->secure()) ||
            (ipEndpoint->type() == Ice::WSEndpointType && !ipEndpoint->secure()) ||
            (ipEndpoint->type() == Ice::WSSEndpointType && ipEndpoint->secure()));

        test(
            (ipEndpoint->type() == Ice::TCPEndpointType && dynamic_pointer_cast<Ice::TCPEndpointInfo>(info)) ||
            (ipEndpoint->type() == Ice::SSLEndpointType && dynamic_pointer_cast<Ice::SSL::EndpointInfo>(info)) ||
            (ipEndpoint->type() == Ice::WSEndpointType && dynamic_pointer_cast<Ice::WSEndpointInfo>(info)) ||
            (ipEndpoint->type() == Ice::WSSEndpointType && dynamic_pointer_cast<Ice::WSEndpointInfo>(info)));

        Ice::UDPEndpointInfoPtr udpEndpoint = dynamic_pointer_cast<Ice::UDPEndpointInfo>(endps[1]->getInfo());
        test(udpEndpoint);
        test(udpEndpoint->host == "udphost");
        test(udpEndpoint->port == 10001);
        test(udpEndpoint->sourceAddress == "10.10.10.10");
        test(udpEndpoint->mcastInterface == "eth0");
        test(udpEndpoint->mcastTtl == 5);
        test(!udpEndpoint->compress);
        test(!udpEndpoint->secure());
        test(udpEndpoint->datagram());
        test(udpEndpoint->type() == Ice::UDPEndpointType);

        Ice::OpaqueEndpointInfoPtr opaqueEndpoint = dynamic_pointer_cast<Ice::OpaqueEndpointInfo>(endps[2]->getInfo());
        test(opaqueEndpoint);
        Ice::EncodingVersion rev;
        rev.major = 1;
        rev.minor = 8;
        test(opaqueEndpoint->rawEncoding == rev);
    }
    cout << "ok" << endl;

    string defaultHost = communicator->getProperties()->getIceProperty("Ice.Default.Host");
    if (communicator->getProperties()->getIceProperty("Ice.Default.Protocol") != "ssl" &&
        communicator->getProperties()->getIceProperty("Ice.Default.Protocol") != "wss")
    {
        cout << "test object adapter endpoint information... " << flush;
        {
            communicator->getProperties()->setProperty(
                "TestAdapter.Endpoints",
                "default -h 127.0.0.1 -t 15000:udp -h 127.0.0.1");
            Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");

            Ice::EndpointSeq endpoints = adapter->getEndpoints();
            test(endpoints.size() == 2);
            Ice::EndpointSeq publishedEndpoints = adapter->getPublishedEndpoints();
            test(publishedEndpoints.size() == 2);
            test(*endpoints[0] == *publishedEndpoints[0]);
            test(*endpoints[1] == *publishedEndpoints[1]);

            Ice::TCPEndpointInfoPtr ipEndpoint = getTCPEndpointInfo(endpoints[0]->getInfo());
            test(ipEndpoint);
            test(
                ipEndpoint->type() == Ice::TCPEndpointType || ipEndpoint->type() == Ice::SSLEndpointType ||
                ipEndpoint->type() == Ice::WSEndpointType || ipEndpoint->type() == Ice::WSSEndpointType);
            test(ipEndpoint->host == "127.0.0.1");
            test(ipEndpoint->port > 0);

            Ice::UDPEndpointInfoPtr udpEndpoint = dynamic_pointer_cast<Ice::UDPEndpointInfo>(endpoints[1]->getInfo());
            test(udpEndpoint);
            test(udpEndpoint->host == "127.0.0.1");
            test(udpEndpoint->datagram());
            test(udpEndpoint->port > 0);

            endpoints.pop_back();
            test(endpoints.size() == 1);
            adapter->setPublishedEndpoints(endpoints);
            publishedEndpoints = adapter->getPublishedEndpoints();
            test(publishedEndpoints.size() == 1);
            test(*endpoints[0] == *publishedEndpoints[0]);

            adapter->destroy();

            int port = helper->getTestPort(1);
            ostringstream portStr;
            portStr << port;
            communicator->getProperties()->setProperty("TestAdapter.Endpoints", "default -h * -p " + portStr.str());
            communicator->getProperties()->setProperty("TestAdapter.PublishedEndpoints", helper->getTestEndpoint(1));
            adapter = communicator->createObjectAdapter("TestAdapter");

            endpoints = adapter->getEndpoints();
            test(endpoints.size() >= 1);
            publishedEndpoints = adapter->getPublishedEndpoints();
            test(publishedEndpoints.size() == 1);

            for (const auto& endpoint : endpoints)
            {
                ipEndpoint = getTCPEndpointInfo(endpoint->getInfo());
                test(ipEndpoint->port == port);
            }

            ipEndpoint = getTCPEndpointInfo(publishedEndpoints[0]->getInfo());
            test(ipEndpoint->host == helper->getTestHost());
            test(ipEndpoint->port == port);

            adapter->destroy();
        }
        cout << "ok" << endl;
    }

    int port = helper->getTestPort();
    TestIntfPrx testIntf(communicator, "test:" + helper->getTestEndpoint() + ":" + helper->getTestEndpoint("udp"));

    cout << "test connection endpoint information... " << flush;
    {
        Ice::EndpointInfoPtr info = testIntf->ice_getConnection()->getEndpoint()->getInfo();
        Ice::TCPEndpointInfoPtr tcpinfo = getTCPEndpointInfo(info);
        test(tcpinfo->port == port);
        test(!tcpinfo->compress);
        test(tcpinfo->host == defaultHost);

        ostringstream os;

        Ice::Context ctx = testIntf->getEndpointInfoAsContext();
        test(ctx["host"] == tcpinfo->host);
        test(ctx["compress"] == "false");
        istringstream is(ctx["port"]);
        int portCtx;
        is >> portCtx;
        test(portCtx > 0);

        info = testIntf->ice_datagram()->ice_getConnection()->getEndpoint()->getInfo();
        Ice::UDPEndpointInfoPtr udp = dynamic_pointer_cast<Ice::UDPEndpointInfo>(info);
        test(udp);
        test(udp->port == portCtx);
        test(udp->host == defaultHost);
    }
    cout << "ok" << endl;

    cout << "testing connection information... " << flush;
    {
        Ice::ConnectionPtr connection = testIntf->ice_getConnection();
        connection->setBufferSize(1024, 2048);

        Ice::TCPConnectionInfoPtr info = getTCPConnectionInfo(connection->getInfo());
        test(info);
        test(!info->incoming);
        test(info->adapterName.empty());
        test(info->localPort > 0);
        test(info->remotePort == port);
        if (defaultHost == "127.0.0.1")
        {
            test(info->remoteAddress == defaultHost);
            test(info->localAddress == defaultHost);
        }
        test(info->rcvSize >= 1024);
        test(info->sndSize >= 2048);

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

        if (testIntf->ice_getConnection()->type() == "ws" || testIntf->ice_getConnection()->type() == "wss")
        {
            Ice::HeaderDict headers;

            Ice::WSConnectionInfoPtr wsinfo = dynamic_pointer_cast<Ice::WSConnectionInfo>(connection->getInfo());
            test(wsinfo);
            headers = wsinfo->headers;

            if (testIntf->ice_getConnection()->type() == "wss")
            {
                auto sslinfo = dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(wsinfo->underlying);
                test(sslinfo);
                test(sslinfo->peerCertificate);
#if TARGET_OS_IPHONE == 0
                // IceSSL on iOS does not support getSubjectName
                checkPeerCertificateSubjectName(Ice::SSL::getSubjectName(sslinfo->peerCertificate));
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
        else if (testIntf->ice_getConnection()->type() == "ssl")
        {
            auto sslinfo = dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(connection->getInfo());
            test(sslinfo);
            test(sslinfo->peerCertificate);
#if TARGET_OS_IPHONE == 0
            // IceSSL on iOS does not support getSubjectName
            checkPeerCertificateSubjectName(Ice::SSL::getSubjectName(sslinfo->peerCertificate));
#endif
        }

        connection = testIntf->ice_datagram()->ice_getConnection();
        connection->setBufferSize(2048, 1024);

        Ice::UDPConnectionInfoPtr udpinfo = dynamic_pointer_cast<Ice::UDPConnectionInfo>(connection->getInfo());
        test(!udpinfo->incoming);
        test(udpinfo->adapterName.empty());
        test(udpinfo->localPort > 0);
        test(udpinfo->remotePort == port);
        if (defaultHost == "127.0.0.1")
        {
            test(udpinfo->remoteAddress == defaultHost);
            test(udpinfo->localAddress == defaultHost);
        }
        test(udpinfo->rcvSize >= 2048);
        test(udpinfo->sndSize >= 1024);
    }
    cout << "ok" << endl;

    testIntf->shutdown();

    communicator->shutdown();
    communicator->waitForShutdown();
}
