// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/IceSSL/EndpointInfo.h>
#import <ConnectionI.h>
#import <LocalObjectI.h>
#import <Util.h>

#include <IceSSL/EndpointInfo.h>

@implementation ICESSLEndpointInfo (IceSSL)

-(id) initWithSSLEndpointInfo:(IceSSL::EndpointInfo*)sslEndpointInfo
{
    self = [super initWithIPEndpointInfo:sslEndpointInfo];
    return self;
}

@end

@implementation ICEEndpointInfo (IceSSL)

+(id) endpointInfoWithType_2:(NSValue*)endpointInfo
{
    if(!endpointInfo)
    {
        return nil;
    }

    IceUtil::Shared* shared = reinterpret_cast<IceUtil::Shared*>([endpointInfo pointerValue]);
    IceSSL::EndpointInfo* obj = dynamic_cast<IceSSL::EndpointInfo*>(shared);
    if(obj)
    {
        return [[[ICESSLEndpointInfo alloc] initWithSSLEndpointInfo:obj] autorelease];
    }
    return nil;
}

@end

@implementation ICESSLWSSEndpointInfo (IceSSL)

-(id) initWithWSSEndpointInfo:(IceSSL::WSSEndpointInfo*)wssEndpointInfo
{
    self = [super initWithSSLEndpointInfo:wssEndpointInfo];
    if(self)
    {
        self->resource = [[NSString alloc] initWithUTF8String:wssEndpointInfo->resource.c_str()];
    }
    return self;
}

@end

@implementation ICEEndpointInfo (IceSSLWSS)

+(id) endpointInfoWithType_5:(NSValue*)endpointInfo
{
    if(!endpointInfo)
    {
        return nil;
    }

    IceUtil::Shared* shared = reinterpret_cast<IceUtil::Shared*>([endpointInfo pointerValue]);
    IceSSL::WSSEndpointInfo* obj = dynamic_cast<IceSSL::WSSEndpointInfo*>(shared);
    if(obj)
    {
        return [[[ICESSLWSSEndpointInfo alloc] initWithWSSEndpointInfo:obj] autorelease];
    }
    return nil;
}

@end

