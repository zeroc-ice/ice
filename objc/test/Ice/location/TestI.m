// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <location/TestI.h>

@implementation ServerManagerI
-(id) init:(ServerLocatorRegistry*)registry initData:(ICEInitializationData*)initData
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    registry_ = registry;
    initData_ = initData;

    [initData_.properties setProperty:@"TestAdapter.Endpoints" value:@"default"];
    [initData_.properties setProperty:@"TestAdapter.AdapterId" value:@"TestAdapter"];
    [initData_.properties setProperty:@"TestAdapter.ReplicaGroupId" value:@"ReplicatedAdapter"];
    
    [initData_.properties setProperty:@"TestAdapter2.Endpoints" value:@"default"];
    [initData_.properties setProperty:@"TestAdapter2.AdapterId" value:@"TestAdapter2"];

    [initData_.properties setProperty:@"Ice.PrintAdapterReady" value:@"0"];
    return self;
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [communicators_ release];
    [super dealloc];
}
#endif

-(void) startServer:(ICECurrent*)current
{
    for(id<ICECommunicator> c in communicators_)
    {
        [c waitForShutdown];
        [c destroy];
    }
#if defined(__clang__) && !__has_feature(objc_arc)
    [communicators_ release];
#endif
    communicators_ = [[NSMutableArray alloc] init];

    //
    // Simulate a server: create a [[communicator alloc] init] and object
    // adapter. The object adapter is started on a system allocated
    // port. The configuration used here contains the Ice.Locator
    // configuration variable. The [[object alloc] init] adapter will register
    // its endpoints with the locator and create references containing
    // the adapter id instead of the endpoints.
    //
    
    id<ICECommunicator> serverCommunicator = [ICEUtil createCommunicator:initData_];
    [communicators_ addObject:serverCommunicator];

    id<ICEObjectAdapter> adapter = [serverCommunicator createObjectAdapter:@"TestAdapter"];
    id<ICEObjectAdapter> adapter2 = [serverCommunicator createObjectAdapter:@"TestAdapter2"];

    id<ICEObjectPrx> locator = [serverCommunicator stringToProxy:@"locator:default -p 12010"];
    [adapter setLocator:[ICELocatorPrx uncheckedCast:locator]];
    [adapter2 setLocator:[ICELocatorPrx uncheckedCast:locator]];

#if defined(__clang__) && !__has_feature(objc_arc)
    ICEObject* object = [[[TestLocationI alloc] init:adapter adapter2:adapter2 registry:registry_] autorelease];
#else
    ICEObject* object = [[TestLocationI alloc] init:adapter adapter2:adapter2 registry:registry_];
#endif
    [registry_ addObject:[adapter add:object identity:[serverCommunicator stringToIdentity:@"test"]]];
    [registry_ addObject:[adapter add:object identity:[serverCommunicator stringToIdentity:@"test2"]]];
    [adapter add:object identity:[serverCommunicator stringToIdentity:@"test3"]];

    [adapter activate];
    [adapter2 activate];
}

-(void) shutdown:(ICECurrent*)current
{
    for(id<ICECommunicator> c in communicators_)
    {
        [c destroy];
    }
    [communicators_ removeAllObjects];
    [[current.adapter getCommunicator] shutdown];
}

-(void) terminate
{
    for(id<ICECommunicator> c in communicators_)
    {
        [c destroy];
    }
    [communicators_ removeAllObjects];
}
@end

@implementation TestLocationI
-(id) init:(id<ICEObjectAdapter>)adapter 
  adapter2:(id<ICEObjectAdapter>)adapter2 
  registry:(ServerLocatorRegistry*)registry
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
#if defined(__clang__) && !__has_feature(objc_arc)
    adapter1_ = [adapter retain];
    adapter2_ = [adapter2 retain];
    registry_ = registry;
    [registry_ addObject:[adapter1_ add:[[[HelloI alloc] init] autorelease]
                                    identity:[[adapter1_ getCommunicator] stringToIdentity:@"hello"]]];
#else
    adapter1_ = adapter;
    adapter2_ = adapter2;
    registry_ = registry;
    [registry_ addObject:[adapter1_ add:[[HelloI alloc] init]
                                    identity:[[adapter1_ getCommunicator] stringToIdentity:@"hello"]]];
#endif
    return self;
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [adapter1_ release];
    [adapter2_ release];
    [super dealloc];
}
#endif

-(void) shutdown:(ICECurrent*)current
{
    [[adapter1_ getCommunicator] shutdown];
}

-(id<TestLocationHelloPrx>) getHello:(ICECurrent*)current
{
    return [TestLocationHelloPrx uncheckedCast:[adapter1_ createIndirectProxy:[[adapter1_ getCommunicator] 
                                                                          stringToIdentity:@"hello"]]];
}

-(id<TestLocationHelloPrx>) getReplicatedHello:(ICECurrent*)current
{
    return [TestLocationHelloPrx uncheckedCast:[adapter1_ createProxy:[[adapter1_ getCommunicator] stringToIdentity:@"hello"]]];
}

-(void) migrateHello:(ICECurrent*)current
{
    ICEIdentity* ident = [[adapter1_ getCommunicator] stringToIdentity:@"hello"];
    @try
    {
        [registry_ addObject:[adapter2_ add:[adapter1_ remove:ident] identity:ident]];
    }
    @catch(ICENotRegisteredException*)
    {
        [registry_ addObject:[adapter1_ add:[adapter2_ remove:ident] identity:ident]];
    }
}
@end

@implementation HelloI
-(void) sayHello:(ICECurrent*)current
{
}
@end
