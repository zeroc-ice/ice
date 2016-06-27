// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

+(void) load
{
    IceObjC::registerConnectionInfoClass([ICESSLConnectionInfo class]);
}

-(id) initWithSSLConnectionInfo:(IceSSL::ConnectionInfo*)sslConnectionInfo
{
    self = [super initWithConnectionInfo:sslConnectionInfo];
    if(self)
    {
        self->cipher = [[NSString alloc] initWithUTF8String:sslConnectionInfo->cipher.c_str()];
        self->certs = toNSArray(sslConnectionInfo->certs);
        self->verified = sslConnectionInfo->verified;
    }
    return self;
}

+(id) checkedConnectionInfoWithConnectionInfo:(Ice::ConnectionInfo*)connectionInfo
{
    IceSSL::ConnectionInfo* sslConnectionInfo = dynamic_cast<IceSSL::ConnectionInfo*>(connectionInfo);
    if(sslConnectionInfo)
    {
        return [[ICESSLConnectionInfo alloc] initWithSSLConnectionInfo:sslConnectionInfo];
    }
    return nil;
}

@end
