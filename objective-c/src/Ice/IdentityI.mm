// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <IdentityI.h>
#import <Util.h>

@implementation ICEIdentity (ICEInternal)

-(ICEIdentity*) initWithIdentity:(const Ice::Identity&)arg
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    category = [[NSString alloc] initWithUTF8String:arg.category.c_str()];
    name = [[NSString alloc] initWithUTF8String:arg.name.c_str()];
    return self;
}

-(Ice::Identity) identity
{
    Ice::Identity ident;
    ident.category = fromNSString(category);
    ident.name = fromNSString(name);
    return ident;
}

+(ICEIdentity*) identityWithIdentity:(const Ice::Identity&)arg
{
    return [[[ICEIdentity alloc] initWithIdentity:arg] autorelease];
}

@end
