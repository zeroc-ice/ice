// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

using namespace std;

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
        Ice::StringSeq sslCerts;
        for(vector<IceSSL::CertificatePtr>::const_iterator i = sslConnectionInfo->certs.begin();
            i != sslConnectionInfo->certs.end(); ++i)
        {
            sslCerts.push_back((*i)->encode());
        }
        self->cipher = [[NSString alloc] initWithUTF8String:sslConnectionInfo->cipher.c_str()];
        self->certs = toNSArray(sslCerts);
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
