//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "UnsupportedAdminFacet.h"
#import "Util.h"

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
