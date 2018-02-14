// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <objc/IceSSL.h>
#import <TestCommon.h>
#import <InfoTest.h>


TestInfoTestIntfPrx*
infoAllTests(id<ICECommunicator> communicator)
{
    tprintf("testing proxy endpoint information... ");
    {
        ICEObjectPrx* p1 =
            [communicator stringToProxy:@"test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:"
                                         "udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:"
                                         "opaque -e 1.8 -t 100 -v ABCD"];

        ICEEndpointSeq* endps = [p1 ice_getEndpoints];
        id<ICEEndpoint> endpoint = [endps objectAtIndex:0];
        ICEIPEndpointInfo<ICEEndpointInfo>* ipEndpoint = (ICEIPEndpointInfo<ICEEndpointInfo>*)[endpoint getInfo];
        test([ipEndpoint isKindOfClass:[ICEIPEndpointInfo class]]);
        test([[ipEndpoint host] isEqualToString:@"tcphost"]);
        test(ipEndpoint.port == 10000);
        test(ipEndpoint.timeout == 1200);
        test([[ipEndpoint sourceAddress] isEqualToString:@"10.10.10.10"]);
        test(ipEndpoint.compress);
        test(![ipEndpoint datagram]);
        test(([ipEndpoint type] == ICETCPEndpointType && ![ipEndpoint secure]) ||
             ([ipEndpoint type] == ICESSLEndpointType && [ipEndpoint secure]) ||
             ([ipEndpoint type] == ICEWSEndpointType && ![ipEndpoint secure]) ||
             ([ipEndpoint type] == ICEWSSEndpointType && [ipEndpoint secure]));

        test(([ipEndpoint type] == ICETCPEndpointType && [ipEndpoint isKindOfClass:[ICETCPEndpointInfo class]]) ||
             ([ipEndpoint type] == ICESSLEndpointType && [ipEndpoint isKindOfClass:[ICESSLEndpointInfo class]]) ||
             ([ipEndpoint type] == ICEWSEndpointType && [ipEndpoint isKindOfClass:[ICEWSEndpointInfo class]]) ||
             ([ipEndpoint type] == ICEWSSEndpointType && [ipEndpoint isKindOfClass:[ICESSLWSSEndpointInfo class]]));


        endpoint = [endps objectAtIndex:1];
        ICEUDPEndpointInfo<ICEEndpointInfo>* udpEndpoint = (ICEUDPEndpointInfo<ICEEndpointInfo>*)[endpoint getInfo];
        test([udpEndpoint isKindOfClass:[ICEUDPEndpointInfo class]]);
        test([udpEndpoint.host isEqualToString:@"udphost"]);
        test(udpEndpoint.port == 10001);
        test([[udpEndpoint sourceAddress] isEqualToString:@"10.10.10.10"]);
        test([udpEndpoint.mcastInterface isEqualToString:@"eth0"]);
        test(udpEndpoint.mcastTtl == 5);
        test(udpEndpoint.timeout == -1);
        test(!udpEndpoint.compress);
        test(![udpEndpoint secure]);
        test([udpEndpoint datagram]);
        test([udpEndpoint type] == ICEUDPEndpointType);

        endpoint = [endps objectAtIndex:2];
        ICEOpaqueEndpointInfo* opaqueEndpoint = (ICEOpaqueEndpointInfo*)[endpoint getInfo];
        test([opaqueEndpoint isKindOfClass:[ICEOpaqueEndpointInfo class]]);
        ICEEncodingVersion* rev = ICE_AUTORELEASE([[ICEEncodingVersion alloc] init:1 minor:8]);
        test([opaqueEndpoint.rawEncoding isEqual:rev]);
    }
    tprintf("ok\n");

    NSString* defaultHost = [[communicator getProperties] getProperty:@"Ice.Default.Host"];
    tprintf("test object adapter endpoint information... ");
    {
        [[communicator getProperties] setProperty:@"TestAdapter.Endpoints" value:@"default -t 15000:udp"];
        id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"TestAdapter"];

        ICEEndpointSeq* endpoints = [adapter getEndpoints];
        test([endpoints count] == 2);
        ICEEndpointSeq* publishedEndpoints = [adapter getPublishedEndpoints];
        test([endpoints isEqualToArray:publishedEndpoints]);

        id<ICEEndpoint> endpoint = [endpoints objectAtIndex:0];
        ICEIPEndpointInfo<ICEEndpointInfo>* ipEndpoint = (ICEIPEndpointInfo<ICEEndpointInfo>*)[endpoint getInfo];
        test([ipEndpoint isKindOfClass:[ICEIPEndpointInfo class]]);
        test([ipEndpoint type] == ICETCPEndpointType || [ipEndpoint type] == ICESSLEndpointType ||
             [ipEndpoint type] == ICEWSEndpointType || [ipEndpoint type] == ICEWSSEndpointType);
        test([ipEndpoint.host isEqualToString:defaultHost]);
        test(ipEndpoint.port > 0);
        test(ipEndpoint.timeout == 15000);

        endpoint = [endpoints objectAtIndex:1];
        ICEUDPEndpointInfo<ICEEndpointInfo>* udpEndpoint = (ICEUDPEndpointInfo<ICEEndpointInfo>*)[endpoint getInfo];
        test([udpEndpoint isKindOfClass:[ICEUDPEndpointInfo class]]);
        test([udpEndpoint.host isEqualToString:defaultHost]);
        test([udpEndpoint datagram]);
        test(udpEndpoint.port > 0);

        [adapter destroy];

        [[communicator getProperties] setProperty:@"TestAdapter.Endpoints" value:@"default -h * -p 12020"];
        [[communicator getProperties] setProperty:@"TestAdapter.PublishedEndpoints" value:@"default -h 127.0.0.1 -p 12020"];
        adapter = [communicator createObjectAdapter:@"TestAdapter"];

        endpoints = [adapter getEndpoints];
        test([endpoints count] >= 1);
        publishedEndpoints = [adapter getPublishedEndpoints];
        test([publishedEndpoints count] == 1);

        for(id object in endpoints)
        {
            endpoint = (id<ICEEndpoint>)object;
            ipEndpoint = (ICEIPEndpointInfo<ICEEndpointInfo>*)[endpoint getInfo];
            test(ipEndpoint.port == 12020);
        }

        ipEndpoint = (ICEIPEndpointInfo<ICEEndpointInfo>*)[[publishedEndpoints objectAtIndex:0] getInfo];
        test([ipEndpoint.host isEqualToString:@"127.0.0.1"]);
        test(ipEndpoint.port == 12020);

        [adapter destroy];
    }
    tprintf("ok\n");

    ICEObjectPrx* base = [communicator stringToProxy:@"test:default -p 12010:udp -p 12010"];
    TestInfoTestIntfPrx* testIntf = [TestInfoTestIntfPrx checkedCast:base];

    tprintf("test connection endpoint information... ");
    {
        ICEEndpointInfo* info = [[[base ice_getConnection] getEndpoint] getInfo];
        ICEIPEndpointInfo* ipinfo = (ICEIPEndpointInfo*)info;
        test([ipinfo isKindOfClass:[ICEIPEndpointInfo class]]);
        test(ipinfo.port == 12010);
        test(!ipinfo.compress);
        test([ipinfo.host isEqualToString:defaultHost]);

        ICEContext* ctx = [testIntf getEndpointInfoAsContext];
        test([[ctx objectForKey:@"host"] isEqualToString:ipinfo.host]);
        test([[ctx objectForKey:@"compress"] isEqualToString:@"false"]);
        test([[ctx objectForKey:@"port"] intValue] > 0);

        info = [[[[base ice_datagram] ice_getConnection] getEndpoint] getInfo];
        ICEUDPEndpointInfo* udp = (ICEUDPEndpointInfo*)info;
        test([udp isKindOfClass:[ICEUDPEndpointInfo class]]);
        test(udp.port == 12010);
        test([udp.host isEqualToString:defaultHost]);
    }
    tprintf("ok\n");

    tprintf("testing connection information... ");
    {
        id<ICEConnection> connection = [base ice_getConnection];
        [connection setBufferSize:1024 sndSize:2048];

        ICEIPConnectionInfo* info = (ICEIPConnectionInfo*)[connection getInfo];
        test([info isKindOfClass:[ICEIPConnectionInfo class]]);
        test(!info.incoming);
        test([info.adapterName isEqualToString:@""]);
        test(info.localPort > 0);
        test(info.remotePort == 12010);
        if([defaultHost isEqualToString:@"127.0.0.1"])
        {
            test([info.remoteAddress isEqualToString:defaultHost]);
            test([info.localAddress isEqualToString:defaultHost]);
        }
        test(info.rcvSize >= 1024);
        test(info.sndSize >= 2048);

        ICEContext* ctx = [testIntf getConnectionInfoAsContext];
        test([[ctx objectForKey:@"incoming"] isEqualToString:@"true"]);
        test([[ctx objectForKey:@"adapterName"] isEqualToString:@"TestAdapter"]);
        test([[ctx objectForKey:@"remoteAddress"] isEqualToString:info.remoteAddress]);
        test([[ctx objectForKey:@"localAddress"] isEqualToString:info.localAddress]);
        test([[ctx objectForKey:@"remotePort"] intValue] == info.localPort);
        test([[ctx objectForKey:@"localPort"] intValue] == info.remotePort);

        if([info isKindOfClass:[ICEWSConnectionInfo class]] || [info isKindOfClass:[ICESSLWSSConnectionInfo class]])
        {
            ICEHeaderDict* headers;
            if([info isKindOfClass:[ICEWSConnectionInfo class]])
            {
                ICEWSConnectionInfo* wsinfo = (ICEWSConnectionInfo*)info;
                headers = wsinfo.headers;
            }

            if([info isKindOfClass:[ICESSLWSSConnectionInfo class]])
            {
                ICESSLWSSConnectionInfo* wssinfo = (ICESSLWSSConnectionInfo*)info;
                headers = wssinfo.headers;
            }

            test([[headers objectForKey:@"Upgrade"] isEqualToString:@"websocket"]);
            test([[headers objectForKey:@"Connection"] isEqualToString:@"Upgrade"]);
            test([[headers objectForKey:@"Sec-WebSocket-Protocol"] isEqualToString:@"ice.zeroc.com"]);
            test([headers objectForKey:@"Sec-WebSocket-Accept"] != nil);

            test([[ctx objectForKey:@"ws.Upgrade"] isEqualToString:@"websocket"]);
            test([[ctx objectForKey:@"ws.Connection"] isEqualToString:@"Upgrade"]);
            test([[ctx objectForKey:@"ws.Sec-WebSocket-Protocol"] isEqualToString:@"ice.zeroc.com"]);
            test([[ctx objectForKey:@"ws.Sec-WebSocket-Version"] isEqualToString:@"13"]);
            test([ctx objectForKey:@"ws.Sec-WebSocket-Key"] != nil);
        }

        connection = [[base ice_datagram] ice_getConnection];
        [connection setBufferSize:2048 sndSize:1024];

        info = (ICEIPConnectionInfo*)[connection getInfo];
        test([info isKindOfClass:[ICEIPConnectionInfo class]]);
        test(!info.incoming);
        test([info.adapterName isEqualToString:@""]);
        test(info.localPort > 0);
        test(info.remotePort == 12010);
        if([defaultHost isEqualToString:@"127.0.0.1"])
        {
            test([info.remoteAddress isEqualToString:defaultHost]);
            test([info.localAddress isEqualToString:defaultHost]);
        }
        test(info.rcvSize >= 2048);
        test(info.sndSize >= 1024);
    }
    tprintf("ok\n");

    return testIntf;
}
