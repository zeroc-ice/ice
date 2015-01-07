// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <InfoTest.h>

 
TestInfoTestIntfPrx*
infoAllTests(id<ICECommunicator> communicator)
{
    tprintf("testing proxy endpoint information... ");
    {
        ICEObjectPrx* p1 = [communicator stringToProxy:@"test -t:default -h tcphost -p 10000 -t 1200 -z:"
                                                        "udp -h udphost -p 10001 --interface eth0 --ttl 5:"
                                                        "opaque -e 1.8 -t 100 -v ABCD"];

        ICEEndpointSeq* endps = [p1 ice_getEndpoints];
        id<ICEEndpoint> endpoint = [endps objectAtIndex:0];
        ICEIPEndpointInfo* ipEndpoint = (ICEIPEndpointInfo*)[endpoint getInfo];
        test([ipEndpoint isKindOfClass:[ICEIPEndpointInfo class]]);
        test([[ipEndpoint host] isEqualToString:@"tcphost"]);
        test(ipEndpoint.port == 10000);
        test(ipEndpoint.timeout == 1200);
        test(ipEndpoint.compress);
        test(![ipEndpoint datagram]);
        test(([ipEndpoint type] == ICETCPEndpointType && ![ipEndpoint secure]) ||
             ([ipEndpoint type] == ICESSLEndpointType && [ipEndpoint secure]));

        test(([ipEndpoint type] == ICETCPEndpointType && [ipEndpoint isKindOfClass:[ICETCPEndpointInfo class]]) ||
             ([ipEndpoint type] == ICESSLEndpointType && [ipEndpoint isKindOfClass:[ICESSLEndpointInfo class]]));


        endpoint = [endps objectAtIndex:1];
        ICEUDPEndpointInfo* udpEndpoint = (ICEUDPEndpointInfo*)[endpoint getInfo];
        test([udpEndpoint isKindOfClass:[ICEUDPEndpointInfo class]]);
        test([udpEndpoint.host isEqualToString:@"udphost"]);
        test(udpEndpoint.port == 10001);
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
        ICEEncodingVersion* rev = [[ICEEncodingVersion alloc] init:1 minor:8];
#if defined(__clang__) && !__has_feature(objc_arc)
        [rev autorelease];
#endif
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
        ICEIPEndpointInfo* ipEndpoint = (ICEIPEndpointInfo*)[endpoint getInfo];
        test([ipEndpoint isKindOfClass:[ICEIPEndpointInfo class]]);
        test([ipEndpoint type] == ICETCPEndpointType || [ipEndpoint type] == ICESSLEndpointType);
        test([ipEndpoint.host isEqualToString:defaultHost]);
        test(ipEndpoint.port > 0);
        test(ipEndpoint.timeout == 15000);

        endpoint = [endpoints objectAtIndex:1];
        ICEUDPEndpointInfo* udpEndpoint = (ICEUDPEndpointInfo*)[endpoint getInfo];
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
            ipEndpoint = (ICEIPEndpointInfo*)[endpoint getInfo];
            test(ipEndpoint.port == 12020);
        }

        ipEndpoint = (ICEIPEndpointInfo*)[[publishedEndpoints objectAtIndex:0] getInfo];
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
        ICEIPConnectionInfo* info = (ICEIPConnectionInfo*)[[base ice_getConnection] getInfo];
        test([info isKindOfClass:[ICEIPConnectionInfo class]]);
        test(!info.incoming);
        test([info.adapterName isEqualToString:@""]);
        test(info.localPort > 0);
        test(info.remotePort == 12010);
        test([info.remoteAddress isEqualToString:defaultHost]);
        test([info.localAddress isEqualToString:defaultHost]);


        ICEContext* ctx = [testIntf getConnectionInfoAsContext];
        test([[ctx objectForKey:@"incoming"] isEqualToString:@"true"]);
        test([[ctx objectForKey:@"adapterName"] isEqualToString:@"TestAdapter"]);
        test([[ctx objectForKey:@"remoteAddress"] isEqualToString:info.remoteAddress]);
        test([[ctx objectForKey:@"localAddress"] isEqualToString:info.localAddress]);
        test([[ctx objectForKey:@"remotePort"] intValue] == info.localPort);
        test([[ctx objectForKey:@"localPort"] intValue] == info.remotePort);

        info = (ICEIPConnectionInfo*)[[[base ice_datagram] ice_getConnection] getInfo];
        test([info isKindOfClass:[ICEIPConnectionInfo class]]);
        test(!info.incoming);
        test([info.adapterName isEqualToString:@""]);
        test(info.localPort > 0);
        test(info.remotePort == 12010);
        test([info.remoteAddress isEqualToString:defaultHost]);
        test([info.localAddress isEqualToString:defaultHost]);
    }
    tprintf("ok\n");

    return testIntf;
}
