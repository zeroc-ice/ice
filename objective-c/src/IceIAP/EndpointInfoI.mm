// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/IceIAP/EndpointInfo.h>
#import <EndpointI.h>
#import <LocalObjectI.h>
#import <Util.h>

#include <IceIAP/EndpointInfo.h>

@implementation ICEIAPEndpointInfo (IceIAP)
+(void) load
{
    IceObjC::registerEndpointInfoClass([ICEIAPEndpointInfo class]);
}

-(id) initWithIAPEndpointInfo:(IceIAP::EndpointInfo*)iapEndpointInfo
{
    self = [super initWithEndpointInfo:iapEndpointInfo];
    if(self)
    {
         self->name = toNSString(iapEndpointInfo->name);
         self->manufacturer = toNSString(iapEndpointInfo->manufacturer);
         self->modelNumber = toNSString(iapEndpointInfo->modelNumber);
         self->protocol = toNSString(iapEndpointInfo->protocol);
    }
    return self;
}

+(id) checkedEndpointInfoWithEndpointInfo:(Ice::EndpointInfo*)endpointInfo
{
    IceIAP::EndpointInfo* iapEndpointInfo = dynamic_cast<IceIAP::EndpointInfo*>(endpointInfo);
    if(iapEndpointInfo)
    {
        return [[ICEIAPEndpointInfo alloc] initWithEndpointInfo:iapEndpointInfo];
    }
    return nil;
}
@end
