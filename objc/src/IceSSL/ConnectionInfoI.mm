// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/IceSSL/ConnectionInfo.h>
#import <ConnectionI.h>
#import <LocalObjectI.h>
#import <Util.h>

#include <IceSSL/ConnectionInfo.h>

@implementation ICESSLConnectionInfo (IceSSL)

-(id) initWithSSLConnectionInfo:(IceSSL::ConnectionInfo*)sslConnectionInfo
{
    self = [super initWithIPConnectionInfo:sslConnectionInfo];
    if(self)
    {
        self->cipher = [[NSString alloc] initWithUTF8String:sslConnectionInfo->cipher.c_str()];
        self->certs = toNSArray(sslConnectionInfo->certs);
    }
    return self;
}

@end

@implementation ICEConnectionInfo (IceSSL)

+(id) connectionInfoWithType_ssl:(NSValue*)connectionInfo
{
    if(!connectionInfo)
    {
        return nil;
    }
    
    IceUtil::Shared* shared = reinterpret_cast<IceUtil::Shared*>([connectionInfo pointerValue]);
    IceSSL::ConnectionInfo* obj = dynamic_cast<IceSSL::ConnectionInfo*>(shared);
    if(obj)
    {
        return [[[ICESSLConnectionInfo alloc] initWithSSLConnectionInfo:obj] autorelease];
    }
    return nil;
}

@end

