//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "IceObjcInputStream.h"

@implementation ICEInputStream

-(instancetype) initWithBytes:(std::vector<Ice::Byte>)bytes;
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->_bytes = std::move(bytes);
    return self;
}

-(void*) data
{
    return _bytes.data();
}

-(size_t) size
{
    return _bytes.size();
}

@end
