// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <objc/IceSSL.h>
#import <info/TestI.h>
#import <TestCommon.h>

@implementation TestInfoTestIntfI


-(void) shutdown:(ICECurrent*)c
{
    [[[c adapter] getCommunicator] shutdown];
}

-(ICEContext*) getEndpointInfoAsContext:(ICECurrent*)current
{
    id<ICEEndpoint> endpoint = [current.con getEndpoint];
    ICEEndpointInfo<ICEEndpointInfo>* info = [endpoint getInfo];

    ICEMutableContext* ctx = [ICEMutableContext dictionaryWithObject:[NSString stringWithFormat:@"%d", [info timeout]]
                                                       forKey:@"timeout"];

    [ctx setObject:info.compress ? @"true" : @"false" forKey:@"compress"];
    [ctx setObject:[info datagram] ? @"true" : @"false" forKey:@"datagram"];
    [ctx setObject:[info secure] ? @"true" : @"false" forKey:@"secure"];
    [ctx setObject:[NSString stringWithFormat:@"%d", [info type]] forKey:@"type"];

    ICEIPEndpointInfo* ipinfo = (ICEIPEndpointInfo*)info;
    [ctx setObject:ipinfo.host forKey:@"host"];
    [ctx setObject:[NSString stringWithFormat:@"%d", ipinfo.port] forKey:@"port"];

    if([ipinfo isKindOfClass:[ICEUDPEndpointInfo class]])
    {
        ICEUDPEndpointInfo* udp = (ICEUDPEndpointInfo*)ipinfo;
        [ctx setObject:udp.mcastInterface forKey:@"mcastInterface"];
        [ctx setObject:[NSString stringWithFormat:@"%d", udp.mcastTtl] forKey:@"mcastTtl"];
    }
    return ctx;
}

-(ICEContext*) getConnectionInfoAsContext:(ICECurrent*)current
{
    ICEConnectionInfo* info = [[current con] getInfo];
    ICEMutableContext* ctx = [ICEMutableContext dictionaryWithObject:[info adapterName] forKey:@"adapterName"];
    [ctx setObject:info.incoming ? @"true" : @"false" forKey:@"incoming"];

    ICEIPConnectionInfo* ipinfo = (ICEIPConnectionInfo*)info;
    [ctx setObject:ipinfo.localAddress forKey:@"localAddress"];
    [ctx setObject:[NSString stringWithFormat:@"%d", ipinfo.localPort] forKey:@"localPort"];
    [ctx setObject:ipinfo.remoteAddress forKey:@"remoteAddress"];
    [ctx setObject:[NSString stringWithFormat:@"%d", ipinfo.remotePort] forKey:@"remotePort"];

    if([info isKindOfClass:[ICEWSConnectionInfo class]])
    {
        ICEWSConnectionInfo* wsinfo = (ICEWSConnectionInfo*)info;
        for(NSString* key in wsinfo.headers)
        {
            [ctx setObject:[wsinfo.headers objectForKey:key] forKey:[NSString stringWithFormat:@"ws.%@", key]];
        }
    }

    if([info isKindOfClass:[ICESSLWSSConnectionInfo class]])
    {
        ICESSLWSSConnectionInfo* wssinfo = (ICESSLWSSConnectionInfo*)info;
        for(NSString* key in wssinfo.headers)
        {
            [ctx setObject:[wssinfo.headers objectForKey:key] forKey:[NSString stringWithFormat:@"ws.%@", key]];
        }
    }
    return ctx;
}
@end
