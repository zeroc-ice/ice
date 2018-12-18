// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
    nextPort_ = 1;

    [initData_.properties setProperty:@"TestAdapter.AdapterId" value:@"TestAdapter"];
    [initData_.properties setProperty:@"TestAdapter.ReplicaGroupId" value:@"ReplicatedAdapter"];

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

-(NSString*) getTestEndpoint:(int)port
{
    return [NSString stringWithFormat:@"default -p %d", 12010 + port];
}
-(void) startServer:(ICECurrent*)__unused current
{
    for(id<ICECommunicator> c in communicators_)
    {
        [c waitForShutdown];
        [c destroy];
    }
    ICE_RELEASE(communicators_);
    communicators_ = [[NSMutableArray alloc] init];

    //
    // Simulate a server: create a [[communicator alloc] init] and object
    // adapter. The object adapter is started on a system allocated
    // port. The configuration used here contains the Ice.Locator
    // configuration variable. The [[object alloc] init] adapter will register
    // its endpoints with the locator and create references containing
    // the adapter id instead of the endpoints.
    //
    int nRetry = 10;
    while(nRetry > 0)
    {
        id<ICEObjectAdapter> adapter = nil;
        id<ICEObjectAdapter> adapter2 = nil;
        @try
        {
            id<ICECommunicator> serverCommunicator = [ICEUtil createCommunicator:initData_];
            [communicators_ addObject:serverCommunicator];

            [[serverCommunicator getProperties] setProperty:@"TestAdapter.Endpoints" value:[self getTestEndpoint:nextPort_++]];
            [[serverCommunicator getProperties] setProperty:@"TestAdapter2.Endpoints" value:[self getTestEndpoint:nextPort_++]];

            adapter = [serverCommunicator createObjectAdapter:@"TestAdapter"];
            adapter2 = [serverCommunicator createObjectAdapter:@"TestAdapter2"];

            id<ICEObjectPrx> locator = [serverCommunicator stringToProxy:@"locator:default -p 12010"];
            [adapter setLocator:[ICELocatorPrx uncheckedCast:locator]];
            [adapter2 setLocator:[ICELocatorPrx uncheckedCast:locator]];

            ICEObject* object = ICE_AUTORELEASE([[TestLocationI alloc] init:adapter adapter2:adapter2 registry:registry_]);
            [registry_ addObject:[adapter add:object identity:[ICEUtil stringToIdentity:@"test"]]];
            [registry_ addObject:[adapter add:object identity:[ICEUtil stringToIdentity:@"test2"]]];
            [adapter add:object identity:[ICEUtil stringToIdentity:@"test3"]];

            [adapter activate];
            [adapter2 activate];
            break;
        }
        @catch(ICESocketException* ex)
        {
            if(nRetry == 0)
            {
                @throw ex;
            }

            // Retry, if OA creation fails with EADDRINUSE (this can occur when running with JS web
            // browser clients if the driver uses ports in the same range as this test, ICE-8148)
            if(adapter != nil)
            {
                [adapter destroy];
            }
            if(adapter2 != nil)
            {
                [adapter2 destroy];
            }
        }
    }
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
    adapter1_ = ICE_RETAIN(adapter);
    adapter2_ = ICE_RETAIN(adapter2);
    registry_ = registry;
    [registry_ addObject:[adapter1_ add:[HelloI hello] identity:[ICEUtil stringToIdentity:@"hello"]]];
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

-(void) shutdown:(ICECurrent*)__unused current
{
    [[adapter1_ getCommunicator] shutdown];
}

-(id<TestLocationHelloPrx>) getHello:(ICECurrent*)__unused current
{
    return [TestLocationHelloPrx uncheckedCast:[adapter1_ createIndirectProxy:[ICEUtil stringToIdentity:@"hello"]]];
}

-(id<TestLocationHelloPrx>) getReplicatedHello:(ICECurrent*)__unused current
{
    return [TestLocationHelloPrx uncheckedCast:[adapter1_ createProxy:[ICEUtil stringToIdentity:@"hello"]]];
}

-(void) migrateHello:(ICECurrent*)__unused current
{
    ICEIdentity* ident = [ICEUtil stringToIdentity:@"hello"];
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
-(void) sayHello:(ICECurrent*)__unused current
{
}
@end
