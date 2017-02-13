// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/IceSSL/EndpointInfo.h>
#import <EndpointI.h>
#import <LocalObjectI.h>
#import <Util.h>

#include <IceSSL/EndpointInfo.h>

@implementation ICESSLEndpointInfo (IceSSL)
+(void) load
{
    IceObjC::registerEndpointInfoClass([ICESSLEndpointInfo class]);
}

+(id) checkedEndpointInfoWithEndpointInfo:(Ice::EndpointInfo*)endpointInfo
{
    IceSSL::EndpointInfo* sslEndpointInfo = dynamic_cast<IceSSL::EndpointInfo*>(endpointInfo);
    if(sslEndpointInfo)
    {
        return [[ICESSLEndpointInfo alloc] initWithEndpointInfo:sslEndpointInfo];
    }
    return nil;
}
@end
