// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <objc/Ice.h>
#include <DirectoryI.h>

@implementation DirectoryI

@synthesize myName;
@synthesize parent;
@synthesize ident;
@synthesize contents;

+(id) directoryi:(NSString *)name parent:(DirectoryI *)parent
{
    DirectoryI *instance = ICE_AUTORELEASE([[DirectoryI alloc] init]);
    if(instance == nil)
    {
        return nil;
    }
    instance.myName = name;
    instance.parent = parent;
    instance.ident = [ICEIdentity identity:(parent ? [ICEUtil generateUUID] : @"RootDir") category:nil];
    instance.contents = [[NSMutableArray alloc] init];
    return instance;
}

-(NSString *) name:(ICECurrent *)current
{
    return myName;
}

-(NSArray *) list:(ICECurrent *)current
{
    return contents;
}

-(void) addChild:(id<FSNodePrx>)child
{
     [contents addObject:child];
}

-(void) activate:(id<ICEObjectAdapter>)a
{
    id<FSNodePrx> thisNode = [FSNodePrx uncheckedCast:[a add:self identity:ident]];
    [parent addChild:thisNode];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [myName release];
    [parent release];
    [ident release];
    [contents release];
    [super dealloc];
}
#endif
@end
