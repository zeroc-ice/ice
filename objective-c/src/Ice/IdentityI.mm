//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
