// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "Ice-Objc.h"

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
