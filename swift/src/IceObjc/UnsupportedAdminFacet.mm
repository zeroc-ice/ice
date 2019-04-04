// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "IceObjcUnsupportedAdminFacet.h"
#import "IceObjcUtil.h"

@implementation ICEUnsupportedAdminFacet

-(instancetype) initWithCppAdminFacet:(std::shared_ptr<Ice::Object>)facet
{
    self = [super initWithLocalObject:facet.get()];
    if(!self)
    {
        return nil;
    }
    _facet = facet;
    return self;
}

@end
