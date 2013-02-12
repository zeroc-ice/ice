// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
        Ice::ObjectPrx p1 = communicator->stringToProxy("test -t:default -h tcphost -p 10000 -t 1200 -z:"
                                                        "udp -h udphost -p 10001 --interface eth0 --ttl 5:"
                                                        "opaque -e 1.8 -t 100 -v ABCD");

        Ice::EndpointSeq endps = p1->ice_getEndpoints();

        Ice::IPEndpointInfoPtr ipEndpoint = Ice::IPEndpointInfoPtr::dynamicCast(endps[0]->getInfo());
        test(ipEndpoint);
        test(ipEndpoint->host == "tcphost");
        test(ipEndpoint->port == 10000);
        test(ipEndpoint->timeout == 1200);
        test(ipEndpoint->compress);
        test(!ipEndpoint->datagram());
        test((ipEndpoint->type() == Ice::TCPEndpointType && !ipEndpoint->secure()) ||
             (ipEndpoint->type() == IceSSL::EndpointType && ipEndpoint->secure()));
        test((ipEndpoint->type() == Ice::TCPEndpointType && Ice::TCPEndpointInfoPtr::dynamicCast(ipEndpoint)) ||
             (ipEndpoint->type() == IceSSL::EndpointType && IceSSL::EndpointInfoPtr::dynamicCast(ipEndpoint)));

        Ice::UDPEndpointInfoPtr udpEndpoint = Ice::UDPEndpointInfoPtr::dynamicCast(endps[1]->getInfo());
        test(udpEndpoint);
        test(udpEndpoint->host == "udphost");
        test(udpEndpoint->port == 10001);
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

    string defaultHost = communicator->getProperties()->getProperty("Ice.Default.Host");
    cout << "test object adapter endpoint information... " << flush;
    {
        communicator->getProperties()->setProperty("TestAdapter.Endpoints", "default -t 15000:udp");
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");

        Ice::EndpointSeq endpoints = adapter->getEndpoints();
        test(endpoints.size() == 2);
        Ice::EndpointSeq publishedEndpoints = adapter->getPublishedEndpoints();
        test(endpoints == publishedEndpoints);

        Ice::IPEndpointInfoPtr ipEndpoint = Ice::IPEndpointInfoPtr::dynamicCast(endpoints[0]->getInfo());
        test(ipEndpoint);
        test(ipEndpoint->type() == Ice::TCPEndpointType || ipEndpoint->type() == IceSSL::EndpointType);
        test(ipEndpoint->host == defaultHost);
        test(ipEndpoint->port > 0);
        test(ipEndpoint->timeout == 15000);

        Ice::UDPEndpointInfoPtr udpEndpoint = Ice::UDPEndpointInfoPtr::dynamicCast(endpoints[1]->getInfo());
        test(udpEndpoint);
        test(udpEndpoint->host == defaultHost);
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

    Ice::ObjectPrx base = communicator->stringToProxy("test:default -p 12010:udp -p 12010");
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
        Ice::IPConnectionInfoPtr info = Ice::IPConnectionInfoPtr::dynamicCast(base->ice_getConnection()->getInfo());
        test(info);
        test(!info->incoming);
        test(info->adapterName.empty());
        test(info->localPort > 0);
        test(info->remotePort == 12010);
        test(info->remoteAddress == defaultHost);
        test(info->localAddress == defaultHost);

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

        info = Ice::IPConnectionInfoPtr::dynamicCast(base->ice_datagram()->ice_getConnection()->getInfo());
        test(!info->incoming);
        test(info->adapterName.empty());
        test(info->localPort > 0);
        test(info->remotePort == 12010);
        test(info->remoteAddress ==defaultHost);
        test(info->localAddress == defaultHost);
    }
    cout << "ok" << endl;

    testIntf->shutdown();

    communicator->shutdown();
    communicator->waitForShutdown();
}
