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

static ICEIPEndpointInfo<ICEEndpointInfo>*
getIPEndpointInfo(ICEEndpointInfo<ICEEndpointInfo>* info)
{
    for(; info; info = info.underlying)
    {
        if([info isKindOfClass:[ICEIPEndpointInfo class]])
        {
            return (ICEIPEndpointInfo<ICEEndpointInfo>*)info;
        }
    }
    return nil;
}

static ICEIPConnectionInfo*
getIPConnectionInfo(ICEConnectionInfo* info)
{
    for(; info; info = info.underlying)
    {
        if([info isKindOfClass:[ICEIPConnectionInfo class]])
        {
            return (ICEIPConnectionInfo*)info;
        }
    }
    return nil;
}

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

    ICEIPEndpointInfo* ipinfo = getIPEndpointInfo(info);
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

    ICEIPConnectionInfo* ipinfo = getIPConnectionInfo(info);
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

    return ctx;
}
@end
