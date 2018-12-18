// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
